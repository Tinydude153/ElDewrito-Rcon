/*
// Simple Object Serialization:
// Syntax: object { data=info, data1=info1 }
// This is just a simple human-readable format for multi-object configurations.
*/

#include <iostream>
#include <string>
#include <map>
#include <iterator>
#include <fstream>
#include <algorithm>

class SOSerial {

    public:
    std::map<std::string, std::map<std::string, std::string>> m_SerializationMap;
    SOSerial();
    SOSerial(const char* path);
    void Deserialize(const char* path);
    void PrintMap();
    std::string GetValue(std::string object, std::string key);

};