//
// Created by patryk on 10.02.26.
//
#pragma once

#include <string>
#include <memory>
#include "BaseTile.h"
#include "../../core/python_kernel/PythonKernel.h"
#include "cells/codeCell.h"
#include "cells/mardownCell.h"



class NotebookTile : public BaseTile {
public:
    NotebookTile(const fs::path& pythonPath);
    ~NotebookTile();

    void Draw() override;
    void Update() override;

    [[nodiscard]]
    std::string getType() const override {return "Notebook";}

    void setKernel(PythonKernel* kernel);

protected:
    std::vector<std::unique_ptr<BaseCell>> cells;

private:
    PythonKernel *kernel;
    inline static const fs::path kerenl_script = "src/ide/gui/core/python_kernel/kernel.py";
    int focus_index = -1;
};
