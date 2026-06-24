//
// Created by patryk on 14.02.26.
//
#pragma once

#include <filesystem>

namespace fs = std::filesystem;



class BaseTile {
public:
    virtual ~BaseTile() = default;
    virtual void Draw() = 0;
    virtual void Update() = 0;

    void SetSize(ImVec2 size);
    void SetPos(ImVec2 pos);

    ImVec2 GetPos();
    ImVec2 GetSize();

    virtual std::string getType() const = 0;
    fs::path getPath() {return path2open;}

    fs::path path2open;
    std::string filename;

    bool isFloating = false;

private:
    ImVec2 floatingPos;
    ImVec2 floatingSize;
};
