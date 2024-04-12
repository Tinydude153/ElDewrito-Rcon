#include <RconCLI/network.h>

namespace Network {

const char* ProtocolToString(Protocol protocol) {

    switch (protocol) {

        case LOG_HTTP:
            return "[HTTP] ";
        case LOG_WEBSOCKET:
            return "[WEBSOCKET] ";
        default:
            return NULL;

    }

    return NULL;
}

void Log(const char* msg, Protocol protocol) {

    LogStream(std::cerr, ProtocolToString(protocol), msg, '\n');

}

// Outputs to stream specified by the first parameter; newline character must be manually appended 
// because this returns a reference to the stream so extra information can be outputted.
std::ostream& Log(std::ostream& stream, const char* msg, Protocol protocol) {

    LogStream(stream, ProtocolToString(protocol), msg);
    return stream;

}

}