#include <RconCLI/http/http.h>

namespace Network {

HTTP::HTTP(const char* host, int port) {

    // Convert port to char* and concatenate the host address with a colon between;
    // for HTTP header purposes.
    HTTP::Port = std::to_string(port);
    HTTP::Host.assign(host);
  
    // Header host field.
    std::string HostField;
    HostField.append(HTTP::Host);
    HostField.push_back(':');
    HostField.append(HTTP::Port);

    // Add host field to be sent with any following request.
    HTTP::AddField("Host", HostField);

    // Create socket for HTTP.
    try {

        HTTP::socket = new Sockets((char*)host, port);

    } catch (const std::exception& e) {

        std::cout << '[' <<__FUNCTION__ << "] " << e.what() << '\n';
        std::cerr << '[' <<__FUNCTION__ << "] " << e.what() << '\n';
        throw;

    }


}

HTTP::~HTTP() {

    delete HTTP::socket;

}

// Returns a reference to a Socket object's socket member.
SOCKET& HTTP::GetSocket() {

    return HTTP::socket->GetInternalSocket();

}

// For accessing Socket class members outside, if needed.
// Returns a reference to a pointer of a Socket object.
Sockets*& HTTP::GetSocketClass() {

    // HTTP::socket is guaranteed to be instantiated, as the constructor will always instantiate the socket class.
    return HTTP::socket;

}

bool HTTP::Connect(const char* host, int port) {

    std::string str_port = std::to_string(port);
    std::string str_host(host);

    std::string header_fields;
    header_fields.append(str_host);
    header_fields.push_back(':');
    header_fields.append(str_port);

    return true;

}

void HTTP::SendRequest(HTTP::METHOD method, const std::string path) {

    // String for the request line; const char* for the final header string conversion to be sent;
    // char* for the received bytes from the server.
    std::string RequestLine;
    const char* headersString;
    char* received;  

    // Create request line depending on the method.
    switch (method) {

        case HTTP::METHOD::GET:

            RequestLine.append("GET ");
            if (path.length() > 0) {

                RequestLine.append(path);

            } else { RequestLine.push_back('/'); }
            RequestLine.append(" HTTP/1.1\r\n");
            break;

        default:
            break;

    }

    // Insert the request line at the beginning of the string before all headers that exist, if any, 
    // add final CRLF, then convert to const char* to send to server.
    HTTP::Headers.insert(0, RequestLine);
    HTTP::Headers.append("\r\n");
    headersString = HTTP::Headers.c_str();
    HTTP::socket->Send(headersString); // TODO: check return value.

    // Reset the header string for the next request.
    HTTP::Headers.clear();

    // Return what is received from the server; if nothing is received, return NULL.
    HTTP::Response = HTTP::socket->Receive(10240);
    /*if (received) return received;
    return NULL;*/

}

void HTTP::AddField(const std::string key, const std::string value) {

    HTTP::Headers.append(key);
    HTTP::Headers.append(": ");
    HTTP::Headers.append(value);
    HTTP::Headers.append("\r\n");

}

char* HTTP::ParseResponse() {

    if (!HTTP::Response) {

        Log("Response is NULL.", Protocol::LOG_HTTP);
        std::cout << std::cerr.rdbuf();
        return NULL;

    }

    // Strings for the loop below for parsing the headers.
    std::string resp(HTTP::Response);
    std::string key;
    std::string value;
    std::string Headers;
    std::string crlf = "\r\n";
    std::string col = ": ";
    std::string Body = resp;
    Body.erase(0, resp.find("\r\n\r\n") + 3);
    if (!Body.empty()) {
        
        HTTP::ResponseBody = Body;
    
    }

    // Separate headers from message body, if it exists; if sequence is not found,
    // the headers are probably not valid.
    size_t header_termination = resp.find("\r\n\r\n");
    if (header_termination == std::string::npos) {

        Log("Invalid HTTP response header received.", Protocol::LOG_HTTP);
        return NULL;

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
        HeaderMap.insert(std::pair<std::string, std::string>(key, value));

        key.clear();
        value.clear();

    }

    return HTTP::Response;

}

std::string HTTP::GetFieldValue(std::string key) {

    std::string value = "";
    for (std::map<std::string, std::string>::iterator it = HTTP::HeaderMap.begin(); it != HTTP::HeaderMap.end(); it++) {

        if (it->first.compare(key) == 0) {

            if (!it->second.empty()) {

                value = it->second;
                return value;

            }

        }

    }

    return value;

}

}