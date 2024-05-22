#ifndef INPUT_H
#define INPUT_H

#include <RconCLI/console_util.h>

#if _WIN32
    #include <Windows.h>
#endif
#include <string>
#include <iostream>
#include <signal.h>
#include <atomic>
#include <cstring>
#define _GNU_SOURCE

class Input {

    public:
    WindowsConsole::Console MainConsole;
    bool SendReady = false;
    static std::string input_buf;
    static std::atomic<bool> LoopEnd;
    void InputLoop();
    static void SetConsoleSignal();

};

#endif