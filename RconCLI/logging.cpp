#include <RconCLI/logging.h>

namespace lg {

std::stringstream lg_sout;
std::ofstream lg_ofs;
// Info stream.
std::stringstream lg_sinfo;
// Error stream.
std::stringstream lg_serror;
// Warning stream.
std::stringstream lg_swarn;
// Output file stream.

void lg_outf(lg_type type) {

    switch (type) {

        case lg_type::LG_ERROR:
            lg_ofs << '[' << lg_sts(type) << ']';
            lg_ofs << lg_serror.rdbuf() << '\n';
            lg_serror.flush();
            break;
        case lg_type::LG_INFO:
            lg_ofs << '[' << lg_sts(type) << ']';
            lg_ofs << lg_sinfo.rdbuf() << '\n';
            lg_sinfo.flush();
            break;
        case lg_type::LG_WARNING:
            lg_ofs << '[' << lg_sts(type) << ']';
            lg_ofs << lg_swarn.rdbuf() << '\n';
            lg_swarn.flush();
            break;
        default: break;
    }
    lg_ofs.flush();
    //lg_ofs << lg_stream.rdbuf();
    //lg_ofs.flush();

}

void lg_close() {

    lg_ofs.close();
    if (lg_ofs.fail()) std::cout << "lg::lg_close failed.\n";

}

void lg_open() {

    lg_ofs.open("log.log", std::ofstream::out | std::ofstream::app);
    if (lg_ofs.fail()) std::cout << "lg::lg_open failed.\n";

}

const char* lg_sts(lg_type type) {

    switch (type) {

        case lg_type::LG_ERROR:
            return "ERROR";
        case lg_type::LG_INFO:
            return "INFO";
        case lg_type::LG_WARNING:
            return "WARNING";
        default:
            return NULL;

    }

    return NULL;
}

// lg_log Class

lg_logstream::lg_logstream(lg_type type) {

    lg_logstream::lg_os->app;
    lg_logstream::lg_streamtype = type;

}

lg_logstream::lg_logstream(lg_type type, const char* path) {

    lg_logstream::lg_streamtype = type;
    lg_logstream::lg_fopen(path);

}

lg_logstream::lg_logstream(lg_type type, std::ostream& stream) {

    lg_logstream::lg_os = &stream;
    lg_streamtype = type;

}

lg_logstream::~lg_logstream() {

    lg_ofs.close();

}

void lg_logstream::lg_fopen(const char* path) {

    lg_logstream::lg_os = &lg_ofs;
    lg_ofs.open(path, std::ofstream::out | std::ofstream::app);

}

std::stringstream& lg_logstream::lg_log() {

    lg_logstream::lg_ss << '[' << lg_logstream::lg_type_tostring(lg_logstream::lg_streamtype) << ']';
    return lg_ss;

}

std::stringstream& lg_logstream::lg_log(lg_logstream::lg_type type) {

    lg_logstream::lg_ss << '[' << lg_logstream::lg_type_tostring(type) << "] ";
    return lg_ss;

}

const char* lg_logstream::lg_type_tostring(lg_type type) {

    switch (type) {

        case lg_type::LG_ERROR:
            return "ERROR";
        case lg_type::LG_INFO:
            return "INFO";
        case lg_type::LG_WARNING:
            return "WARNING";
        default:
            return NULL;

    }

    return NULL;
}

void lg_logstream::lg_write() {

    *lg_logstream::lg_os << lg_logstream::lg_ss.rdbuf() << '\n';
    lg_logstream::lg_ss.flush();
    lg_logstream::lg_os->flush();

}

}

/*
Logging::Logging() {

    Logging::lg_ofs.open("log.log", std::ofstream::out | std::ofstream::app);

}

Logging::~Logging() {

    Logging::lg_ofs.close();

}

// Returns reference to log stringstream.
std::stringstream& Logging::GetStream() {

    return Logging::lg_ss;

}

void Logging::WriteLog() {

    Logging::lg_ofs << Logging::lg_ss.str(); 

}

char* Logging::GetTime() {

    std::chrono::system_clock::time_point today = std::chrono::system_clock::now();
    time_t tt;
    tt = std::chrono::system_clock::to_time_t(today);
    return ctime(&tt);

}
*/