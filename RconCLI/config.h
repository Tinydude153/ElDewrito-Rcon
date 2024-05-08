#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <string>
#include <map>
#include <iterator>
#include <fstream>

class Config {

    public:
    std::map<std::string, std::string> m_Map;
    void Parse(const char* path);
    void ReadCfg();
    std::string GetValue(std::string key);
    void SetValue(std::string key, std::string value);
    Config();
    Config(const char* path);

};

#endif