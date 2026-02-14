//
// Created by patryk on 11.02.26.
//

#include "FileSystemService.h"


bool FileSystemService::CreateFile(const fs::path& parent, const std::string& name) {
    try {
        fs::path fullPath =  parent / name;

        std::ofstream file(fullPath);

        if (fs::exists(fullPath))
            return false;

        file.close();
        return true;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

bool FileSystemService::CreateFolder(const fs::path& parent, const std::string& name) {
    try {
        fs::path fullPath =  parent / name;

        if (!fs::is_directory(fullPath))
            return false;

        fs::create_directories(fullPath);
        return true;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

bool FileSystemService::Move(const fs::path& from, const fs::path& to) {
    try {
        fs::rename(from, to);
        return true;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}


bool FileSystemService::Delete(const fs::path& path, bool force) {
    if (fs::is_regular_file(path)) {
        fs::remove(path);
        return true;
    } else if (fs::is_directory(path)) {
        bool is_empty = fs::is_empty(path);
        if (is_empty) {
            fs::remove(path);
            return true;
        } else if (!force) {
            fs::remove_all(path);
            return true;
        }
        return false;
    }
}
