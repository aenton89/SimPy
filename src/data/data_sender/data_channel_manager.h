//
// Created by tajbe on 21.08.2025.
//

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <string>
#include <variant>
#include <any>
#include <chrono>
#include <vector>
#include <iostream>

// Struktura dla przechowywania różnych typów danych
struct DataPacket {
    std::string channelName;
    std::string dataType;
    std::any data;
    std::chrono::steady_clock::time_point timestamp;

    DataPacket(const std::string& channel, const std::string& type, const std::any& value);
};

// Manager kanałów komunikacyjnych
class DataChannelManager {
private:
    static std::unordered_map<std::string, std::queue<DataPacket>> channels;
    static std::mutex channelsMutex;
    static std::condition_variable dataAvailable;
    static std::atomic<bool> shouldStop;

public:
    // Wysyłanie danych do konkretnego kanału
    template<typename T>
    static void sendData(const std::string& channelName, const T& data, const std::string& dataType = "") {
        std::lock_guard<std::mutex> lock(channelsMutex);

        std::string type = dataType.empty() ? typeid(T).name() : dataType;
        channels[channelName].push(DataPacket(channelName, type, std::make_any<T>(data)));

        // Opcjonalne ograniczenie rozmiaru kolejki
        const size_t maxQueueSize = 1000;
        if (channels[channelName].size() > maxQueueSize) {
            channels[channelName].pop();
        }

        dataAvailable.notify_all();
    }

    // Odbieranie danych z konkretnego kanału (non-blocking)
    template<typename T>
    static bool receiveData(const std::string& channelName, T& data) {
        std::lock_guard<std::mutex> lock(channelsMutex);

        auto it = channels.find(channelName);
        if (it != channels.end() && !it->second.empty()) {
            try {
                DataPacket packet = it->second.front();
                it->second.pop();
                data = std::any_cast<T>(packet.data);
                return true;
            } catch (const std::bad_any_cast& e) {
                std::cerr << "Type mismatch in channel " << channelName << ": " << e.what() << std::endl;
                return false;
            }
        }
        return false;
    }

    // Odbieranie danych z timeoutem (blocking)
    template<typename T>
    static bool waitForData(const std::string& channelName, T& data, int timeoutMs = 1000) {
        std::unique_lock<std::mutex> lock(channelsMutex);

        if (dataAvailable.wait_for(lock, std::chrono::milliseconds(timeoutMs),
            [&channelName]{
                auto it = channels.find(channelName);
                return (it != channels.end() && !it->second.empty()) || shouldStop;
            })) {

            auto it = channels.find(channelName);
            if (it != channels.end() && !it->second.empty()) {
                try {
                    DataPacket packet = it->second.front();
                    it->second.pop();
                    data = std::any_cast<T>(packet.data);
                    return true;
                } catch (const std::bad_any_cast& e) {
                    std::cerr << "Type mismatch in channel " << channelName << ": " << e.what() << std::endl;
                    return false;
                }
            }
        }
        return false;
    }

    // Pobieranie informacji o pakiecie (typ, timestamp)
    static bool getChannelInfo(const std::string& channelName, std::string& dataType,
                              std::chrono::steady_clock::time_point& timestamp);

    // Utility funkcje
    static int getChannelSize(const std::string& channelName);
    static std::vector<std::string> getAvailableChannels();
    static void clearChannel(const std::string& channelName);
    static void stopAll();
};