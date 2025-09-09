//
// Created by tajbe on 21.08.2025.
//


#include "data_channel_manager.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>



// initialize static member
std::unique_ptr<DataChannelManager> DataChannelManager::instance = nullptr;

DataChannelManager::DataChannelManager() : is_connected(false), pipe_name("") {
#ifdef _WIN32
    hPipe = INVALID_HANDLE_VALUE;
#else
    pipe_fd = -1;
#endif
}

DataChannelManager::~DataChannelManager() {
    close();
}

DataChannelManager* DataChannelManager::getInstance() {
    if (instance == nullptr)
        instance = std::unique_ptr<DataChannelManager>(new DataChannelManager());
    return instance.get();
}

bool DataChannelManager::initialize(const std::string& pipeName) {
    if (is_connected)
        close();

    pipe_name = pipeName;

#ifdef _WIN32
    // Windows named pipe - sprawdź czy pipeName już zawiera pełną ścieżkę
    std::string winPipeName;
    // już zawiera pełną ścieżkę
    if (pipeName.find("\\\\.\\pipe\\") == 0)
        winPipeName = pipeName;
    else
        winPipeName = "\\\\.\\pipe\\" + pipeName;

    // na Windows serwer tworzy pipe PRZED próbą połączenia klienta
    hPipe = CreateNamedPipeA(
        winPipeName.c_str(),
        PIPE_ACCESS_OUTBOUND,                    // tylko do zapisu
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,                                       // maksymalnie 1 instancja
        65536,                                   // rozmiar bufora wyjściowego
        65536,                                   // rozmiar bufora wejściowego
        0,                                       // domyślny timeout
        NULL                                     // domyślne atrybuty bezpieczeństwa
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        std::cerr << "Failed to create pipe '" << winPipeName << "': " << error << std::endl;
        return false;
    }

    std::cout << "Pipe created: " << winPipeName << std::endl;
    std::cout << "Waiting for Python client to connect..." << std::endl;

    // czekaj na połączenie klienta (Python)
    BOOL connected = ConnectNamedPipe(hPipe, NULL);

    if (!connected) {
        DWORD error = GetLastError();
        // ERROR_PIPE_CONNECTED oznacza że klient już się połączył
        if (error != ERROR_PIPE_CONNECTED) {
            std::cerr << "Failed to connect to client: " << error << std::endl;
            CloseHandle(hPipe);
            hPipe = INVALID_HANDLE_VALUE;
            return false;
        }
    }

    is_connected = true;
    std::cout << "Python client connected successfully!" << std::endl;

#else
    // Unix/Linux named pipe (FIFO) - bez zmian
    if (mkfifo(pipe_name.c_str(), 0666) == -1) {
        if (errno != EEXIST) {
            std::cerr << "Failed to create FIFO: " << strerror(errno) << std::endl;
            return false;
        }
    }

    pipe_fd = open(pipe_name.c_str(), O_WRONLY | O_NONBLOCK);
    if (pipe_fd == -1) {
        pipe_fd = open(pipe_name.c_str(), O_WRONLY);
        if (pipe_fd == -1) {
            std::cerr << "Failed to open FIFO: " << strerror(errno) << std::endl;
            return false;
        }
    }

    is_connected = true;
#endif

    return true;
}

std::string DataChannelManager::createJSON(const std::vector<float>& data, float dt, float simTime) {
    std::stringstream json;
    json << std::fixed << std::setprecision(6);
    json << "{";
    json << "\"dt\":" << dt << ",";
    json << "\"simTime\":" << simTime << ",";
    json << "\"data\":[";

    for (size_t i = 0; i < data.size(); ++i) {
        json << data[i];
        if (i < data.size() - 1) {
            json << ",";
        }
    }

    json << "]";
    // add newline as message delimiter
    json << "}\n";

    return json.str();
}

bool DataChannelManager::sendData(const std::vector<float>& data, float dt, float simTime) {
    if (!is_connected) {
        std::cerr << "Not connected to pipe. Call initialize() first." << std::endl;
        return false;
    }

    std::string jsonData = createJSON(data, dt, simTime);

#ifdef _WIN32
    DWORD bytesWritten;
    BOOL success = WriteFile(
        hPipe,
        jsonData.c_str(),
        static_cast<DWORD>(jsonData.length()),
        &bytesWritten,
        NULL
    );

    if (!success || bytesWritten != jsonData.length()) {
        std::cerr << "Failed to write to pipe: " << GetLastError() << std::endl;
        is_connected = false;
        return false;
    }

    // flush the pipe
    FlushFileBuffers(hPipe);
#else
    ssize_t bytesWritten = write(pipe_fd, jsonData.c_str(), jsonData.length());

    if (bytesWritten == -1) {
        if (errno == EPIPE) {
            std::cerr << "Broken pipe - reader disconnected" << std::endl;
            is_connected = false;
        }
        else
            std::cerr << "Failed to write to pipe: " << strerror(errno) << std::endl;
        return false;
    }

    if (static_cast<size_t>(bytesWritten) != jsonData.length()) {
        std::cerr << "Partial write to pipe" << std::endl;
        return false;
    }
#endif

    return true;
}

void DataChannelManager::close() {
    if (!is_connected)
        return;

#ifdef _WIN32
    if (hPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
        hPipe = INVALID_HANDLE_VALUE;
    }
#else
    if (pipe_fd != -1) {
        ::close(pipe_fd);
        pipe_fd = -1;
    }
#endif

    is_connected = false;
    std::cout << "Disconnected from pipe" << std::endl;
}

std::string DataChannelManager::getLastError() const {
#ifdef _WIN32
    DWORD error = GetLastError();
    if (error == 0)
        return "No error";

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer, 0, NULL
    );

    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);

    return message;
#else
    return std::string(strerror(errno));
#endif
}