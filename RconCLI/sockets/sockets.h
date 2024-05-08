#ifndef SOCKETS_H
#define SOCKETS_H

#include <RconCLI/wsnonce.h>
#include <RconCLI/utils.h>

#include <exception>
#include <map>
#include <string>
#include <iostream>
#include <stdio.h>

#if _WIN32  
    #include <Winsock2.h>
    #include <WS2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socketfd_t;
    #define SOCKET_ERROR_NO (WSAGetLastError())
    #define S_EWOULDBLOCK (WSAEWOULDBLOCK)
#else 
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <arpa/inet.h>
    typedef int socketfd_t;
    #define SOCKET_ERROR_NO (errno)
    #define S_EWOULDBLOCK (EWOULDBLOCK)
#endif

class Sockets {
private:
    char* host;
    int port;
    sockaddr_in hint;

    // Internal socket object.
    socketfd_t m_socket;

    // For dealing with HTTP responses.
    std::map<std::string, std::string> ParseHttpHeaders(char* data);
    std::string GetHttpHeaderValue(std::string key, std::map<std::string, std::string> headers);

    // Socket connection.
    int ConnectSocket();
    int RetryConnection();
    const char* GetErrorMsg();

public:

    // Returns a reference to the internal socket.
    socketfd_t& GetInternalSocket();
    // Sends data on a connected socket.
    int Send(const char* buffer);
    // Receives all pending data on a socket; non-blocking by default.
    char* Receive(size_t buf_size);
    // Poll() is just a mask for the select() function.
    int Poll(socketfd_t socket, long seconds, long micro_seconds);
    // Sets a socket to non-blocking mode.
    void SetNonBlocking(socketfd_t socket);
    // Sets a socket to blocking mode.
    void SetBlocking(socketfd_t socket);
    // Sets a socket within a Sockets object to non-blocking mode.
    void SetNonBlocking();
    // Sets a socket within a Sockets object to blocking mode.
    void SetBlocking();
    // If response is expected to be that of HTTP, this function is used to find headers and get content length;
    // the header_buffer argument is if headers are to be exported, optionally.
    int HttpContentLength(int bytes_received, char* curr_buf);
    // Constructor; creates a TCP socket and attempts to connect it.
    Sockets(char* m_host, int m_port, char* header);
    Sockets(char* m_host, int m_port);
    ~Sockets();
};

#endif