//
// Created by tajbe on 21.08.2025.
//


// DataSenderBlock.h
#include "data_channel_manager.h"

// Definicje statyczne
std::unordered_map<std::string, std::queue<DataPacket>> DataChannelManager::channels;
std::mutex DataChannelManager::channelsMutex;
std::condition_variable DataChannelManager::dataAvailable;
std::atomic<bool> DataChannelManager::shouldStop{false};

// Implementacja konstruktora DataPacket
DataPacket::DataPacket(const std::string& channel, const std::string& type, const std::any& value)
    : channelName(channel), dataType(type), data(value), timestamp(std::chrono::steady_clock::now()) {}

// Implementacja metod DataChannelManager

bool DataChannelManager::getChannelInfo(const std::string& channelName, std::string& dataType,
                                       std::chrono::steady_clock::time_point& timestamp) {
    std::lock_guard<std::mutex> lock(channelsMutex);

    auto it = channels.find(channelName);
    if (it != channels.end() && !it->second.empty()) {
        const DataPacket& packet = it->second.front();
        dataType = packet.dataType;
        timestamp = packet.timestamp;
        return true;
    }
    return false;
}

int DataChannelManager::getChannelSize(const std::string& channelName) {
    std::lock_guard<std::mutex> lock(channelsMutex);
    auto it = channels.find(channelName);
    return (it != channels.end()) ? it->second.size() : 0;
}

std::vector<std::string> DataChannelManager::getAvailableChannels() {
    std::lock_guard<std::mutex> lock(channelsMutex);
    std::vector<std::string> channelNames;
    for (const auto& pair : channels) {
        channelNames.push_back(pair.first);
    }
    return channelNames;
}

void DataChannelManager::clearChannel(const std::string& channelName) {
    std::lock_guard<std::mutex> lock(channelsMutex);
    auto it = channels.find(channelName);
    if (it != channels.end()) {
        while (!it->second.empty()) {
            it->second.pop();
        }
    }
}

void DataChannelManager::stopAll() {
    shouldStop = true;
    dataAvailable.notify_all();
}