//
// Created by patryk on 11.02.26.
//

#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

#ifndef FILESYSTEMSERVICE_H
#define FILESYSTEMSERVICE_H

class FileSystemService {
public:
    bool CreateFolder(const fs::path& parent, const std::string& name);
    bool CreateFile(const fs::path& parent, const std::string& name);
    bool Delete(const fs::path& path, bool force = false);
    bool Move(const fs::path& from, const fs::path& to);
};

#endif //FILESYSTEMSERVICE_H
