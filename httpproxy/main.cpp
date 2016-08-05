//
//  main.cpp
//  HTTP Proxy
//
//  Created by Swathi Iyer on 7/19/16.
//  Copyright © 2016 Swathi Iyer. All rights reserved.
//

#define BACKLOG 10
#define REQUESTSIZE 100
#define RESPONSESIZE 1024

#include <iostream>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <Poco/URI.h>
#include <Poco/Net/HostEntry.h>

void makeConnection(int & cli, int port);
void getRequest(int & cli);
void getRequestInfo(char buffer[REQUESTSIZE]);
void getData(int & cli);
void returnData(char response[RESPONSESIZE], int & cli);

struct req{
    char buffer[REQUESTSIZE];
    std::string method;
    std::string URI;
    std::string version;
    std::string host;
    unsigned short port;
    std::string path;
};

struct req request;

int main(int argc, const char * argv[]) {
    if(argc != 2){
        std::cerr << "Requires two arguments." << std::endl;
        exit(-1);
    }
    int port = atoi(argv[1]);
    int cli;
    makeConnection(cli, port);
    getData(cli);
    close(cli);
    return 0;
}

/* Connect Proxy and Client
 */

void makeConnection(int & cli, int port){
    int proxServ;
    // 1. Define Socket.
    if ((proxServ = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket: ");
        exit(-1);
    }
    
    // 2. Bind server socket to port
    struct sockaddr_in proxAddr; // Struct to hold address information
    bzero( &proxAddr, sizeof(proxAddr));
    proxAddr.sin_family = AF_INET;
    proxAddr.sin_port = htons(port); // short, network byte order
    proxAddr.sin_addr.s_addr= htons(INADDR_ANY);
    
    if((bind(proxServ, (struct sockaddr *) &proxAddr, sizeof(struct sockaddr_in))) == -1){
        perror("bind");
        exit(-1);
    }
    
    // 3. Listen for connections
    if(listen(proxServ, BACKLOG) == -1){
        perror("listen");
        exit(-1);
    }
    
    // 4. Accept an incoming connection from client
    if((cli = accept(proxServ, (struct sockaddr*) NULL, NULL)) == -1){
        perror("accept");
        exit(-1);
    }
    
    //5. Get Request from Client
    getRequest(cli);
    close(proxServ);
    
}

void getRequest(int & cli){
    std::string currLine;
    char buffer [REQUESTSIZE];
    char tempBuff[REQUESTSIZE];
    int i = 0;
    while(true){
        bzero(tempBuff, REQUESTSIZE);
        read(cli, tempBuff, REQUESTSIZE);
        std::cout << tempBuff << std::endl;
        if(i == 0){
            strcpy(buffer, tempBuff);
        } else {
            strcat(buffer, tempBuff);
        }
        currLine = buffer;
        if(currLine.find("\r\n\r\n") != std::string::npos){
            break;
        }
        i++;
    }
    printf("Received - %s",buffer);
    getRequestInfo(buffer);
}

/* Parse HTTP request. Parse URI using Poco lib
 */

void getRequestInfo(char buffer[REQUESTSIZE]){
    std::strcpy(request.buffer, buffer);
    
    //Parse Request
    char *token = std::strtok(buffer, " ");
    std::vector<std::string> v;
    while (token!= NULL) {
        v.push_back(token);
        token = std:: strtok(NULL, " ");
    }
    
    request.method = v[0];
    request.URI = v[1];
    request.version = v[2];
    
    //Parse URI
    Poco::URI uri(request.URI);
    request.host = uri.getHost();
    request.port = uri.getPort();
    request.path = uri.getPath();
    delete[] token;
}

/* Connect to requested host, send HTTP request, receive response
 */

void getData(int & cli){
    int proxCli;
    if ((proxCli = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("proxyCli socket: ");
        close(proxCli);
        exit(-1);
    }
    
    //Convert domain name to ip address
    struct hostent * ip;
    if((ip = gethostbyname(request.host.c_str())) == NULL){
        perror("gethostbyname");
        exit(-1);
    };
    struct in_addr **addr_list = (struct in_addr **)ip->h_addr_list;
    std::string str = inet_ntoa(*addr_list[0]);
    printf("The ip address is %s ", str.c_str());
    
    //Fill struct
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(request.port);
    inet_aton(str.c_str(),&servAddr.sin_addr);


    //Connect
    if((connect(proxCli, (struct sockaddr *) & servAddr, sizeof(servAddr))) != 0){
        perror("connect");
        close(proxCli);
        
        exit(-1);
    }
    
    //Send
    if(send(proxCli, &request.buffer, std::strlen(request.buffer), 0) == -1){
        perror("send request");
        exit(-1);
    }
    
    //Get Response
    char response[RESPONSESIZE];
    while(recv(proxCli,response,RESPONSESIZE,0) > 0){
        returnData(response, cli);
    }
    close(proxCli);
    delete[] ip;
}

/* Send data to client
 */

void returnData(char response[RESPONSESIZE], int & cli){
    if(send(cli, response, std::strlen(response), 0) == -1){
        perror("send request");
        exit(-1);
    }
}
