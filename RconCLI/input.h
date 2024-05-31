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
#include <condition_variable>
#include <mutex>
#include <chrono>
#define _GNU_SOURCE

class Input {

    public:
    WindowsConsole::Console MainConsole;
    bool SendReady = false;
    static std::string input_buf;
    static std::atomic<bool> LoopEnd;
    static std::condition_variable WaitCondition;
    static std::mutex WaitConditionMutex;
    static std::unique_lock<std::mutex> lck;
    static std::atomic<int> atbool;
    void InputLoop();
    static void SetConsoleSignal();

};

#endif