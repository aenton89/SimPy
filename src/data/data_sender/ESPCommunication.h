//
// Created by patryk on 23.09.25.
//

#include <vector>
#include <string>

#ifndef ESP_COMMUNICATION_H
#define ESP_COMMUNICATION_H

// dla linuxa komuniakcja po przez stream danych portu szeregowego
namespace ESP_com {
    std::vector<std::string> listSerialPorts();
}

#endif //ESP_COMMUNICATION_H
