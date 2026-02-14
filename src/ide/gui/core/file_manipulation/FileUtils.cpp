//
// Created by patryk on 14.02.26.
//

#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <sstream>
#include <fstream>
#include <iostream>
#include <glad/glad.h>
#include "stb_image.h"

#include "FileUtils.h"

///////////////////////////////////////////////////////////////////////////////////////// Operacje na jsonach plus kilka funkji popmocniczych
std::vector<std::string> FileUtils::split(const std::string& str, char delimiter)
{
    std::vector<std::string> results;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter))
        results.push_back(token);

    return results;
}

std::vector<std::string> FileUtils::parseKernelList(const std::string& jsonStr)
{
    std::vector<std::string> result;
    auto j = json::parse(jsonStr);

    if (j.contains("kernels") && j["kernels"].is_array()) {
        for (auto& k : j["kernels"])
            result.push_back(k.get<std::string>());
    }
    return result;
}

std::string FileUtils::readJsonFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Nie można otworzyć pliku: " + path);

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void FileUtils::writeJsonFile(const std::string& path, const std::string& jsonPath)
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




////////////////////////////////////////////////////////////////////// Otwieranie plikow (.py .ipynb itd ////////////////////////////////////////////////////////

std::map<int, OpenFile::CellData> OpenFile::openIpynb(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć pliku: " << path << std::endl;
        return {};
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << "Błąd parsowania JSON: " << e.what() << std::endl;
        return {};
    }

    if (!j.contains("cells") || !j["cells"].is_array()) {
        std::cerr << "Plik nie zawiera sekcji 'cells' lub ma niewłaściwy format." << std::endl;
        return {};
    }

    std::map<int, CellData> cellMap;

    for (size_t i = 0; i < j["cells"].size(); ++i) {
        const auto& cell = j["cells"][i];
        CellData cellData;

        // Typ komórki (np. "code" lub "markdown")
        cellData.cell_type = cell.value("cell_type", "");

        // Zbieranie źródła komórki (source)
        if (cell.contains("source")) {
            if (cell["source"].is_array()) {
                for (const auto& line : cell["source"]) {
                    cellData.source += line.get<std::string>();
                }
            } else if (cell["source"].is_string()) {
                cellData.source += cell["source"].get<std::string>();
            }
        }

        // Parsowanie outputów (jeśli istnieją)
        cellData.outputs.clear();
        cellData.base64_img.clear();

        if (cell.contains("outputs") && cell["outputs"].is_array()) {
            for (const auto& output : cell["outputs"]) {
                std::string output_type = output.value("output_type", "");

                if (output_type == "stream") {
                    // tekst z print() lub stderr
                    if (output.contains("text")) {
                        if (output["text"].is_array()) {
                            for (const auto& line : output["text"]) {
                                cellData.outputs += line.get<std::string>();
                            }
                        } else if (output["text"].is_string()) {
                            cellData.outputs += output["text"].get<std::string>();
                        }
                    }
                }
                else if (output_type == "execute_result" || output_type == "display_data") {
                    // wynik działania — najczęściej jest w data/text/plain lub może obraz base64
                    if (output.contains("data")) {
                        const auto& data = output["data"];

                        // Dodaj tekst z "text/plain", jeśli jest
                        if (data.contains("text/plain")) {
                            if (data["text/plain"].is_array()) {
                                for (const auto& line : data["text/plain"]) {
                                    cellData.outputs += line.get<std::string>();
                                }
                            } else if (data["text/plain"].is_string()) {
                                cellData.outputs += data["text/plain"].get<std::string>();
                            }
                        }

                        // Spróbuj znaleźć obraz PNG w base64 (pierwszy taki)
                        if (data.contains("image/png") && cellData.base64_img.empty()) {
                            if (data["image/png"].is_array()) {
                                for (const auto& line : data["image/png"]) {
                                    cellData.base64_img += line.get<std::string>();
                                }
                            } else if (data["image/png"].is_string()) {
                                cellData.base64_img = data["image/png"].get<std::string>();
                            }
                        }
                    }
                }
                else if (output_type == "error") {
                    // błąd — pokaż ename + evalue + traceback
                    cellData.outputs += "Error: ";
                    cellData.outputs += output.value("ename", "") + ": " + output.value("evalue", "") + "\n";
                    if (output.contains("traceback")) {
                        for (const auto& line : output["traceback"]) {
                            cellData.outputs += line.get<std::string>() + "\n";
                        }
                    }
                }
            }
        }

        cellMap[static_cast<int>(i)] = std::move(cellData);
    }

    return cellMap;
}




std::map<int, OpenFile::CellData> OpenFile::createIpynb_map(
    const std::vector<std::unique_ptr<BaseCell>>& cells)
{
    std::map<int, OpenFile::CellData> cellMap;
    int i = 0;
    for (auto& cellPtr : cells)
    {
        const auto& cell = *cellPtr;   // referencja do obiektu
        CellData cellData;

        if (cell.getType() == CellType::CodeCell)
        {
            auto& codeCell = dynamic_cast<const CodeCell&>(cell);
            cellData.cell_type = "code";
            cellData.source = codeCell.getCodeInput();
            cellData.outputs = codeCell.getCodeOutput();
            cellData.base64_img = codeCell.getBase64();
        }
        else
        {
            auto& mdCell = dynamic_cast<const MardownCell&>(cell);
            cellData.cell_type = "markdown";
            cellData.source = mdCell.getInput();
        }
        cellMap[i++] = cellData;
    }
    return cellMap;
}


void OpenFile::writeIpynb(const std::string& path, const std::map<int, CellData>& cells)
{
    json j;
    j["cells"] = json::array();

    for (const auto& [index, cell] : cells) {
        json cellJson;
        cellJson["cell_type"] = cell.cell_type;
        cellJson["metadata"] = json::object();

        // source jest stringiem - podajemy jako tablicę stringów, aby format był poprawny
        cellJson["source"] = json::array();
        cellJson["source"].push_back(cell.source);

        if (cell.cell_type == "code") {
            cellJson["outputs"] = json::array();

            // Jeśli jest tekstowy output, zapisz go jako stream
            if (!cell.outputs.empty()) {
                json outputJson;
                outputJson["output_type"] = "stream";
                outputJson["name"] = "stdout";

                // Jupyter akceptuje string lub tablicę stringów dla 'text'
                outputJson["text"] = cell.outputs;
                cellJson["outputs"].push_back(outputJson);
            }

            // Jeśli jest base64 obrazek, dodaj jako display_data z image/png
            if (!cell.base64_img.empty()) {
                json imgOutput;
                imgOutput["output_type"] = "display_data";
                imgOutput["data"] = {
                    {"image/png", cell.base64_img}
                };
                imgOutput["metadata"] = json::object();
                cellJson["outputs"].push_back(imgOutput);
            }

            // W notebooku jest execution_count - na razie null
            cellJson["execution_count"] = nullptr;
        } else {
            // dla np. markdownu, outputs nie istnieje
        }

        j["cells"].push_back(cellJson);
    }

    j["metadata"] = {
        {"kernelspec", {
                {"display_name", "Python 3"},
                {"language", "python"},
                {"name", "python3"}
        }},
        {"language_info", {
                {"name", "python"},
                {"version", "3.10"}
        }}
    };
    j["nbformat"] = 4;
    j["nbformat_minor"] = 5;

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Nie można zapisać pliku: " << path << std::endl;
        return;
    }

    file << j.dump(4); // zapis z wcięciem 4 spacji
}


std::string OpenFile::openPyfile(const std::string& path)
{
    std::ifstream file(path);
    if (file)
    {
        std::stringstream ss;
        ss << file.rdbuf();

        return ss.str();
    }
    return "";
}

//----------------------------------------------------------------- grafy matplotliba ---------------------------------------------
std::string ImgOpen::base64_decode(const std::string& in) {
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) {
        T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
    }

    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

GLuint ImgOpen::LoadTextureFromMemory(const void* data, int size, int& width, int& height) {
    int channels;
    unsigned char* image = stbi_load_from_memory((const unsigned char*)data, size, &width, &height, &channels, STBI_rgb_alpha);
    if (!image) {
        std::cerr << "Failed to load image from memory!" << std::endl;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(image);
    return texture;
}
// -------------------------------------------------------------------------------------------------------------------------------------