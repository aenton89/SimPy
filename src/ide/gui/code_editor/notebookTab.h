//
// Created by patryk on 10.02.26.
//

#ifndef NOTEBOOK_H
#define NOTEBOOK_H
#include <string>

#include "BaseTab.h"
#include "../core/python_kernel/PythonKernel.h"

class NotebookTab : public BaseTagType {
public:
    NotebookTab();
    ~NotebookTab() = default;

    void Draw() override;
    void Update() override;

    std::string getType() const override {return "Notebook";}

    void setKernel(PythonKernel* kernel);

private:
    PythonKernel *kernel;

};

#endif //NOTEBOOK_H
