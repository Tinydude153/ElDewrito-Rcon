#include <RconCLI/rcon.h>

namespace Dewrito {

// Intermediary class definitions for getting a reference to this Rcon instance for callbacks.
// Both static.
Rcon* GetRcon::rconref;

Rcon*& GetRcon::GetRconInstance() {

    return GetRcon::rconref;

}

Rcon::Rcon() {

    SetConsoleTitle("RconCLI - Reading config...");
    if (Rcon::ReadConfig()) {

        SetConsoleTitle("RconCLI - Connecting...");
        if (Rcon::CreateWebsocket()) {

            Rcon::Fail = false;
            Rcon::FailWebsocket = false;
            if (Rcon::RconMode == Rcon::Mode::RCON) {
                SetConsoleTitle("RconCLI - Connected : Mode - RCON");
            } else { SetConsoleTitle("RconCLI - Connected : Mode - SYSTEM"); }

        } else { 
            // Rcon::Fail is not set because the program should continue running, just in a disconnected state.
            Rcon::FailWebsocket = true; 
            if (Rcon::RconMode == Rcon::Mode::RCON) {
                SetConsoleTitle("RconCLI - Disconnected : Mode - RCON");
            } else { SetConsoleTitle("RconCLI - Disconnected : Mode - SYSTEM"); }
        } 

    } else { Rcon::Fail = true; }

}

bool Rcon::ReadConfig() {

    Rcon::Cfg = new Config("config/config.cfg");
    Rcon::Password = Rcon::Cfg->GetValue("password");
    Rcon::Address = Rcon::Cfg->GetValue("address");
    Rcon::RconPort = Rcon::Cfg->GetValue("rcon_port");
    if (Rcon::Password.empty()) {
        Rcon::Fail = true;
        std::cerr << "[CONFIG] Password failed.\n";
        return false;   
    }
    if (Rcon::Address.empty()) {
        Rcon::Fail = true;
        std::cerr << "[CONFIG] IP address failed.\n";
        return false;  
    }
    if (Rcon::RconPort.empty()) {
        Rcon::Fail = true;
        std::cerr << "[CONFIG] Port failed.\n";
        return false;  
    }

    std::string announceserv = Rcon::Cfg->GetValue("run_as_announce_service_only");
    if (announceserv.compare("true") == 0) {
        Rcon::AnnounceServiceOnly = true;
    }

    std::string bindump = Rcon::Cfg->GetValue("binary_dump");
    if (bindump.compare("true") == 0) {
        Rcon::BinDump = true;
    }

    return true;

}

bool Rcon::CreateWebsocket() {

    try {

        Rcon::Websock = new Websocket(Rcon::Address.c_str(), (int)(strtol(Rcon::RconPort.c_str(), NULL, 10)), "dew-rcon");
        Rcon::Websock->sendData(Rcon::Websock->opcode_type::TEXT, (char*)Rcon::Password.c_str());
        char* wsRec = Rcon::Websock->receiveData();
        char* wsPay = Rcon::Websock->parse_payload(wsRec);
        if (strncmp((const char*)wsPay, "accept", strlen("accept"))) {
            printf("\nThe websocket connection was not accepted.\n"); 
            return false;
        }

    } catch (const std::exception& e) {

        std::cout << '[' <<__FUNCTION__ << "] " << e.what() << '\n';
        std::cerr << '[' <<__FUNCTION__ << "] " << e.what() << '\n';
        return false;

    }

    return true;
}

// Refreshed RCON connection.
void Rcon::keycb_f5(Command& cmd) {

    cmd.SaveBuffer() = true;
    SetConsoleTitle("RconCLI - Connecting...");
    if (GetRcon::GetRconInstance()->FailWebsocket) {

        if (GetRcon::GetRconInstance()->CreateWebsocket()) {
            
            if (GetRcon::GetRconInstance()->RconMode == Rcon::Mode::RCON) {
                SetConsoleTitle("RconCLI - Connected : Mode - RCON");
            } else { SetConsoleTitle("RconCLI - Connected : Mode - SYSTEM"); }
            GetRcon::GetRconInstance()->Fail = false;
            GetRcon::GetRconInstance()->FailWebsocket = false;

        } else { 
            if (GetRcon::GetRconInstance()->RconMode == Rcon::Mode::RCON) {
                SetConsoleTitle("RconCLI - Disconnected : Mode - RCON");
            } else { SetConsoleTitle("RconCLI - Disconnected : Mode - SYSTEM"); }
            GetRcon::GetRconInstance()->FailWebsocket = true; 
        }

    } else { 

        if (GetRcon::GetRconInstance()->RconMode == Rcon::Mode::RCON) {
            SetConsoleTitle("RconCLI - Connected : Mode - RCON");
        } else { SetConsoleTitle("RconCLI - Connected : Mode - SYSTEM"); }

    }

}

void Rcon::keycb_f4(Command& cmd) {

    if (GetRcon::GetRconInstance()->RconMode == Rcon::Mode::RCON) {
        GetRcon::GetRconInstance()->RconMode = Rcon::Mode::SYSTEM;
        if (GetRcon::GetRconInstance()->FailWebsocket) {
            SetConsoleTitle("RconCLI - Disconnected : Mode - SYSTEM");
        } else {
            SetConsoleTitle("RconCLI - Connected : Mode - SYSTEM");
        }
    } else if (GetRcon::GetRconInstance()->RconMode == Rcon::Mode::SYSTEM) {
        GetRcon::GetRconInstance()->RconMode = Rcon::Mode::RCON;
        if (GetRcon::GetRconInstance()->FailWebsocket) {
            SetConsoleTitle("RconCLI - Disconnected : Mode - RCON");
        } else {
            SetConsoleTitle("RconCLI - Connected : Mode - RCON");
        }
    }

}

void Rcon::RconLoop() {

    std::thread receiveThread;
    if (!Rcon::FailWebsocket) receiveThread = std::thread(&Websocket::threadedOutput, &*Rcon::Websock);

    // Binary server output dump in config is true.
    if (!Rcon::FailWebsocket) {
        if (Rcon::BinDump) {
            Rcon::Websock->DumpBinary = true;
        }
        if (Rcon::RobustLog) {
            Rcon::Websock->Log_Robust = true;
        }
    }
    
    // Start command-line parsing.
    Command Cmd;

    // My function pointer implementation for callbacks is disgusting but this at least works so I'm done for now lol
    GetRcon::rconref = this;
    Command::key_callback funcptr_f5 = &Rcon::keycb_f5;
    Command::key_callback funcptr_f4 = &Rcon::keycb_f4;
    Cmd.CustomKey(Rcon::Keycode::F5, funcptr_f5);
    Cmd.CustomKey(Rcon::Keycode::F4, funcptr_f4);
    if (!Rcon::FailWebsocket) {
        Rcon::Websock->GetCommandBuffer(&Cmd);
    }
    while (Cmd.Active) {

        // Initialize command input.
        if (!Cmd.Initialize()) {

            printf("\nERROR: Command::Initialize(): FALSE");
            return;

        }

        // Begin the input loop.
        Cmd.Input();

        // Send over websocket connection.
        if (Cmd.Send) {

            switch (Rcon::FailWebsocket) {

                case true:
                    printf("\nSend failed: disconnected.\n");
                    break;
                case false:
                    if (!Rcon::Websock->sendData(Rcon::Websock->opcode_type::TEXT, Cmd.Buffer)) {

                        printf("\nFailed to send buffer\n");
                        continue;

                }

            }

        }

    }
    if (!Rcon::FailWebsocket) Rcon::Websock->sendData(Rcon::Websock->CLOSE);

    /*
    // Keeping the manually-called destructor commented out below as a monument of my misunderstanding; I did
    // not know that destructors are called automatically when objects exit scope (unless constructed with new,
    // then they must be deleted manually).
    */
    //Cmd.~Command();

    delete Rcon::Cfg;
    if (!Rcon::FailWebsocket) {
        closesocket(Rcon::Websock->GetSocket());
        delete Rcon::Websock;
    }
    WSACleanup();
    if (!Rcon::FailWebsocket) receiveThread.join();

}

void Rcon::RconAnnounceLoop() {

    std::thread receiveThread;
    if (!Rcon::FailWebsocket) receiveThread = std::thread(&Websocket::threadedOutput, &*Rcon::Websock);

    // Binary server output dump in config is true.
    if (!Rcon::FailWebsocket) {
        if (Rcon::BinDump) {
            Rcon::Websock->DumpBinary = true;
        }
        if (Rcon::RobustLog) {
            Rcon::Websock->Log_Robust = true;
        }
    }

    Config AnnounceCfg("config/announce.cfg");
    std::string Msg = AnnounceCfg.GetValue("server_0");
    char* msg_ws = (char*)Msg.c_str();

    if (!Msg.empty()) {

        while (true) {

            if (!Rcon::FailWebsocket) {
                Rcon::Websock->sendData(Rcon::Websock->opcode_type::TEXT, (char*)Msg.c_str());
            }
            std::this_thread::sleep_for(std::chrono::minutes(5));

        }

    }

}

}