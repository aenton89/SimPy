//
// Created by patryk on 14.02.26.
//

#include <filesystem>

namespace fs = std::filesystem;

#ifndef BASETAB_H
#define BASETAB_H

class BaseTagType
{
public:
    virtual ~BaseTagType() = default;
    virtual void Draw() = 0;
    virtual void Update() = 0;

    virtual std::string getType() const = 0;
    fs::path getPath() {return path2open;}

    fs::path path2open;
    std::string filename;

    bool isFloating = false;
    ImVec2 floatingPos = ImVec2(100, 100);
    ImVec2 floatingSize = ImVec2(300, 300);
};


#endif //BASETAB_H
