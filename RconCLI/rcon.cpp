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
        if (Rcon::AnnounceServiceOnly == false) {
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
        }

    } else { Rcon::Fail = true; }

}

bool Rcon::ReadConfig() {

    Rcon::Cfg = new Config("config/config.cfg");
    Rcon::Password = Rcon::Cfg->GetValue("Rcon.Password");
    Rcon::Address = Rcon::Cfg->GetValue("Server.Address");
    Rcon::RconPort = Rcon::Cfg->GetValue("Rcon.Port");
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

    std::string announceserv = Rcon::Cfg->GetValue("Rcon.TimedAnnounce");
    if (announceserv.compare("true") == 0) {
        Rcon::AnnounceServiceOnly = true;
    }

    std::string bindump = Rcon::Cfg->GetValue("Rcon.System.DumpWebsocketBinary");
    if (bindump.compare("true") == 0) {
        Rcon::BinDump = true;
    }

    return true;

}

bool Rcon::ReadSos() {

    Rcon::Sos = new SOSerial;
    Sos->Deserialize("config/servers.sos");
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
    Input Terminal;

    // My function pointer implementation for callbacks is disgusting but this at least works so I'm done for now lol
    GetRcon::rconref = this;
    Command::key_callback funcptr_f5 = &Rcon::keycb_f5;
    Command::key_callback funcptr_f4 = &Rcon::keycb_f4;
    Cmd.CustomKey(Rcon::Keycode::F5, funcptr_f5);
    Cmd.CustomKey(Rcon::Keycode::F4, funcptr_f4);
    /*
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
    */

    while (Terminal.LoopEnd == false) {
        Terminal.InputLoop();
        if (Terminal.SendReady == true) {
            Rcon::Websock->sendData(Rcon::Websock->opcode_type::TEXT, (char*)Terminal.input_buf.c_str());
            Terminal.SendReady = false;
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

void Rcon::RconWebsocketThread(Rcon::annsrv announce_server) {

    try {

        Websocket ws(announce_server.address, announce_server.port, announce_server.protocol);
        ws.sendData(Websocket::opcode_type::TEXT, announce_server.password);
        char* wsRec = Rcon::Websock->receiveData();
        char* wsPay = Rcon::Websock->parse_payload(wsRec);
        if (strncmp((const char*)wsPay, "accept", strlen("accept"))) {
            return;
        }

        while (true) {
            
            ws.sendData(Websocket::opcode_type::TEXT, (char*)announce_server.msg.c_str());
            std::this_thread::sleep_for(std::chrono::minutes(announce_server.interval));

        }
    } catch (const std::exception& e) {

        std::cerr << '[' <<__FUNCTION__ << "] " << e.what() << '\n';
        return;

    }

}

void Rcon::RconAnnounceLoop() {

    //std::thread receiveThread;
    //if (!Rcon::FailWebsocket) receiveThread = std::thread(&Websocket::threadedOutput, &*Rcon::Websock);

    Rcon::Sos = new SOSerial;
    Rcon::Sos->Deserialize("config/servers.sos");

    for (std::map<std::string, std::map<std::string, std::string>>::iterator it = Rcon::Sos->m_SerializationMap.begin(); it != Rcon::Sos->m_SerializationMap.end(); it++) {

        try {
            std::string rconpass = Rcon::Sos->GetValue(it->first, "Server.RconPassword");
            std::string rconport = Rcon::Sos->GetValue(it->first, "Server.RconPort");
            std::string rconaddr = Rcon::Sos->GetValue(it->first, "Server.Address");
            Rcon::WSConnections.insert(std::pair<std::string, Websocket*>(it->first, new Websocket(rconaddr.c_str(), (int)(strtol(rconport.c_str(), NULL, 10)), "dew-rcon")));
            Rcon::WSConnections.at(it->first)->sendData(Rcon::Websock->opcode_type::TEXT, (char*)rconpass.c_str());
            char* wsRec = Rcon::WSConnections.at(it->first)->receiveData();
            char* wsPay = Rcon::WSConnections.at(it->first)->parse_payload(wsRec);
            if (strncmp((const char*)wsPay, "accept", strlen("accept"))) {
                // Remove connection if not accepted.
                std::cerr << "[ERROR] Password not accepted on " << it->first << "; ignoring.\n";
                Rcon::WSConnections.at(it->first)->sendData(Rcon::Websock->opcode_type::CLOSE);
                Rcon::WSConnections.erase(it->first);
            }
        } catch (const std::exception& e) {
            std::cerr << '[' << __FUNCTION__ << "()] " << e.what() << '\n';
            std::cerr << "[INFO] " << it->first << " failed to connect.\n";
        }

    }

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

    // Set console signals.
    Input::SetConsoleSignal();

    // Currently, the thread simply sleeps until a given time interval then continues the loop, then repeat.
    // Therefore, to stop the program at this time, a kill signal must be sent. Hopefully I can make this better at some point.
    if (!Rcon::WSConnections.empty()) {

        while (Input::LoopEnd == false) {

            for (std::map<std::string, Websocket*>::iterator it = Rcon::WSConnections.begin(); it != Rcon::WSConnections.end(); it++) {
                it->second->sendData(Rcon::Websock->opcode_type::TEXT, (char*)AnnounceCfg.GetValue(it->first).c_str());
            }
            std::this_thread::sleep_for(std::chrono::minutes(3));

            //Input::WaitCondition.wait_until(lk, now + std::chrono::minutes(5));

        }


    }

}

} // End namespace.