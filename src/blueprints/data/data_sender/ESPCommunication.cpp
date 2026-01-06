//
// Created by patryk on 23.09.25.
//
#include "ESPCommunication.h"

#ifdef __linux__
    #include <dirent.h>
#elif defined(_WIN32)
    #define NOMINMAX
    #include <windows.h>
#endif

std::vector<std::string> ESP_com::listSerialPorts() {
    std::vector<std::string> serialPorts;

#ifdef __linux__
    DIR *dir = opendir("/dev");
    if (dir == nullptr) return serialPorts;

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name(entry->d_name);
        if (name.find("ttyUSB") != std::string::npos || name.find("ttyACM") != std::string::npos) {
            serialPorts.push_back("/dev/" + name);
        }
    }
    closedir(dir);

#elif defined(_WIN32)
    for (int i = 1; i <= 255; i++) {
        std::string portName = "COM" + std::to_string(i);
        std::string fullName = R"(\\.\)" + portName;

        HANDLE hSerial = CreateFileA(fullName.c_str(),
                                     GENERIC_READ | GENERIC_WRITE,
                                     0, nullptr, OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hSerial != INVALID_HANDLE_VALUE) {
            serialPorts.push_back(portName);
            CloseHandle(hSerial);
        }
    }
#endif

    if (serialPorts.empty())
        serialPorts.emplace_back("USB device is not avalibe");
    return serialPorts;
}


