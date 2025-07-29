//
// Created by patryk on 26.07.25.
//

#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "file_menager.h"
#include <sstream>
#include <fstream>
#include <iostream>

using json = nlohmann::json;


std::vector<std::string> FileMenager::split(const std::string& str, char delimiter)
{
    std::vector<std::string> results;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter))
        results.push_back(token);

    return results;
}

std::vector<std::string> FileMenager::parseKernelList(const std::string& jsonStr)
{
    std::vector<std::string> result;
    auto j = json::parse(jsonStr);

    if (j.contains("kernels") && j["kernels"].is_array()) {
        for (auto& k : j["kernels"])
            result.push_back(k.get<std::string>());
    }
    return result;
}

std::string FileMenager::readJsonFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Nie można otworzyć pliku: " + path);

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void FileMenager::writeJsonFile(const std::string& path, const std::string& jsonPath)
{
    std::ifstream inFile(jsonPath);
    json j;
    if (inFile.is_open())
    {
        inFile >> j;
        inFile.close();

        j["kernels"].push_back(path);

        std::ofstream outFile(jsonPath);
        if (outFile.is_open()) {
            outFile << j.dump(4);
            outFile.close();
        }
        else std::cout << "Can't open json file for writing" << std::endl;
    }
    else std::cout << "Can't open json file for reading" << std::endl;
}




