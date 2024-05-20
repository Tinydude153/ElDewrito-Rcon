#ifndef RCON_H
#define RCON_H

#include <RconCLI/logging.h>
#include <RconCLI/websockets.h>
#include <RconCLI/config.h>
#include <RconCLI/sos/sos.h>
#include <RconCLI/input.h>
#if _WIN32
    #include <RconCLI/cmd.h> // Heavy Windows-based file; Linux implementation will not require this.
#endif
#include <chrono>
#include <ratio>
#include <string>
#include <thread>
#include <atomic>
#include <map>

namespace Dewrito {

static lg::lg_logstream logstream(lg::lg_logstream::lg_type::LG_ERROR, std::cerr);

class Rcon {
    private:
    enum Keycode {

        F5 = 0x3F, // Connection refresh.
        F4 = 0x3E // Change mode.

    };
    static void keycb_f5(Command& cmd);

    enum Mode {

        RCON,
        SYSTEM

    };
    static void keycb_f4(Command& cmd); // Switched input mode between the two modes.

    public:
    struct annsrv {
        std::string msg;
        const char* address = 0;
        int port;
        std::string protocol;
        char* password;
        int interval = 5;
    };
    Websocket* Websock;
    Config* Cfg;
    SOSerial* Sos;
    std::map<std::string, Websocket*> WSConnections; // This is only used if AnnounceServiceOnly is true.
    Command::key_callback GetCallback();
    bool BinDump = false;
    bool Fail = false;
    bool FailWebsocket = false;
    bool RobustLog = false;
    Mode RconMode = Mode::RCON;
    std::string Address;
    std::string Password;
    std::string RconPort;
    bool AnnounceServiceOnly = false;
    bool ReadConfig();
    bool ReadSos();
    bool CreateWebsocket();
    void RconAnnounceLoop();
    void RconLoop();
    void RconWebsocketThread(annsrv announce_server);
    Rcon();

};

class GetRcon {

    public:
    static Rcon* rconref;
    static Rcon*& GetRconInstance();

};

}

#endif