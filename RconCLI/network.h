#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>
#include <ostream>

namespace Network {

static bool g_networkfail;

enum Protocol {

    LOG_HTTP,
    LOG_WEBSOCKET

};

template<typename T>
void LogStream(std::ostream& stream, T data) {

    stream << data;

}

template<typename T, typename... Args>
void LogStream(std::ostream& stream, T data, Args... args) {

    stream << data;
    LogStream(stream, args...);

}

const char* ProtocolToString(Protocol protocol);
void Log(const char* msg, Protocol protocol);
std::ostream& Log(std::ostream& stream, const char* msg, Protocol protocol);

} 

#endif