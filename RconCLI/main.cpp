#include <RconCLI/rcon.h>

#include <bitset>
#include <string>
#include <conio.h>
#include <thread>
#include <mutex>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

int main() {

    // Set window title.
    SetConsoleTitle("RconCLI");
    //printf("\e]0;RconCLI\a"); // Does not work with regular CMD window, but it does in MSYS.

    // Associate stderr with a log file, as the many libraries output to stderr for logging.
    freopen("log.log", "a", stderr);

    // Log that RconCLI has been opened.
    Dewrito::logstream.lg_log(Dewrito::logstream.LG_INFO) << "RconCLI opened.";
    Dewrito::logstream.lg_write();

    Dewrito::Rcon rcon;
    if (!rcon.Fail) {

        // Begin main loop.
        rcon.RconLoop();

    } else {

        Dewrito::logstream.lg_log(Dewrito::logstream.LG_ERROR) << "Config failure: check config.json.\n";
        Dewrito::logstream.lg_log(Dewrito::logstream.LG_ERROR) << "RconCLI exiting.\n";
        Dewrito::logstream.lg_write();
        return 1;

    }

    Dewrito::logstream.lg_log(Dewrito::logstream.LG_INFO) << "RconCLI exited normally.";
    Dewrito::logstream.lg_write();
    return 0;
}
