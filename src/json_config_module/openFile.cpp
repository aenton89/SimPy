//
// Created by patryk on 27.07.25.
//

#include "openFile.h"
#include <string>
#include <nlohmann/json.hpp>
#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include <BaseCell.h>

#include "CodeCell.h"
#include "MarkDownCell.h"

using json = nlohmann::json;

std::map<int, OpenFile::CellData> OpenFile::openIpynb(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć pliku: " << path << std::endl;
        return {};
    }

    json j;
    file >> j;

    std::map<int, CellData> cellMap;

    for (size_t i = 0; i < j["cells"].size(); ++i) {
        const auto& cell = j["cells"][i];

        CellData cellData;
        cellData.cell_type = cell.value("cell_type", "");

        // Łączenie wszystkich linii źródła w jeden string
        if (cell.contains("source") && cell["source"].is_array()) {
            for (const auto& line : cell["source"]) {
                cellData.source += line.get<std::string>();
            }
        }

        // Jeśli to komórka kodu -> zbieramy output
        if (cellData.cell_type == "code" && cell.contains("outputs")) {
            for (const auto& output : cell["outputs"]) {
                if (output.contains("text")) {
                    for (const auto& line : output["text"]) {
                        cellData.outputs = line.get<std::string>();
                    }
                }
            }
        }

        cellMap[i] = cellData;
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

        if (cell.getType() == "code")
        {
            auto& codeCell = dynamic_cast<const Cell&>(cell);
            cellData.cell_type = "code";
            cellData.source = codeCell.getCodeInput();
            cellData.outputs = codeCell.getCodeOutput();
        }
        else
        {
            auto& mdCell = dynamic_cast<const MarkDownCell&>(cell);
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
        cellJson["source"] = json::array();
        cellJson["source"].push_back(cell.source);

        if (cell.cell_type == "code") {
            cellJson["outputs"] = json::array();
            for (const auto& out : cell.outputs) {
                json outputJson;
                outputJson["output_type"] = "stream";
                outputJson["name"] = "stdout";
                outputJson["text"] = out;
                cellJson["outputs"].push_back(outputJson);
            }
            cellJson["execution_count"] = nullptr;
        }

        j["cells"].push_back(cellJson);
    }

    // obowiązkowe pola nagłówkowe notatnika
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
    file << j.dump(4); // zapis z wcięciem
}

void OpenFile::openPyfile(const std::string& path)
{

}
