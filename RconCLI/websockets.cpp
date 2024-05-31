#include <RconCLI/websockets.h>

// Websocket class. This is for frame building based on the
// websocket base framing protocol (RFC 6455 5.2) 
Websocket::Websocket(const char* host, int port, std::string protocol) {

    // Generate the 16-byte nonce.
    char* wsNonce = genNonce();
    std::string nonceStr((const char*)wsNonce);
    free(wsNonce);

    // Prepare an HTTP request to switch to the websocket protocol.
    try {

        Websocket::Http = new Network::HTTP(host, port);
        Websocket::Http->AddField("Connection", "upgrade");
        Websocket::Http->AddField("Upgrade", "WebSocket");
        Websocket::Http->AddField("Sec-WebSocket-Version", "13");
        if (!protocol.empty()) {
            Websocket::Http->AddField("Sec-WebSocket-Protocol", protocol);
        }
        Websocket::Http->AddField("Sec-WebSocket-Key", nonceStr);
        Websocket::Http->SendRequest(Network::HTTP::METHOD::GET);

        // Set the socket to block because websockets are persistent.
        Websocket::Http->GetSocketClass()->SetBlocking();
        Websocket::web_socket = Websocket::Http->GetSocket();
        msg_size = 0;

    } catch (const std::exception& e) {

        std::cout << '[' <<__FUNCTION__ << "()] " << e.what() << '\n';
        std::cerr << '[' <<__FUNCTION__ << "()] " << e.what() << '\n';
        throw;

    }
    
}

Websocket::~Websocket() {

    delete Websocket::Http;

}

SOCKET& Websocket::GetSocket() {

    return Websocket::web_socket;

}

// This function is meant to parse the opcode of a received websocket packet
Websocket::OPCODE_TYPE Websocket::parse_opcode(char* recBuffer) {

    /*
    // DEBUGGING
    std::cout << std::endl << std::bitset<8>(recBuffer[0]) << std::endl;
    std::cout << std::endl << std::bitset<8>(recBuffer[0] << 4) << std::endl;
    std::cout << std::endl << std::bitset<8>(websocket::opcode_type::TEXT << 4) << std::endl;
    std::cout << std::endl << std::bitset<8>(((recBuffer[0] << 4 ) ^ (websocket::opcode_type::TEXT << 4))) << std::endl; 
    */
    if ((recBuffer[0] & 15) == Websocket::opcode_type::TEXT) {
        return Websocket::opcode_type::TEXT;
    } else if ((recBuffer[0] & 15) == Websocket::opcode_type::BINARY) {
        return Websocket::opcode_type::BINARY;
    } else if ((recBuffer[0] & 15) == Websocket::opcode_type::CLOSE) {
        return Websocket::opcode_type::CLOSE;
    } else if ((recBuffer[0] & 15) == Websocket::opcode_type::PING) {
        return Websocket::opcode_type::PING;
    } else if ((recBuffer[0] & 15) == Websocket::opcode_type::PONG) {
        return Websocket::opcode_type::PONG;
    }

    return Websocket::opcode_type::INVALID;
}

// Checks a websocket frame to ensure its validity; somewhat incomplete because it does not check the 
// actual payload itself
bool Websocket::checkFrame(char* frame) {

    Network::LogStream(Websocket::frameoutput, "\n[WEBSOCKET FRAME]");
    Websocket::OPCODE_TYPE ot = parse_opcode(frame);
    if (ot != Websocket::opcode_type::INVALID) {

        // Output and determine opcode and whether it is invalid or not.
        switch (ot) {
            case Websocket::opcode_type::TEXT:
                Network::LogStream(Websocket::frameoutput, "\nOPCODE: TEXT");
                break;
            case Websocket::opcode_type::BINARY:
                Network::LogStream(Websocket::frameoutput, "\nOPCODE: BINARY");
                break;
            case Websocket::opcode_type::CLOSE:
                Network::LogStream(Websocket::frameoutput, "\nOPCODE: CLOSE");
                break;
            case Websocket::opcode_type::PING:
                Network::LogStream(Websocket::frameoutput, "\nOPCODE: PING");
                break;
            case Websocket::opcode_type::PONG:
                Network::LogStream(Websocket::frameoutput, "\nOPCODE: PONG");
                break;
            default:
                Network::LogStream(Websocket::frameoutput, "\nOPCODE: INVALID");
                return false;
        }

        // Output websocket payload length and the masking key used.
        unsigned long long payload_length = 0;
        switch (frame[1] & 0x7f) { 
            case 0 ... 125: // 0x7f = 01111111 

                Network::LogStream(
                    Websocket::frameoutput,
                    "\nPAYLOAD LENGTH: ", +frame[1] & 0x7f,
                    "\nMASKING KEY: ", +frame[2], ',',
                    +frame[3], ',', +frame[4], ',',
                    +frame[5], '\n'
                );
                break;

            case 126:

                Network::LogStream(
                    Websocket::frameoutput,
                    "\nPAYLOAD LENGTH: ", +frame[3],
                    "\nMASKING KEY: ", +frame[4], ',',
                    +frame[5], ',', +frame[6], ',',
                    +frame[7], '\n'
                );
                break;

            case 127:

                payload_length = (unsigned long long)frame[9] << 56;
                payload_length = (unsigned long long)frame[8] << 48;
                payload_length = (unsigned long long)frame[7] << 40;
                payload_length = (unsigned long long)frame[6] << 32;
                payload_length = (unsigned long long)frame[5] << 24;
                payload_length = (unsigned long long)frame[4] << 16;
                payload_length = (unsigned long long)frame[3] << 8;
                payload_length = (unsigned long long)frame[2] << 0;

                Network::LogStream(
                    Websocket::frameoutput,
                    "\nPAYLOAD LENGTH: ", payload_length,
                    "\nMASKING KEY: ", +frame[10], ',',
                    +frame[11], ',', +frame[12], ',',
                    +frame[13], '\n'
                );
                break;

            default:

                Network::LogStream(Websocket::frameoutput, "\nINCORRECT PAYLOAD LENGTH");
                std::cerr << Websocket::frameoutput.rdbuf();
                Websocket::frameoutput.flush();
                return false;
                
        }

    } else { return false; }

    if (this->Log_Robust) {
        std::cerr << Websocket::frameoutput.rdbuf();
        Websocket::frameoutput.flush();
    }
    return true;

}

void Websocket::InspectPacket(const char* packet) {

    std::bitset<8> fin_res(packet[0]);
    std::bitset<8> payloadlen(packet[1]);
    std::bitset<4> opcode(packet[0]);
    std::bitset<3> res;
    uint8_t mask[4];
    uint64_t len64;
    uint16_t len16;
    uint8_t len8;
    int fin;
    // Check FIN bit.
    if (fin_res.test(7)) {
        std::cout << "\n1... .... = Fin: True\n";
        fin = 1;
    } else {
        std::cout << "\n0... .... = Fin: False\n";
        fin = 0;
    }

    // Check reserved bits; all three should be 0.
    if (fin_res.test(6)) {
        res.set(0, 1);
    } else {
        res.set(0, 0);
    }

    // Check second reserved bit.
    if (fin_res.test(5)) {
        res.set(1, 1);
    } else {
        res.set(1, 0);
    }

    // Check second reserved bit.
    if (fin_res.test(4)) {
        res.set(2, 1);
    } else {
        res.set(2, 0);
    }

    // Print reserved bits info.
    std::cout << '.' << res.to_string() << " .... = Reserved: " << res.to_ulong() << '\n';

    // Check opcode.
    std::cout << ".... " << opcode << " = Opcode: " << opcode.to_ulong() << '\n';

    // Check mask bit and payload length.
    if (payloadlen.test(7)) {
        std::cout << "1... .... = Mask: True\n";
    } else {
        std::cout << "0... .... = Mask: False\n";
    }

    if ((packet[1] & 0x7f) < 126) {
        std::cout << '.' << std::bitset<3>(packet[1] >> 4) << ' ' << std::bitset<4>(packet[1]);
        std::cout << " = Payload length: " << (+packet[1] & 0x7f) << '\n';

        len8 = packet[1] & 0x7f;
        mask[0] = packet[2];
        mask[1] = packet[3];
        mask[2] = packet[4];
        mask[3] = packet[5];
        
        // Output masked payload.
        /*
        for (int i = 6, x = 0; i < len8 + 6; i++, x++) {
            std::cout << packet[i];
            if (x && x % 4 == 0) std::cout << ' ';
            if (x && x % 8 == 0) std::cout << '\n';
        }
        std::cout << '\n';

        // Output unmasked payload.
        for (int i = 6, m = 0; i < len8 + 6; i++, m++) {
            std::cout << (char)(mask[m % 4] ^ packet[i]);
            if (m && m % 4 == 0) std::cout << ' ';
            if (m && m % 8 == 0) std::cout << '\n';
        }
        std::cout << '\n';
        */

    } else if ((packet[1] & 0x7f) == 126) {
        len16 = (packet[3] << 8) | packet[2];
        std::cout << '.' << std::bitset<3>(packet[1] >> 4) << ' ' << std::bitset<4>(packet[1]) << '\n';
        std::cout << std::bitset<4>(packet[2] >> 4) << ' ' << std::bitset<4>(packet[2]) << '\n';
        std::cout << std::bitset<4>(packet[3] >> 4) << ' ' << std::bitset<4>(packet[3]);
        std::cout << " = Extended payload length: " << len16 << '\n';

        mask[0] = packet[4];
        mask[1] = packet[5];
        mask[2] = packet[6];
        mask[3] = packet[7];
        // Output masked payload then unmasked payload.
        /*
        for (int i = 8; i < len16 + 8; i++) {
            std::cout << packet[i];
            if (i % 4 == 0) std::cout << ' ';
            if (i % 8 == 0) std::cout << '\n';
        }
        for (int i = 0, x = 0; i < ((packet[2] << 8) | packet[3]); i++, x++) {
            std::cout << (char)(mask[x % 4] ^ packet[8 + i]);
            if (x && i % 4 == 0) std::cout << ' ';
            if (x && i % 8 == 0) std::cout << '\n';
        }
        */

    }

}

// Compose a websocket data frame with the input from the inputData parameter
// of the websocket class constructor  
// TODO: change malloc(strlen(data)) to an allocation that is far less volatile 
// than an arbitrary amount of bytes (strlen); one missing null-terminator and it's OVER
char* Websocket::compose_frame(Websocket::opcode_type type, char* input) {
    data = input;
    msg_size = strlen(data);
    const uint8_t masking_key[4] = { 0x12, 0x34, 0x56, 0x78 };

    // Allocate memory for where the encoded payload is to be stored
    // ==========
    // My failure to include "+ 1" in the call to malloc below for the NULL-byte, and 
    // also failing to zero out the whole buffer, has caused many websocket sends to 
    // result in an "invalid use of reserved bits" error from the server as well as 
    // some segmentation faults here and there. Essentially, data was being written
    // to where it shouldn't.
    // ==========
    char* enc_payload = (char*)malloc((strlen(data) + 1)*sizeof(char*));
    memset(enc_payload, 0, strlen(data) + 1);

    // the position to start moving data to after the payload length
    // calculation is finished.
    int offset;

    if (msg_size < 126) {
        offset = 6;
    } else if (msg_size < 65536) {
        offset = 8;
    } else { offset = 14; }

    char* frame = (char*)malloc((strlen(data) + offset + 1)*sizeof(char*));
    memset(frame, 0, strlen(data) + offset + 1);

    switch (type) {
        case Websocket::opcode_type::TEXT:
            frame[0] = 0x81;
            break;
        case Websocket::opcode_type::BINARY:
            frame[0] = 0x82;
            break;
        case Websocket::opcode_type::CLOSE:
            frame[0] = 8;
            break;
        case Websocket::opcode_type::PING:
            frame[0] = 0x89;
            break;
        case Websocket::opcode_type::PONG:
            frame[0] = 0xa;
            break;
        default:
            Network::Log(Websocket::LogSs, "compose_frame() failure: unknown opcode.", Network::Protocol::LOG_WEBSOCKET) << '\n';
            std::cerr << Websocket::LogSs.rdbuf();
            return NULL;
    }

    Network::LogStream(Websocket::frameoutput, "[WEBSOCKET PAYLOAD SIZE]\n");
    if (msg_size < 126) {

        // ==
        // The reason that 'frame[1] = (msg_size & 0xff) | 0x80' is for the mask bit;
        // because the client is required to mask the payload, the mask bit must 
        // always be set to 1. So, to do this while still maintaining the proper 7-bit
        // payload length, the payload size is bitwise-OR'd with 0x80 (10000000), so 
        // that the most significant bit, the mask bit, is set to 1 if it is 0, but no
        // other bit. 
        // ==
        frame[1] = (msg_size & 0xff) | 0x80;
        frame[2] = masking_key[0];
        frame[3] = masking_key[1];
        frame[4] = masking_key[2];
        frame[5] = masking_key[3];
        Network::LogStream(Websocket::frameoutput, std::bitset<8>(frame[1]), '\n');

    } else if (msg_size < 65535) {

        frame[1] = 126 | 0x80;
        frame[2] = (msg_size << 8) & 0xff;
        frame[3] = (msg_size << 0) & 0xff;
        Network::LogStream(Websocket::frameoutput, std::bitset<8>(frame[1]), '\n');
        Network::LogStream(Websocket::frameoutput, std::bitset<8>(frame[2]), '\n');
        Network::LogStream(Websocket::frameoutput, std::bitset<8>(frame[3]), '\n');

        // set the masking key after the length bytes
        frame[4] = masking_key[0];
        frame[5] = masking_key[1];
        frame[6] = masking_key[2];
        frame[7] = masking_key[3];

    } else {

        frame[1] = 127 | 0x80;
        frame[2] = (msg_size >> 56) & 0xff;
        frame[3] = (msg_size >> 48) & 0xff;
        frame[4] = (msg_size >> 40) & 0xff;
        frame[5] = (msg_size >> 32) & 0xff;
        frame[6] = (msg_size >> 24) & 0xff;
        frame[7] = (msg_size >> 16) & 0xff;
        frame[8] = (msg_size >> 8) & 0xff;
        frame[9] = (msg_size >> 0) & 0xff;

        // set the masking key after the 8 bytes for length
        frame[10] = masking_key[0];
        frame[11] = masking_key[1];
        frame[12] = masking_key[2];
        frame[13] = masking_key[3];

    }

    // mask the payload data with the masking key
    for (int i = 0; i < msg_size; i++) {

        enc_payload[i] = data[i] ^ masking_key[i % 4];
    
    } 

    enc_payload[strlen(data) + 1] = 0x0;

    // Transfer information from data to frame to make a complete websocket data frame. There
    // are better ways to do this (+1 for null-byte).
    /*for (int i = 0, x = offset; i <= strlen(enc_payload), x <= strlen(enc_payload) + offset + 1; i++, x++) {

        frame[x] = enc_payload[i];

    }*/
    memcpy(&frame[offset], &enc_payload[0], strlen(data) * sizeof(char*));

    /*
    std::cout << enc_payload << '\n';
    for (int i = offset, m = 0; i < msg_size; i++, m++) {
        std::cout << (char)(masking_key[m % 4] ^ frame[i]);
        if (m && m % 4 == 0) std::cout << ' ';
        if (m && m % 8 == 0) std::cout << '\n';
    }
    */

    free(enc_payload);

    if (this->Log_Robust) {
        std::cerr << Websocket::frameoutput.rdbuf();
        Websocket::frameoutput.flush();
    }
    
    // Outputs information on a given websocket frame header.
    //Websocket::InspectPacket(frame);

    return frame;
}

// This function parses the payload of a SERVER-SENT websocket packet; a little simpler
// when compared to parsing a client-sent packet. Returns NULL if recBuffer is NULL.
char* Websocket::parse_payload(char* recBuffer) {

    // instead of using 'x' for the len variable i could instead just put 'recBuffer[1] /OFFSET/' right inside of 
    // 'output[x] = '\0' but i didn't
    if (recBuffer == NULL) {
        Network::Log(Websocket::LogSs, "parse_payload failure: buffer is null.", Network::Protocol::LOG_WEBSOCKET) << '\n';
        std::cerr << Websocket::LogSs.rdbuf();
        return NULL;
    }
    char* output;
    if (recBuffer[1] < 126) {
        output = (char*)malloc(sizeof(char*) * recBuffer[1]);
        for (int i = 2, x = recBuffer[1] + 2, y = 0; i < x; i++, y++) { // 'x = recBuffer[1] + 2;' "+2" for the offset, 'i'
            output[y] = recBuffer[i];
            output[x] = '\0';
        }
    } else if (recBuffer[1] < 65536) {
        output = (char*)malloc(sizeof(char*) * recBuffer[1]);
        for (int i = 4, x = recBuffer[1] + 4, y = 0; i < x; i++, y++) {
            output[y] = recBuffer[i];
            output[x] = '\0';
        }
    } else if (recBuffer[1] == 127) {
        output = (char*)malloc(sizeof(char*) * recBuffer[1]);
        for (int i = 10, x = recBuffer[1] + 10, y = 0; i < x; i++, y++) {
            output[y] = recBuffer[i];
            output[x] = '\0';
        }
    } else { return NULL; }

    return output;
}

bool Websocket::sendData(Websocket::opcode_type type, char* input) {

    char* m_data;

    if (type == Websocket::opcode_type::TEXT) {

        m_data = Websocket::compose_frame(Websocket::opcode_type::TEXT, input);

    } else if (type == Websocket::opcode_type::BINARY) {

        m_data = Websocket::compose_frame(Websocket::opcode_type::BINARY, input);

    } else {

        return false;

    }

    if (!checkFrame(m_data)) {

        Network::Log(Websocket::LogSs, "sendData():checkFrame(): invalid frame.", Network::Protocol::LOG_WEBSOCKET) << '\n';
        std::cerr << Websocket::LogSs.rdbuf();
        std::cerr << Websocket::frameoutput.rdbuf();
        Websocket::frameoutput.flush();
        return false;

    }

    try {

        Websocket::Http->GetSocketClass()->Send((const char*)m_data);

    } catch (const std::exception& e) {

        Network::Log(Websocket::LogSs, "sendData exception: ", Network::Protocol::LOG_WEBSOCKET) << e.what() << '\n';
        std::cerr << Websocket::LogSs.rdbuf();

    }
    if (m_data) free(m_data);

    return true;
}

// Overload for std::string type input.
bool Websocket::sendData(Websocket::opcode_type type, std::string input) {

    char* m_data;

    if (type == Websocket::opcode_type::TEXT) {

        m_data = Websocket::compose_frame(Websocket::opcode_type::TEXT, (char*)input.c_str());

    } else if (type == Websocket::opcode_type::BINARY) {

        m_data = Websocket::compose_frame(Websocket::opcode_type::BINARY, (char*)input.c_str());

    } else {

        return false;

    }

    if (!checkFrame(m_data)) {

        Network::Log(Websocket::LogSs, "sendData():checkFrame(): invalid frame.", Network::Protocol::LOG_WEBSOCKET) << '\n';
        std::cerr << Websocket::LogSs.rdbuf();
        std::cerr << Websocket::frameoutput.rdbuf();
        Websocket::frameoutput.flush();
        return false;

    }

    try {

        Websocket::Http->GetSocketClass()->Send((const char*)m_data);

    } catch (const std::exception& e) {

        Network::Log(Websocket::LogSs, "sendData exception: ", Network::Protocol::LOG_WEBSOCKET) << e.what() << '\n';
        std::cerr << Websocket::LogSs.rdbuf();

    }
    if (m_data) free(m_data);

    return true;
}

bool Websocket::sendData(Websocket::opcode_type type) {
    char* data;

    if (type == Websocket::opcode_type::CLOSE) {

        try {

            data = Websocket::compose_frame(Websocket::opcode_type::CLOSE, (char*)"close");
            Websocket::Http->GetSocketClass()->Send((const char*)data);

        } catch (const std::exception& e) {

            Network::Log(Websocket::LogSs, "sendData() exception: ", Network::Protocol::LOG_WEBSOCKET) << e.what() << '\n';
            std::cerr << Websocket::LogSs.rdbuf();

        }

    } else if (type == Websocket::opcode_type::PING || type == Websocket::opcode_type::PONG) {

        try {

            data = Websocket::compose_frame(Websocket::opcode_type::CLOSE, (char*)"ping");
            Websocket::Http->GetSocketClass()->Send((const char*)data);

        } catch (const std::exception& e) {

            Network::Log(Websocket::LogSs, "sendData() exception: ", Network::Protocol::LOG_WEBSOCKET) << e.what() << '\n';
            std::cerr << Websocket::LogSs.rdbuf();

        }

    } else { return false; }

    return true;
}

// max size of the buffer to receive is 512 bytes; it shouldn't need to be larger
char* Websocket::receiveData() {
    char* rec_buf = (char*)malloc(sizeof(char*) * 512);
    memset(rec_buf, 0, 512);
    int rec;

    do {

        rec = recv(Websocket::web_socket, rec_buf, 512, 0);
        if (rec > 0) {
            return rec_buf;
        }
        if (rec <= 0) {

            // Receiving 0 bytes does not always mean that the connection has been closed; leaving this commented out for now.
            //Network::Log(Websocket::LogSs, "receiveData(): connection has been gracefully closed.", Network::Protocol::LOG_WEBSOCKET) << '\n';
            //std::cerr << Websocket::LogSs.rdbuf();

        } else if (rec == SOCKET_ERROR) {

            Network::Log(Websocket::LogSs, "sendData() error: recv() Winsock error ", Network::Protocol::LOG_WEBSOCKET) << WSAGetLastError() << '\n';
            std::cerr << Websocket::LogSs.rdbuf();
            break;

        }
    } while (rec > 0);

    return NULL;
}

/*char* Websocket::GetCommandBuffer(Command* CommandInstance) {

    char* cmd;
    Websocket::CommandBuf = CommandInstance;
    cmd = Websocket::CommandBuf->Buffer;
    return cmd;

}*/

// Function for looping receiving data on an established socket; for use in a separate thread.
void Websocket::threadedOutput() {
    
    std::ofstream binfile;
    char* output;
    while (Input::LoopEnd == false) {

        output = Websocket::receiveData();
        if (output) {   
            
            // If DumpBinary is true, dump each server packet to one .bin file.
            if (Websocket::DumpBinary) {

                binfile = std::ofstream("output_binary_dump.bin", std::ofstream::app | std::ofstream::binary);
                if (binfile.fail()) {

                    binfile.clear();
                    binfile.write(output, strlen(output) + 1);
                
                } else {
                    
                    binfile.write(output, strlen(output) + 1);
                
                }

            } else {

                if (binfile.is_open()) {
                    binfile.close();
                }

            }
            printf("\e7"); // Save cursor position.
            printf("\e[G"); // Move to beginning of line.
            printf("\e[K");  // Clear line beginning from cursor.
            //printf("%s\n", output);
            printf("\e[G"); // Move to beginning of line
            //std::cout << Input::input_buf;
            printf("\e8"); // Restore cursor position from memory
            // Command::Buffer is static.
            //printf("%s", Command::Buffer);

            /*
            lg::lg_sinfo << output << '\n';
            lg::lg_outf(lg::lg_type::LG_INFO);
            */

        } 
        free(output);

    }
    binfile.close();

}


