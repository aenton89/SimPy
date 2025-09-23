//
// Created by tajbe on 21.08.2025.
//

#ifndef DATA_CHANNEL_MANAGER_H
#define DATA_CHANNEL_MANAGER_H

#include <vector>
#include <string>
#include <memory>
#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#else
    #include <unistd.h>
#endif


class DataChannelManager {
private:
    static std::unique_ptr<DataChannelManager> instance;
    bool is_connected;
    std::string pipe_name;

#ifdef _WIN32
    HANDLE hPipe;
#else
    int pipe_fd;
#endif

    // private constructor for singleton
    DataChannelManager();
    // helper function to create JSON string
    std::string createJSON(const std::vector<double>& data, float dt, float simTime);

public:
    // singleton pattern
    static DataChannelManager* getInstance();
    // destructor
    ~DataChannelManager();
    // delete copy constructor and assignment operator
    DataChannelManager(const DataChannelManager&) = delete;
    DataChannelManager& operator=(const DataChannelManager&) = delete;
    // initialize connection to Python
    bool initialize(const std::string& pipeName = "/tmp/simulink_pipe");
    // send data to Python
    bool sendData(const std::vector<double>& data, float dt, float simTime);
    // close connection
    void close();
    // check if connected
    bool isConnected() const { return is_connected; }
    // get last error message
    std::string getLastError() const;
};


#endif // DATA_CHANNEL_MANAGER_H