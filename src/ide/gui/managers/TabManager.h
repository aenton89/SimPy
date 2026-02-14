//
// Created by patryk on 14.02.26.
//

#ifndef TABMANAGER_H
#define TABMANAGER_H

class tagManager//: private BaseTagType
{
public:
    void Render(ImVec2 pos_xy, ImVec2 window_size);
    void Update();

    fs::path path2open;
    fs::path lastPathOpened;


    void set_kernel(PythonKernel* kernel);
    void set_path2open(fs::path path);

protected:
    std::vector<std::unique_ptr<BaseTagType>> tags;

private:
};


#endif //TABMANAGER_H
