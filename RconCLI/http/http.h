#ifndef HTTP_H
#define HTTP_H

#include <RconCLI/utils.h>
#include <RconCLI/sockets/sockets.h>
#include <RconCLI/debug.h>
#include <RconCLI/network.h>
#include <string>
#include <map>
#include <vector>

namespace Network {

class HTTP {

    // Private class-wide header string that contains the headers to be sent in a request.
        // When sent, this will be converted into a const char*. 
    std::string Headers;
    std::string Host;
    std::string Port;
    char* Response = NULL;
    
    // For mapping response headers.
    std::map<std::string, std::string> HeaderMap;

    // Public class-wide instantiation of the Sockets class; this is public so the socket
    // can be passed, if necessary.
    Sockets* socket;

    public:
    std::string ResponseBody;
    typedef enum method {

        GET,
        POST

    } METHOD;

    // Returns a reference to the socket from the Sockets class, mainly for socket reference.
    SOCKET& GetSocket();
    // Returns a reference to a pointer of the Sockets class.
    Sockets*& GetSocketClass();
    // Constructor; creates socket and creates host header field and adds it with AddField().
    HTTP(const char* host, int port);
    ~HTTP();
    // Creates and connects a socket with the specified host and port.
    bool Connect(const char* host, int port);
    // Sends HTTP request.
    void SendRequest(METHOD method, const std::string path = "");
    // For adding header fields.
    void AddField(const std::string key, const std::string value);
    // For parsing headers.
    char* ParseResponse();
    // Iterates through the shared map of headers to find the value of a specified key.
    std::string GetFieldValue(std::string key);

};

}

#endif