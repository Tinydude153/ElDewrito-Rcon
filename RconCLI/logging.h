#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <ostream>
#include <streambuf>
#include <chrono>
#include <ctime>
#include <string>

namespace lg {

enum lg_type {

    LG_INFO,
    LG_ERROR,
    LG_WARNING

};

extern std::stringstream lg_sout;
// Info stream.
extern std::stringstream lg_sinfo;
// Error stream.
extern std::stringstream lg_serror;
// Warning stream.
extern std::stringstream lg_swarn;
// Output file stream.
extern std::ofstream lg_ofs;
void lg_outf(lg_type type);
void lg_close();
void lg_open();

// lg_type to string.
const char* lg_sts(lg::lg_type type);

/*
//
// I 100% could have just use stderr as the error output just to streamline things but
// for some reason I thought it needed another layer of abstraction; all I've done is 
// complicate things but that's fine, I've already written this out so I'll just use it.
// 
*/
class lg_logstream {

    public:
    enum lg_type {

        LG_INFO,
        LG_ERROR,
        LG_WARNING

    };

    lg_type lg_streamtype;
    std::stringstream lg_ss;
    std::ostream* lg_os;
    bool lg_file;

    // Sets log type.
    lg_logstream(lg_type type);
    // Sets log type and sets *lg_os to &ostream.
    lg_logstream(lg_type type, std::ostream& stream);
    // Sets log type and opens file defined in path.
    lg_logstream(lg_type type, const char* path);
    // Closes file.
    ~lg_logstream();
    // Returns reference to log stringstream.
    std::stringstream& lg_log();
    std::stringstream& lg_log(lg_type type);
    // lg_type to string.
    const char* lg_type_tostring(lg_type type);
    // Writes the contents of lg_ss to lg_os.
    void lg_write();
    void lg_fopen(const char* path);
    void lg_fclose();
    // Returns a char* containing the current date and time.
    //char* GetTime();

};

}

#endif