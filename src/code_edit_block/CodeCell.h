//
// Created by patryk on 23.07.25.
//

#ifndef CODECELL_H
#define CODECELL_H

#pragma once
#include "PythonKernel.h"
#include "BaseCell.h"
#include <imgui.h>
#include <iostream>

class Cell : public BaseCell {
    PythonKernel* notebook;  // tylko wskaźnik
public:
    explicit Cell(PythonKernel& kernel);
    Cell(Cell&& other) noexcept;
    Cell& operator=(Cell&& other) noexcept;

    void setCodeInput(std::string input);
    std::string getCodeInput() const;
    void setCodeOutput(std::string output);
    std::string getCodeOutput() const;

    bool Draw(int id) override;
    void setKernel(PythonKernel* kernel) override;
    std::string getType() const override {return "code";}

private:
    char text[1024*10];
    char output_text[1024*10];
};

#endif // CODECELL_H
