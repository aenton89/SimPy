//
// Created by patryk on 26.07.25.
//

#include <vector>
#include <string>

#ifndef JSON_MODIFY_H
#define JSON_MODIFY_H

struct FileDecoder{
    std::string filename;
    std::string file_full_path;
};

class FileMenager
{
    public:
        virtual std::vector<std::string> parseKernelList(const std::string& jsonStr);
        virtual std::vector<std::string> split(const std::string& str, char delimiter);
        virtual std::string readJsonFile(const std::string& path);
        virtual void writeJsonFile(const std::string& path, const std::string& jsonPath);
};

#endif //JSON_MODIFY_H
