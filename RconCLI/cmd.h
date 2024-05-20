#ifndef CMD_H
#define CMD_H

#include <RconCLI/cache.h>
#include <RconCLI/console_util.h>

#include <iostream>
#include <string>
#include <conio.h>
#include <mutex>
#include <atomic>
#include <vector>
#include <algorithm>
#include <map>
#if _WIN32
    #include <Windows.h>
#endif

class Command {
    private:

    typedef enum key_value {

        CTRL_C = 0x03,
        BACKSPACE = 0x08,
        CARRIAGE_RETURN = 0x0D,
        ARROW_LEFT = 0x4B,
        ARROW_UP = 0x48,
        ARROW_RIGHT = 0x4D,
        ARROW_DOWN = 0x50

    } KEY_VALUE;

    Cache* CmdCache;
    // Windows console API functions.
    WindowsConsole::Console MainConsole;

    // To alter the functionality of Initialize(); if true, it skips certain 0-assignments in
    // Initialize().
    bool savebuffer = false;

    // Windows console API information
    #if _WIN32
        HANDLE hStdout;
        CONSOLE_SCREEN_BUFFER_INFO CSBI;
    #endif

    // Key processing functions.
    void ArrowLeft(char character);
    void ArrowUp(char character);
    void ArrowRight(char character);
    void ArrowDown(char character);

    // Processes the enter key (return).
    void CarriageReturn(char character);
    // Processes the backspace key.
    void Backspace(char character);
    // Processes all basic alpha-numeric keys.
    void StandardKey(char character);
    // Print current static console information.
    void Update();
    void Update(const char* input);
    void Update(char input);

    // Print debug.
    void PrintInformation();

    public:

    // Callback function typedef that takes a reference to a Command instance; this is 
    // meant to take the current object, but it's not necessary.
    typedef void(*key_callback)(Command&);

    // std::map that holds a const int for a keycode, and a function pointer to a callback function.
    std::map<const int, key_callback> callback_map;
    static char* Buffer; // Static for global access.
    char Character;
    int Cursor;

    // Flags.
    static std::atomic<bool> Active; // Input active flag.
    bool Send = false;  // Whether to send on socket or not flag.
    bool EOS = true; // End of String: whether cursor is at the end or not; for ARROW_LEFT / ARROW_RIGHT. 

    char* CommandCache;

    bool& SaveBuffer();
    void CustomKey(const int code, key_callback callbackfunc);
    void AddKey(int code);
    void Input();
    bool Initialize();

    Command();
    ~Command();

};

#endif 