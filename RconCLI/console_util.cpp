#include <RconCLI/console_util.h>

namespace WindowsConsole {

Console& Console::Size() {

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    this->columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    this->rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return *this;

}

void Console::GetConsoleSize() {

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    this->columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    this->rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

}

}