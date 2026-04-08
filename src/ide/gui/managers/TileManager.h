//
// Created by patryk on 14.02.26.
//
#pragma once


// : private BaseTileType
class TileManager {
public:
    void Render(ImVec2 pos_xy, ImVec2 window_size);
    void Update();

    fs::path path2open;
    fs::path lastPathOpened;

    void set_kernel(PythonKernel* kernel);
    void set_path2open(fs::path path);

protected:
    std::vector<std::unique_ptr<BaseTileType>> tags;
};
