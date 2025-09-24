//
// Created by patryk on 23.09.25.
//
#include "ESP_communication.h"

#ifdef __linux__
    #include <dirent.h>
#elif defined(_WIN32)
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
        if (name.find("ttyUSB") != std::string::npos ||
            name.find("ttyACM") != std::string::npos)
        {
            serialPorts.push_back("/dev/" + name);
        }
    }
    closedir(dir);

#elif defined(_WIN32)
    for (int i = 1; i <= 255; i++) {
        std::string portName = "COM" + std::to_string(i);
        std::string fullName = "\\\\.\\" + portName;

        HANDLE hSerial = CreateFileA(fullName.c_str(),
                                     GENERIC_READ | GENERIC_WRITE,
                                     0, NULL, OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL, NULL);

        if (hSerial != INVALID_HANDLE_VALUE) {
            serialPorts.push_back(portName);
            CloseHandle(hSerial);
        }
    }
#endif

    if (serialPorts.empty())
        serialPorts.push_back("USB device is not avalibe");
    return serialPorts;
}


