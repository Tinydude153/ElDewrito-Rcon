#ifndef RCON_H
#define RCON_H

#include <RconCLI/logging.h>
#include <RconCLI/websockets.h>
#include <RconCLI/cmd.h>
#include <RconCLI/config.h>
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
    Config* Cfg;
    Command::key_callback GetCallback();
    bool BinDump = false;
    bool Fail = false;
    bool FailWebsocket = false;
    bool RobustLog = false;
    std::string Address;
    std::string Password;
    std::string RconPort;
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