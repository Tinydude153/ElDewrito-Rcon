#include <RconCLI/sockets/sockets.h>

//
// This header file is becoming somewhat obsolete, as most of the socket handling is being migrated
// into the websocket class in the websocket.h header file. This class may still be used, but on a 
// smaller scale, only for HTTP purposes. It probably won't handle sockets unless from a handle 
// created by the websocket class.
//

std::map<std::string, std::string> Sockets::ParseHttpHeaders(char* data) {

    std::map<std::string, std::string> header_fields;
    header_fields.clear();

    if (!data) {

        LogSocketError("HTTP response is null.");
        return header_fields;

    }

    // Strings for the loop below for parsing the headers.
    std::string resp(data);
    std::string key;
    std::string value;
    std::string Headers;
    std::string crlf = "\r\n";
    std::string col = ": ";

    // Separate headers from message body, if it exists; if sequence is not found,
    // the headers are probably not valid.
    size_t header_termination = resp.find("\r\n\r\n");
    if (header_termination == std::string::npos) {

        LogSocketError("Invalid HTTP headers received.");
        return header_fields;

    } else {

        Headers = resp.substr(0, header_termination + 2);
        Headers.erase(0, Headers.find(crlf) + 2); 

    }

    // For colon position and CRLF position tracking.
    size_t col_pos = 0;
    size_t crlf_pos = 0;

    while (crlf_pos != std::string::npos) {

        crlf_pos = Headers.find(crlf);
        col_pos = Headers.find(col);

        if (col_pos == std::string::npos) break;
        if (crlf_pos == std::string::npos) break;

        key = Headers.substr(0, col_pos);
        value = Headers.substr(col_pos + 2, crlf_pos - col_pos - 2);
        Headers.erase(0, crlf_pos + 2);
        crlf_pos = Headers.find(crlf);

        // Insert key value pair into map.
        header_fields.insert(std::pair<std::string, std::string>(key, value));

        key.clear();
        value.clear();

    }

    return header_fields;

}

std::string Sockets::GetHttpHeaderValue(std::string key, std::map<std::string, std::string> headers) {

    std::string value = "";

    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++) {

        if (it->first.compare(key) == 0) {

            if (!it->second.empty()) {

                value = it->second;
                return value;

            }

        }

    }

    return value;

}

// Sets a specified socket to non-blocking mode.
void Sockets::SetNonBlocking(SOCKET socket) {

    // 0 for blocking, 1 for non-blocking.
    unsigned long mode = 1;
    int ioct_res = ioctlsocket(socket, FIONBIO, &mode);
    if (ioct_res == SOCKET_ERROR) {
        LogSocketError("Winsock ioctlsocket() failed: ", WSAGetLastError());
    }

    return;
}

// Sets the socket within a Sockets object to non-blocking.
void Sockets::SetNonBlocking() {

    // 0 for blocking, 1 for non-blocking.
    unsigned long mode = 1;
    int ioct_res = ioctlsocket(this->m_socket, FIONBIO, &mode);
    if (ioct_res == SOCKET_ERROR) {
        LogSocketError("Winsock ioctlsocket() failed: ", WSAGetLastError());
        std::cout << std::cerr.rdbuf();
    }

    return;
}

// Sets a specified socket to blocking mode.
void Sockets::SetBlocking(SOCKET socket) {

    // 0 for blocking, 1 for non-blocking.
    unsigned long mode = 0;
    int ioct_res = ioctlsocket(socket, FIONBIO, &mode);
    if (ioct_res == SOCKET_ERROR) {
        LogSocketError("Winsock ioctlsocket() failed: ", WSAGetLastError());
        std::cout << std::cerr.rdbuf();
    }

    return;
}

void Sockets::SetBlocking() {

    // 0 for blocking, 1 for non-blocking.
    unsigned long mode = 0;
    int ioct_res = ioctlsocket(this->m_socket, FIONBIO, &mode);
    if (ioct_res == SOCKET_ERROR) {
        LogSocketError("Winsock ioctlsocket() failed: ", WSAGetLastError());
        std::cout << std::cerr.rdbuf();
    }

    return;
}

int Sockets::Poll(SOCKET socket, long seconds, long micro_seconds) {

    TIMEVAL timev;
    fd_set fds;

    timev.tv_sec = seconds;
    timev.tv_usec = micro_seconds;

    FD_ZERO(&fds);
    FD_SET(socket, &fds);
    int sel = select(0, &fds, NULL, NULL, &timev);
    if (sel == SOCKET_ERROR) {
        LogSocketError("Winsock Select() failed: ", WSAGetLastError());
    }

    return sel;
}

// Returns content-length value AND the total length of headers.
int Sockets::HttpContentLength(int bytes_received, char* curr_buf) {

    int offset = 0;
    int content_length;
    std::string cl;
    std::map<std::string, std::string> response_headers;
    char* headers_c;
    size_t sz;

    for (offset = 0; offset < bytes_received; offset++) {

        if (curr_buf[offset] == '\r' && curr_buf[offset + 3] == '\n') {
                
            headers_c = Substring(curr_buf, 0, offset + 3);
            response_headers = Sockets::ParseHttpHeaders(headers_c);
            cl = Sockets::GetHttpHeaderValue("Content-Length", response_headers);
            if (!cl.empty()) {

                content_length = std::stoi(cl, &sz);

                // Adding three for the three unaccounted escape characters "\n\r\n"
                content_length = content_length + offset + 3;

            } else {

                return 0;
                
            }
            std::cout << "\nCONTENT_LENGTH: " << content_length << "\n";
            printf("\nSIZE_OF_BUF: %d\n", strlen(curr_buf));
            std::cout << "\nHEADER_FROM_REC: \n" << curr_buf;
            return content_length;

        }

    }

    return 0;

}

// Connects a socket with data that is defined and initialized by the constructor.
// Returns 1 on successful connection, and 0 on SOCKET_ERROR.
int Sockets::ConnectSocket() {

    int gla;
    int connection;
    connection = connect(m_socket, (sockaddr*)&hint, sizeof(hint));
    if (connection != SOCKET_ERROR) {

        //printf("\nConnection successful\nHost: %s:%d", host, port);
        LogSocket(std::cerr, "[SOCKET] ", "Connection ", host, ':', port, " success.", '\n');
        return connection;

    }
    if (connection == SOCKET_ERROR) {
        
        gla = WSAGetLastError();
        switch (gla) {

            case WSAEADDRNOTAVAIL:
                LogSocketError("The remote address is not a valid address.", gla);
                return gla;
            case WSAECONNREFUSED:
                LogSocketError("The attempt to connect was forcefully rejected.", gla);
                return gla;
            case WSAEISCONN:
                LogSocketError("The socket is already connected.", gla);
                return gla;
            case WSAETIMEDOUT:
                LogSocketError("The connection attempt timed out without establishing a connection.", gla);
                return gla;
            default: 
                LogSocketError("The connection attempt failed.", gla);
                WSACleanup();
                return gla;

        }

    }

    return connection;
}

// Sends on the socket defined and initialized by the constructor. Returns bytes 
// sent on success and Winsock error code upon failure (SOCKET_ERROR).
int Sockets::Send(const char* buffer) {

    int gla;
    int sent = send(m_socket, buffer, strlen(buffer), 0);

    if (sent != SOCKET_ERROR) {
        return sent;
    }
    if (sent == SOCKET_ERROR) {

        gla = WSAGetLastError();
        if (gla == WSAETIMEDOUT) {
            LogSocketError("The connection has been dropped.", gla);
            throw std::runtime_error("Socket exception: connection has been dropped.");
            return gla;
        } else if (gla == WSAENOTCONN) {
            LogSocketError("The socket is not connected.", gla);
            throw std::runtime_error("Socket exception: not connected.");
            return gla;
        }
        LogSocketError("Winsock Send() failed: ", gla);
        throw std::runtime_error("Socket exception: Send() failed.");
        closesocket(m_socket);
        return gla;

    }

    return 0;
}

char* Sockets::Receive(size_t buf_size) {

    char* rec_buf = (char*)malloc(sizeof(char*) * buf_size);
    memset(rec_buf, 0, buf_size);
    int total_content_length = 0;
    size_t total_rec = 0;
    int gla;

    Sockets::SetNonBlocking(Sockets::m_socket);
    int rec = 0;
    int sel;
    bool seq_f = false;
    while (!seq_f) {

        // Use select() on socket to check for readability.
        sel = Sockets::Poll(Sockets::m_socket, 1, 0);

        // Receive on socket.
        if (sel > 0) {

            // Receive then search for headers and get content-length, if existent.
            rec = recv(m_socket, rec_buf + total_rec, buf_size - total_rec - 1, 0);
            total_content_length = Sockets::HttpContentLength(rec, rec_buf);
            if (total_content_length) {
                total_content_length++;
                total_rec += rec;
                if (total_rec == total_content_length) {
                    return rec_buf;
                }

            }

        } else if (sel == 0) {

            // Socket timed out.
            return NULL;

        } else if (sel == SOCKET_ERROR) {

            gla = WSAGetLastError();
            if (gla == WSAEWOULDBLOCK) {

                // No data to be read; return.
                seq_f = true;
                printf("\nRECEIVED_RETURNED: %s\n", rec_buf);
                return rec_buf;

            } else {
                
                LogSocketError("Winsock Send() failed: ", WSAGetLastError());
                return NULL;

            }

        }
        
    }

    if (rec_buf) {
        return rec_buf;
    } else { return NULL; }

}

// Returns a reference to the internal socket.
SOCKET& Sockets::GetInternalSocket() {

    return Sockets::m_socket;

}

// Tries to connect a socket five times, if failed.
int Sockets::RetryConnection() {

    int conn;
    for (int i = 0; i < 1; i++) {
        conn = Sockets::ConnectSocket();
        if (!conn) return 0;
    }

    return 1;
}

// Constructor for the socketInit class. This constructor creates a socket
// and connects it, then immediately send the data specified in the third
// parameter, "char* header;" this was written with HTTP requests in mind.
Sockets::Sockets(char* m_host, int m_port, char* data) {
    host = m_host;
    port = m_port;

    // Initialize Winsock.
    WSADATA wsaData;
    int initWS;
    initWS = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (initWS != 0) {
        LogSocketError("WSAStartup failed: ", initWS);
    }

    SOCKET m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) {
        LogSocketError("Failed to create socket: ", WSAGetLastError());
        WSACleanup();
    }

    // sockaddr_in structure definitions
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, host, &hint.sin_addr);

    m_socket = m_socket;

    int socket_connection = ConnectSocket();
    if (socket_connection == 1) {
        LogSocketError("Socket connected.");
        int sendBuf = Send(data);
    } else {
        LogSocketError("Failed to send on socket: ", WSAGetLastError());
    }
}

// Constructor for the socketInit class. This constructor creates a socket
// with the specified host and port then attempts to connect it.
Sockets::Sockets(char* m_host, int m_port) {
    host = m_host;
    port = m_port;

    // Initialize Winsock.
    WSADATA wsaData;
    int initWS;
    initWS = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (initWS != 0) {
        printf("\nWSAStartup failed: %d\n", initWS);
    }

    SOCKET t_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (t_socket == INVALID_SOCKET) {
        printf("\nFailed to create socket: %d", WSAGetLastError());
        WSACleanup();
    }

    // sockaddr_in structure definitions
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, host, &hint.sin_addr);

    Sockets::m_socket = t_socket;

    int retry_conn;
    int socket_connection = ConnectSocket();
    if (socket_connection == 0) {
        printf("\nSocket: connected\n");
    } else {
        retry_conn = RetryConnection();
        if (!retry_conn) {
            printf("\nSocket: connected\n");
        } else {

            std::cerr << '[' <<__FUNCTION__ << "] " << "Socket exception thrown: cannot connect to address." << '\n';
            throw std::runtime_error("Socket exception thrown: cannot connect to address.");
            WSACleanup();

        }
    }
}

Sockets::~Sockets() {

    shutdown(Sockets::m_socket, 2);
    closesocket(Sockets::m_socket);
    WSACleanup();

}
