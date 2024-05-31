#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include <RconCLI/wsnonce.h>
#include <RconCLI/http/http.h>
#include <RconCLI/logging.h>
#include <RconCLI/console_util.h>
#include <RconCLI/input.h> 

#include <sstream>
#include <iostream>
#include <string>
#include <cstring>
#include <mutex>
#include <bitset>

class Websocket {
    
    //Command* CommandBuf;
    Network::HTTP* Http;
    char* CmdBuffer;
    char* data;
    uint64_t msg_size;

    SOCKET web_socket;

    // Print debug.
    std::string debug_info;

    public:

    typedef enum opcode_type : uint8_t {
        TEXT = 0x1,
        BINARY = 0x2,
        CLOSE = 8,
        PING = 9,
        PONG = 0xa,
        INVALID
    } OPCODE_TYPE;

    // If true, dumps websocket output to a binary file.
    bool DumpBinary = false;
    // For logging.
    bool Log_Robust = false;
    std::stringstream LogSs;
    // Stringstream for each frame; this is only logged when there is an invalid frame error.
    std::stringstream frameoutput;
    // Constructor; initializes web_socket to sock.
    Websocket(const char* host, int port, std::string protocol = "");
    ~Websocket();
    // Returns a reference to the web_socket member.
    SOCKET& GetSocket();
    // Parses the opcode of a websocket packet.
    OPCODE_TYPE parse_opcode(char* recBuffer);
    // Parses the payload of a websocket packet and returns the payload in a char*.
    char* parse_payload(char* recBuffer);
    // Composes a websocket packet to be sent; this needs to be updated and cleaned up, but it presently functions fine.
    char* compose_frame(opcode_type type, char* input);
    // for sending on binary and text websocket packets on an established socket;
    bool sendData(opcode_type type, char* input);
    // std::string overload.
    bool sendData(opcode_type type, std::string input);
    // for sending CLOSE, PING, and PONG websockets packets on an established socket
    bool sendData(opcode_type type);
    // for receiving data on an established socket
    char* receiveData();
    // for receiving data on an established socket and processing the data in a separate thread
    void threadedOutput();
    // Checks a websocket frame to ensure its validity; somewhat incomplete because it does not check the 
    // actual payload itself
    bool checkFrame(char* frame);
    // Returns the input buffer from a Command instance.
    //char* GetCommandBuffer(Command* CommandInstance);
    // Inspects a websocket packet for debugging purposes; prints the information.
    void InspectPacket(const char* packet);

};

#endif