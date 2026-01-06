//
// Created by patryk on 23.09.25.
//
#pragma once

#include <string>
// #include <vector>



// dla linuxa komuniakcja po przez stream danych portu szeregowego
namespace ESP_com {
    std::vector<std::string> listSerialPorts();
}