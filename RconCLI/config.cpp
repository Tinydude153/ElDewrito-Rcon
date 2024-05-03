#include <RconCLI/config.h>

void Config::Parse(const char* path) {

    std::ifstream file(path);
    if (file.fail()) {
        std::cerr << "[CONFIG] File failed to open.\n";
        return;
    }
    std::string cfg((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::string key;
    std::string value;
    size_t g_pos = 0;

    g_pos = cfg.find('=');
    while (g_pos != std::string::npos) {

        g_pos = cfg.find('=');
        if (g_pos != std::string::npos) {
            key = cfg.substr(0, g_pos - 1);
            cfg.erase(0, g_pos);
        }
        g_pos = cfg.find('"');
        if (g_pos != std::string::npos) {
            value = cfg.substr(g_pos + 1, (cfg.find('"', g_pos + 1)) - 3);
            cfg.erase(0, g_pos);
        }
        g_pos = cfg.find("\n");
        if (g_pos != std::string::npos) {
            cfg.erase(0, g_pos + 1);
        }
        if (!key.empty() && !value.empty()) {
            this->m_Map.insert(std::pair<std::string, std::string>(key, value));
        }

    }

}

void Config::ReadCfg() {

    for (std::map<std::string, std::string>::iterator it = this->m_Map.begin(); it != this->m_Map.end(); it++) {
        if (!it->first.empty()) std::cout << "Key: " << it->first << "\n";
        if (!it->second.empty()) std::cout << "Value: " << it->second << "\n";
    }

}

std::string Config::GetValue(std::string key) {

    std::string value = "";
    for (std::map<std::string, std::string>::iterator it = this->m_Map.begin(); it != this->m_Map.end(); it++) {

        if (it->first.compare(key) == 0) {

            if (!it->second.empty()) {

                value = it->second;
                return value;

            }

        }

    }

    return value;

}

Config::Config(const char* path) {

    Config::Parse(path);

}

Config::Config() {}