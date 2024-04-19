#ifndef RCON_H
#define RCON_H

#include <RconCLI/json/json.h>
#include <RconCLI/logging.h>
#include <RconCLI/websockets.h>
#include <RconCLI/cmd.h>
#include <string>
#include <thread>
#include <atomic>

namespace Dewrito {

static lg::lg_logstream logstream(lg::lg_logstream::lg_type::LG_ERROR, std::cerr);

class Rcon {
    private:
    enum Keycode {

        F5 = 0x3F // connection refresh.

    };
    static void keycb_f5(Command& cmd);

    public:
    Websocket* Websock;
    Command::key_callback GetCallback();
    bool BinDump = false;
    bool Fail = false;
    bool FailWebsocket = false;
    bool RobustLog = false;
    const char* Address;
    const char* Password;
    long int RconPort;
    bool ReadConfig();
    bool CreateWebsocket();
    void RconLoop();
    Rcon();

};

class GetRcon {

    public:
    static Rcon* rconref;
    static Rcon*& GetRconInstance();

};

}

#endif