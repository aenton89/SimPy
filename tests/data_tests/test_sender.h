//
// Created by tajbe on 21.08.2025.
//

#pragma once

#include "../../src/data/data_sender/data_channel_manager.h"
#include "../../src/data/blocks.h"
#include "../../src/data/structures.h"
#include <thread>
#include <cassert>
#include <iostream>

class DataSenderTests {

public:
    // podstawowa funkcjonalność DataChannelManager
    static void testBasicChannelCommunication();
    // wysyłanie różnych typów danych
    static void testMultipleDataTypes();
    // timeout przy odbieraniu danych
    static void testWaitForDataTimeout();
    // przepełnienie kolejki
    static void testQueueOverflow();
    // czyszczenie kanału
    static void testChannelClearing();
    // informacje o kanale
    static void testChannelInfo();
    // test DataSenderBlock
    static void testDataSenderBlock();
    // test wielowątkowej komunikacji
    static void testThreadedCommunication();

    static void runAllTests();

private:
    // pomocnicza funkcja do testu wielowątkowego
    static void receiverThread(const std::string& channelName, int expectedCount);
};