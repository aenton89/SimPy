//
// Created by tajbe on 21.08.2025.
//

#include "test_sender.h"
#include <chrono>


void DataSenderTests::testBasicChannelCommunication() {
    std::cout << "Testing basic channel communication..." << std::endl;

    const std::string testChannel = "test_basic";

    // wyślij dane
    DataChannelManager::sendData<float>(testChannel, 42.5f, "float");

    // odbierz dane
    float receivedValue;
    bool success = DataChannelManager::receiveData<float>(testChannel, receivedValue);

    assert(success);
    assert(receivedValue == 42.5f);

    // Sprawdź czy kanał jest pusty po odbiorze
    assert(DataChannelManager::getChannelSize(testChannel) == 0);

    std::cout << "✓ Basic channel communication test passed" << std::endl;
}

void DataSenderTests::testMultipleDataTypes() {
    std::cout << "Testing multiple data types..." << std::endl;

    const std::string testChannel = "test_types";

    // wyślij różne typy danych
    DataChannelManager::sendData<int>(testChannel, 123, "int");
    DataChannelManager::sendData<double>(testChannel, 3.14159, "double");
    DataChannelManager::sendData<float>(testChannel, 2.71f, "float");

    assert(DataChannelManager::getChannelSize(testChannel) == 3);

    // odbierz w kolejności FIFO
    int intVal;
    double doubleVal;
    float floatVal;

    assert(DataChannelManager::receiveData<int>(testChannel, intVal));
    assert(intVal == 123);

    assert(DataChannelManager::receiveData<double>(testChannel, doubleVal));
    assert(doubleVal == 3.14159);

    assert(DataChannelManager::receiveData<float>(testChannel, floatVal));
    assert(floatVal == 2.71f);

    std::cout << "✓ Multiple data types test passed" << std::endl;
}

void DataSenderTests::testWaitForDataTimeout() {
    std::cout << "Testing waitForData timeout..." << std::endl;

    const std::string testChannel = "test_timeout";

    float value;
    auto start = std::chrono::steady_clock::now();

    // to powinno się skończyć timeoutem
    bool success = DataChannelManager::waitForData<float>(testChannel, value, 100);

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    assert(!success);
    assert(duration.count() >= 100); // Minimum 100ms timeout

    std::cout << "✓ Timeout test passed (" << duration.count() << "ms)" << std::endl;
}

void DataSenderTests::testQueueOverflow() {
    std::cout << "Testing queue overflow..." << std::endl;

    const std::string testChannel = "test_overflow";

    // wyślij więcej niż limit kolejki (1000)
    for (int i = 0; i < 1100; ++i) {
        DataChannelManager::sendData<int>(testChannel, i, "int");
    }

    // sprawdź czy rozmiar jest ograniczony
    int queueSize = DataChannelManager::getChannelSize(testChannel);
    assert(queueSize <= 1000);

    // sprawdź czy najstarsze dane zostały usunięte (powinniśmy mieć 100-1099)
    int receivedValue;
    DataChannelManager::receiveData<int>(testChannel, receivedValue);
    // pierwsze 100 wartości zostało usuniętych
    assert(receivedValue == 100);

    DataChannelManager::clearChannel(testChannel);

    std::cout << "✓ Queue overflow test passed (queue size: " << queueSize << ")" << std::endl;
}

void DataSenderTests::testChannelClearing() {
    std::cout << "Testing channel clearing..." << std::endl;

    const std::string testChannel = "test_clear";

    // wyślij kilka wartości
    for (int i = 0; i < 5; ++i) {
        DataChannelManager::sendData<int>(testChannel, i, "int");
    }

    assert(DataChannelManager::getChannelSize(testChannel) == 5);

    // wyczyść kanał
    DataChannelManager::clearChannel(testChannel);

    assert(DataChannelManager::getChannelSize(testChannel) == 0);

    std::cout << "✓ Channel clearing test passed" << std::endl;
}

void DataSenderTests::testChannelInfo() {
    std::cout << "Testing channel info..." << std::endl;

    const std::string testChannel = "test_info";

    // wyślij dane z określonym typem
    DataChannelManager::sendData<float>(testChannel, 42.0f, "custom_float");

    std::string dataType;
    std::chrono::steady_clock::time_point timestamp;

    bool success = DataChannelManager::getChannelInfo(testChannel, dataType, timestamp);

    assert(success);
    assert(dataType == "custom_float");

    // sprawdź czy timestamp jest niedawny (w ostatnich 1000ms)
    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp);
    assert(diff.count() < 1000);

    DataChannelManager::clearChannel(testChannel);

    std::cout << "✓ Channel info test passed" << std::endl;
}

void DataSenderTests::testDataSenderBlock() {
    std::cout << "Testing DataSenderBlock..." << std::endl;

    // tworzymy DataSenderBlock
    DataSenderBlock sender(1, "test_block_channel");

    // testuj gettery/settery
    assert(sender.getChannelName() == "test_block_channel");
    assert(sender.getDataType() == "float");
    assert(sender.isSendEnabled() == true);
    assert(sender.getSendCounter() == 0);

    sender.setChannelName("new_channel");
    sender.setDataType("int");
    sender.setSendEnabled(false);

    assert(sender.getChannelName() == "new_channel");
    assert(sender.getDataType() == "int");
    assert(sender.isSendEnabled() == false);

    // reset dla testu process()
    sender.setSendEnabled(true);
    sender.setDataType("float");

    // symuluj inputValues (normalnie ustawiane przez system)
    sender.getInputValues().push_back(123.45);

    // test process()
    sender.process();
    assert(sender.getSendCounter() == 1);

    // sprawdź czy dane zostały wysłane
    float receivedValue;
    bool success = DataChannelManager::receiveData<float>("new_channel", receivedValue);
    assert(success);
    assert(receivedValue == 123.45f);

    std::cout << "DataSenderBlock test passed" << std::endl;
}

void DataSenderTests::receiverThread(const std::string& channelName, int expectedCount) {
    int receivedCount = 0;
    float value;

    while (receivedCount < expectedCount) {
        if (DataChannelManager::waitForData<float>(channelName, value, 1000)) {
            receivedCount++;
            std::cout << "Received: " << value << " (count: " << receivedCount << ")" << std::endl;
        } else {
            std::cout << "Timeout waiting for data" << std::endl;
            break;
        }
    }

    assert(receivedCount == expectedCount);
}

void DataSenderTests::testThreadedCommunication() {
    std::cout << "Testing threaded communication..." << std::endl;

    const std::string testChannel = "test_threaded";
    const int messageCount = 10;

    // uruchom wątek odbierający
    std::thread receiver(receiverThread, testChannel, messageCount);

    // poczekaj chwilę, żeby receiver się uruchomił
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // wyślij dane z głównego wątku
    for (int i = 0; i < messageCount; ++i) {
        DataChannelManager::sendData<float>(testChannel, static_cast<float>(i * 10), "float");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // poczekaj na zakończenie wątku odbierającego
    receiver.join();

    std::cout << "Threaded communication test passed" << std::endl;
}

void DataSenderTests::runAllTests() {
    std::cout << "=== Running DataSender Tests ===" << std::endl;

    try {
        testBasicChannelCommunication();
        testMultipleDataTypes();
        testWaitForDataTimeout();
        testQueueOverflow();
        testChannelClearing();
        testChannelInfo();
        testDataSenderBlock();
        testThreadedCommunication();

        std::cout << "\nAll tests passed successfully!" << std::endl;

        // Wyczyść wszystkie kanały testowe
        auto channels = DataChannelManager::getAvailableChannels();
        for (const auto& channel : channels) {
            DataChannelManager::clearChannel(channel);
        }

        std::cout << "Test cleanup completed" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        throw;
    }
}