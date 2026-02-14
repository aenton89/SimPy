//
// Created by patryk on 10.02.26.
//

#ifndef NOTEBOOK_H
#define NOTEBOOK_H
#include <string>

#include "BaseTab.h"
#include "../../core/python_kernel/PythonKernel.h"
#include "cells/codeCell.h"
#include "cells/mardownCell.h"
#include <memory>

class NotebookTab : public BaseTag {
public:
    NotebookTab(const fs::path& pythonPath);
    ~NotebookTab();

    void Draw() override;
    void Update() override;

    std::string getType() const override {return "Notebook";}

    void setKernel(PythonKernel* kernel);

protected:
    std::vector<std::unique_ptr<BaseCell>> cells;

private:
    PythonKernel *kernel;

    inline static const fs::path kerenl_script = "src/ide/gui/core/python_kernel/kernel.py";

    int focus_index = -1;

};

#endif //NOTEBOOK_H
