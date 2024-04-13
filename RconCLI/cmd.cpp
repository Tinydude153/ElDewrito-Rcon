#include <RconCLI/cmd.h>

char* Command::Buffer;
std::atomic<bool> Command::Active = true;

Command::Command() {

    // Set Windows console information; primarily used for cursor positioning.
    Command::hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwMode = 0;
    if (!GetConsoleMode(hStdout, &dwMode))
    {
        printf("\nERROR: Command::Command(): GetConsoleMode(): %d\n", GetLastError());
        Command::Active = false;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hStdout, dwMode))
    {
        printf("\nERROR: Command::Command(): SetConsoleMode(): %d\n", GetLastError());
        Command::Active = false;
    }

    if (!GetConsoleScreenBufferInfo(Command::hStdout, &CSBI)) {

        printf("\nERROR: Command::Command(): GetConsoleScreenBufferInfo(): %d\n", GetLastError());
        Command::Active = false;

    }

    // Cache class instantiation.
        // TODO: De-hardcode the cache size.
    Command::CmdCache = new Cache(10);

    // Initialize cursor integer
    Command::Cursor = 0;

    // Allocate global command buffer.
    Command::Buffer = (char*)malloc(sizeof(char*) * 512);

}

Command::~Command() {

    // Delete cache class instance.
    delete Command::CmdCache;

    // Free allocated memory.
    if (Command::Buffer) free(Command::Buffer);

}

// Returns a reference to the savebuffer boolean which determines whether the buffer and cursor position is 
// to be saved when Initialize() is called.
bool& Command::SaveBuffer() {

    return Command::savebuffer;

}

// This function initializes significant variables; this is to be called
// before every call to Command::Input(). 
bool Command::Initialize() {

    // Set send flag to false.
    Command::Send = false;

    // If savebuffer is true, set savebuffer to false, then return true to skip the following assignments.
    if (savebuffer) {

        Command::savebuffer = false;
        return true;

    }

    // Allocate the command buffer.
        // TODO: Global enumeration for different buffer sizes.
    //Command::Buffer = (char*)malloc(sizeof(char*) * 64);
    memset(Command::Buffer, 0, 512);

    // Set cursor position to 0.
    Command::Cursor = 0;

    // Set character char to 0.
    Command::Character = 0;

    // Value check.
    if (Command::Cursor != 0) return false;
    if (Command::Character != 0) return false;
    if (Command::Buffer[0] != 0x0) return false; // Unreliable way of checking buffer but simple enough for now

    return true;
}

void Command::ArrowLeft(char character) {

    if (Command::Cursor == 0) return;
    if (Command::Cursor > 0) {

        Command::Cursor--;
        Command::EOS = false;

    } else { return; }
    printf("\e[1D"); // Shift cursor back by 1.
    printf("\e7");   // Save cursor position.

}

void Command::ArrowUp(char character) {

    Command::CommandCache = Command::CmdCache->Reverse();
    // Keep the line clear to indicate that the cache has been exhausted.
    if (!Command::CommandCache) {
        
        printf("\e[G"); // Move cursor to beginning.
        printf("\e[K"); // Clear line beginning from cursor.
        memset(Command::Buffer, 0x0, 512);
        Command::Cursor = 0;
        return;

    }

    memcpy(Command::Buffer, Command::CommandCache, strlen(Command::CommandCache) + 1);
    Command::Cursor = strlen(Command::CommandCache) - 1; // Minus 1 for the following increment, so cursor=strlen(buffer).

    if (Command::Buffer) {

        printf("\e[G"); // Move cursor to beginning.
        printf("\e[K"); // Clear line beginning from cursor.
        printf("%s", Command::Buffer);

    }

}

void Command::ArrowRight(char character) {

    if (Command::Cursor == strlen(Command::Buffer)) {
        
        // TODO: fix this.
        Command::Cursor--; // For the following cursor increment at the end of the switch.
        return;

    }
    if (Command::Cursor <= strlen(Command::Buffer)) {

        Command::Cursor++;
        Command::EOS = false;

    } else { Command::EOS = true; return; }
    printf("\e[1C");
    printf("\e[7");

}

void Command::ArrowDown(char character) {

    Command::CommandCache = Command::CmdCache->Forward();
    // Keep the line clear to indicate that the cache has been exhausted.
    if (!Command::CommandCache) {
        
        printf("\e[G"); // Move cursor to beginning.
        printf("\e[K"); // Clear line beginning from cursor.
        memset(Command::Buffer, 0x0, 512);
        return;

    }

    memcpy(Command::Buffer, Command::CommandCache, strlen(Command::CommandCache) + 1);
    Command::Cursor = strlen(Command::CommandCache) - 1; // Minus 1 for the following increment, so cursor=strlen(buffer).

    if (Command::Buffer) {

        printf("\e[G"); // Move cursor to beginning.
        printf("\e[K");  // Clear line beginning from cursor.
        printf("%s", Command::Buffer);

    }

}

// Processes the enter key (return).
void Command::CarriageReturn(char character) {

    if (!Command::Buffer[0]) {
                        
            memset(Command::Buffer, 0x0, 512);
            Command::EOS = true;
            return;

    }
    Command::CmdCache->Insert(Command::Buffer);
    // If Command::EOS is false, the NULL-terminator is added in the default case.
    if (Command::EOS) Command::Buffer[Command::Cursor] = 0x0;
    Command::Send = true;
    Command::EOS = true;
    return;

}

// Processes the backspace key.
void Command::Backspace(char character) {

    // Break and move to next iteration if there has been no input.
    if (Command::Cursor == 0) return;

    // Whether cursor is at the end of the input buffer or not will determine what processing takes place:
    // if EOS false, then characters from cursor to length of buffer must move back by 1; else, just visually
    // backspace and remove from end of input buffer.
    if (!Command::EOS) {
        
        printf("\e7"); // Save cursor position.
        Command::Buffer[strlen(Command::Buffer) + 2] = 0x0;
        for (int i = Command::Cursor - 1; i <= strlen(Command::Buffer) + 1; i++) {

            Command::Buffer[i] = Command::Buffer[i + 1];

        }
        Command::Cursor = Command::Cursor - 1;
        printf("\e[G"); // Move to beginning of line.
        printf("\e[K");  // Clear line beginning from cursor.
        printf("%s", Command::Buffer);
        printf("\e8"); // Restore cursor position from memory.
        printf("\e[1D"); // Move cursor backward by 1.
        return;

    }
    printf("\e7");
    printf("\b \b");
    memset(&Command::Buffer[Command::Cursor - 1], 0x0, 1); 
    Command::Cursor = Command::Cursor - 2; // "-2" because of the following increment
    return;

}

// Prints cursor position and buffer length.
void Command::PrintInformation() {

    printf("\e7"); // Save cursor position.
    printf("\e[?25l"); // Hide cursor.
    printf("\e[%dd", 2); // Move to vertical row 1.
    printf("\e[K"); // Clear line beginning from cursor.
    printf("\e[%dG", Command::MainConsole.Size().columns - 24); // Move cursor horizontally to position. 
    printf("\e[%dX", 24); // Erase 16 characters from cursor position.
    printf("CURSOR: %d, LEN: %d", Command::Cursor, strlen(Command::Buffer));
    printf("\e8"); // Restore cursor position from memory.
    printf("\e[?25h"); // Show cursor.

}

// Processes all basic alpha-numeric keys.
void Command::StandardKey(char character) {

    if (!Command::EOS) {
        
        // TODO: Command::Cursor seems to be inaccurate; needs fix.
        Command::Buffer[strlen(Command::Buffer) + 2] = 0x0;
        for (int i = strlen(Command::Buffer) + 1; i >= Command::Cursor; i--) {
            
            Command::Buffer[i] = Command::Buffer[i - 1];

        }
        memset(&Command::Buffer[Command::Cursor], Command::Character, 1);
        Command::Cursor++;

        //Command::PrintInformation();

        printf("\e[G"); // Move to beginning of line.
        printf("\e[K"); // Clear line beginning from cursor.
        printf("%s", Command::Buffer);
        printf("\e8"); // Restore cursor position from memory.
        printf("\e[1C"); // Move cursor forward by 1.
        printf("\e7"); // Save cursor position.
        return;

    }

    // Set buffer at cursor to input character then add null termination ahead of that.
    memset(&Command::Buffer[Command::Cursor], character, 1);
    memset(&Command::Buffer[Command::Cursor + 1], 0x0, 1);

    return;

}

void Command::Update() {

    printf("\e7"); // Save cursor position.
    printf("\e[?25l");
    printf("\e[%dd", 0);
    //printf("\e[K");
    printf("\e[G");
    printf(Command::ipchar);
    printf("\e8"); // Restore cursor position.
    printf("\e[?25h");

}

void Command::Update(const char* input) {

    printf("\e7"); // Save cursor position.
    printf("\e[?25l");
    printf("\e[%dd", 0);
    //printf("\e[K");
    printf("\e[%dG", Command::MainConsole.Size().columns - strlen(input));
    printf("%X", input);
    printf("\e8"); // Restore cursor position.
    printf("\e[?25h");

}

// Prints the hex value of a character.
void Command::Update(char input) {

    printf("\e7"); // Save cursor position.
    printf("\e[?25l"); // Hide cursor.
    printf("\e[%dd", 0);
    //printf("\e[K");
    printf("\e[%dG", Command::MainConsole.Size().columns - 24); // Move cursor horizontally to position. 
    printf("\e[%dX", 24); // Erase 16 characters from cursor position.
    printf("%X", input); // print hex value of char.
    printf("\e8"); // Restore cursor position.
    printf("\e[?25h"); // Show cursor.

}

void Command::CustomKey(const int code, Command::key_callback callbackfunc) {

    Command::callback_map.insert(std::pair<const int, Command::key_callback>(code, callbackfunc));

}

// Visualize textual input; writes input into buffer. This buffer will be used
// in command parsing, also.
void Command::Input() {

    // Switch that indicates a custom key was used and the while loop needs skipped.
    bool SkipSwitch = false;
    char tmpgtch = 0;

    // Place cursor at the last row in the console viewport.
    printf("\e[%dd", Command::MainConsole.Size().rows);

    // Begin loop that is active until until Command::Active flag is false.
    while (Command::Active) {

        SkipSwitch = false;
        Command::Character = (char)getch();
        // Print hex value of current character in top right corner.
        //Command::Update(Command::Character);

        // Playing with ""duplexing.""
        //if (!(Command::Cursor % ((Command::MainConsole.Size().columns / 2) - 1)) && Command::Cursor) std::cout << '\n';

        // Try keys in Command::callback_map prior to regular key processing, in case a key's action is overrided or a
        // key is added by CustomKey().
        if (!Command::callback_map.empty()) {

            for (std::map<const int, Command::key_callback>::iterator it = Command::callback_map.begin(); it != Command::callback_map.end(); it++) {
                
                if (Command::Character == 0 || Command::Character == 0xE0) {

                    tmpgtch = (char)getch();

                }
                if (it->first == Command::Character || it->first == tmpgtch) {
                    (*it->second)(*this); // call implementation-defined callback function.
                    SkipSwitch = true; // skip the upcoming switch statements.
                }

            }
            if (SkipSwitch) break;

        }

        // 0xFFFFFFE0 is too large to evaluate against a value of type char, so it must be done here.
        // This is a key code sent preceding special keys, such as arrow keys.
        // The shift key is a modifier key and thus modifies the sent keycode of a specific key; for example,
        // when Shift+k (capital k) is sent, 0x25 is changed to 0x4B, which conflicts with arrow keys represented 
        // keycode (0x4B).
        if (Command::Character == 0xFFFFFFE0 && !(GetKeyState(VK_SHIFT) & 0x8000) && !SkipSwitch) {
            
            Command::Character = (char)getch();
            
            switch (Command::Character) {

                case 0: break; // Sent after some special keys.

                case Command::KEY_VALUE::ARROW_LEFT:

                    Command::ArrowLeft(Command::Character);
                    //Command::PrintInformation();
                    break;

                case Command::KEY_VALUE::ARROW_RIGHT:

                    Command::ArrowRight(Command::Character);
                    //Command::PrintInformation();
                    break;

                case Command::KEY_VALUE::ARROW_UP:
                    
                    Command::ArrowUp(Command::Character);
                    //Command::PrintInformation();
                    return;

                case Command::KEY_VALUE::ARROW_DOWN:

                    Command::ArrowDown(Command::Character);
                    //Command::PrintInformation();
                    return;

                default: break;

            }

        } else if (!SkipSwitch) {

            switch (Command::Character) {

                case 0: break; // Sent after some special keys.

                case Command::KEY_VALUE::CARRIAGE_RETURN:

                    Command::CarriageReturn(Command::Character);
                    return;

                case Command::KEY_VALUE::CTRL_C:

                    Command::Active = false;
                    return;

                case Command::KEY_VALUE::BACKSPACE:

                    Command::Backspace(Command::Character);
                    break;

                default:    

                    Command::StandardKey(Command::Character);
                    if (Command::EOS) std::cout << Command::Buffer[Command::Cursor];
                    break;

            }

        }

        // Reset Command::Character for the next iteration.
        Command::Character = 0;

        // Increment cursor if cursor is at EOS (end of string).
        if (Command::EOS) Command::Cursor++;

        //Command::Update();
        //Command::PrintInformation();

    }

    return;

}



