//
// Created by patryk on 14.02.26.
//

#include <string>
#include "../../code_editor/cells/mardownCell.h"
#include "../../code_editor/cells/codeCell.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#ifndef FILEUTILS_H
#define FILEUTILS_H

struct FileDecoder{
    std::string filename;
    std::string file_full_path;
};

namespace FileUtils {
    std::vector<std::string> parseKernelList(const std::string& jsonStr);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string readJsonFile(const std::string& path);
    void writeJsonFile(const std::string& path, const std::string& jsonPath);
};


class OpenFile {
public:
    struct CellData {
        std::string cell_type;
        std::string source;
        std::string outputs;
        std::string base64_img;
    };

    static std::map<int, CellData> openIpynb(const std::string& path);
    static std::map<int, CellData> createIpynb_map(const std::vector<std::unique_ptr<BaseCell>>& cells);
    static void writeIpynb(const std::string& path, const std::map<int, CellData>& cells);
    static std::string openPyfile(const std::string& path);

};

class ImgOpen {
public:
    static std::string base64_decode(const std::string& in);
    static GLuint LoadTextureFromMemory(const void* data, int size, int& width, int& height);
};

#endif //FILEUTILS_H
