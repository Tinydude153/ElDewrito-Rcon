#include <RconCLI/input.h>

std::atomic<bool> Input::LoopEnd = false;
std::string Input::input_buf;
std::condition_variable Input::WaitCondition;
std::mutex Input::WaitConditionMutex;
std::atomic<int> Input::atbool {0};

void sigterm_handler(int sig) {

    std::cerr << "[SIGINT] RconCLI terminated.\n";
    exit(1);
    Input::LoopEnd = true;

}

#if _WIN32
BOOL WINAPI WindowsConsoleHandler(DWORD dwType) {
    
    switch(dwType) {
        case CTRL_C_EVENT:
            std::cerr << "[SIGINT] RconCLI terminated.\n";
            exit(1);
            Input::LoopEnd = true;
            std::cin.setstate(std::ios_base::eofbit);
            break;
        case CTRL_BREAK_EVENT:
            Input::LoopEnd = true;
            std::cin.setstate(std::ios_base::eofbit);
            break;
        default:
            break;
    }
    return TRUE;

}
#endif

void Input::SetConsoleSignal() {

    #if _WIN32
        if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)WindowsConsoleHandler,TRUE)) {
            std::cerr << "[ERROR] Unable to install WindowsConsoleHandler(DWORD).\n";
        }
    #else
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = sigterm_handler;
        sigaction(SIGTERM | SIGINT, &sa, NULL);
    #endif

}

void Input::InputLoop() {

    #if _WIN32
        if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)WindowsConsoleHandler,TRUE)) {
            std::cerr << "[ERROR] Unable to install WindowsConsoleHandler(DWORD).\n";
        }
    #else
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = sigterm_handler;
        sigaction(SIGTERM | SIGINT, &sa, NULL);
    #endif

    this->SendReady = false;
    Input::input_buf.clear();
    printf("\e[%dd", Input::MainConsole.Size().rows);
    std::getline(std::cin, Input::input_buf); // <string>
    if (Input::input_buf.compare("quit") == 0) {
        Input::LoopEnd = true;
    }
    this->SendReady = true;

}