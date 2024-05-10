#include <RconCLI/sos/sos.h>

void SOSerial::Deserialize(const char* path) {

    std::ifstream file(path);
    if (file.fail()) {
        std::cerr << "[SOSerial] File failed to open.\n";
        return;
    }
    std::string serial((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    serial.erase(std::remove(serial.begin(), serial.end(), ' '), serial.end());
    serial.erase(std::remove(serial.begin(), serial.end(), '\n'), serial.end());
    serial.erase(std::remove(serial.begin(), serial.end(), '\t'), serial.end());
    std::map<std::string, std::string> SOSMap;
    std::string object_key;
    std::string key;
    std::string value;
    std::string object_temp;
    size_t g_pos = 0;
    size_t c_pos = 0;

    g_pos = serial.find('{');
    while (g_pos != std::string::npos) {
        SOSMap.clear();
        g_pos = serial.find('{');
        if (g_pos != std::string::npos) {
            object_key = serial.substr(0, g_pos);
            serial.erase(0, g_pos);
        }
        g_pos = serial.find('}');
        if (g_pos != std::string::npos) {
            object_temp = serial.substr(0, g_pos + 1);
            while (object_temp.find(',') != std::string::npos) {
                c_pos = object_temp.find(',');
                if (c_pos != std::string::npos) {
                    g_pos = object_temp.find('=');
                    if (g_pos != std::string::npos) {
                        key = object_temp.substr(1, g_pos - 1);
                        object_temp.erase(0, g_pos);
                        c_pos = object_temp.find(',');
                        if (c_pos != std::string::npos) {
                            value = object_temp.substr(1, c_pos - 1);
                        } else { value = object_temp.substr(1, object_temp.find('}') - 1); }
                        c_pos = object_temp.find(',');
                        if (c_pos != std::string::npos) {
                            object_temp.erase(0, object_temp.find(','));
                        } else { object_temp.erase(object_temp.begin(), object_temp.end()); }
                        SOSMap.insert(std::pair<std::string, std::string>(key, value));
                    }
                    c_pos = object_temp.find(',');
                    if (c_pos == std::string::npos) {
                        g_pos = object_temp.find('=');
                        if (g_pos != std::string::npos) {
                            key = object_temp.substr(0, g_pos);
                            object_temp.erase(0, g_pos);
                            value = object_temp.substr(0);
                            object_temp.erase(object_temp.begin(), object_temp.end());
                            SOSMap.insert(std::pair<std::string, std::string>(key, value));
                        }
                    }
                }
            }
        }
        serial.erase(0, serial.find('}'));
        g_pos = serial.find('{');
        if (g_pos != std::string::npos) serial.erase(0, 2);
        if (!key.empty() && !value.empty()) {
            this->m_SerializationMap.insert(std::pair<std::string, std::map<std::string, std::string>>(object_key, SOSMap));
        }

    }

}

void SOSerial::PrintMap() {

    for (std::map<std::string, std::map<std::string, std::string>>::iterator it = this->m_SerializationMap.begin(); it != this->m_SerializationMap.end(); it++) {
        if (!it->first.empty()) std::cout << it->first << " {\n";
        for (std::map<std::string, std::string>::iterator lit = it->second.begin(); lit != it->second.end(); lit++) {
            if (!lit->second.empty()) std::cout << "\t" << lit->first << " = ";
            if (!lit->second.empty()) std::cout << lit->second << "\n";
        }
        std::cout << "}\n";
    }

}

std::string SOSerial::GetValue(std::string object, std::string key) {

    std::string value = "";
    for (std::map<std::string, std::map<std::string, std::string>>::iterator it = this->m_SerializationMap.begin(); it != this->m_SerializationMap.end(); it++) {

        if (it->first.compare(object) == 0) {

            for (std::map<std::string, std::string>::iterator lit = it->second.begin(); lit != it->second.end(); lit++) {

                if (lit->first.compare(key) == 0) {

                    if (!lit->second.empty()) {

                        value = lit->second;
                        return value;
                    
                    }
                
                }
           
            }
       
        } 
   
    }
    return value;

}

SOSerial::SOSerial() {}
SOSerial::SOSerial(const char* path) {}