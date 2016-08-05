# Web-Proxy
Single-threaded http proxy that passes requests and data between a web client and server

### Usage:
Run the proxy in the terminal with the following command:

    ./proxy <port>
    
Specify a port number outside of the reserved range (1-1024) for the proxy to listen on. 
    
Request a page using telnet:

    telnet localhost <port>
    Trying 127.0.0.1...
    Connected to localhost.localdomain (127.0.0.1).
    Escape character is '^]'.
    GET http://www.stanford.edu/ HTTP/1.0

HTTP request line format: 

    <request-method-name> <request-URI> <HTTP/version>
    <---Empty line--->
    
Supports GET and HEAD requests. Does not support https. URI must have absolute path and end with "/".
Proxy returns the headers and HTML of the requested page. 
