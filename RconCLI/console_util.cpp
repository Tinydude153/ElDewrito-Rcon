#include <RconCLI/console_util.h>

namespace WindowsConsole {

Console& Console::Size() {

    #if _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        this->columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        this->rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        return *this;
    #else
        struct winsize wsize;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
        this->columns = (int)wsize.ws_col;
        this->rows = (int)wsize.ws_row;
        return *this;
    #endif

}

void Console::GetConsoleSize() {

    #if _WIN32 
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        this->columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        this->rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    #else
        struct winsize wsize;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
        this->columns = (int)wsize.ws_col;
        this->rows = (int)wsize.ws_row;
    #endif

}

}