//
// Created by patryk on 23.07.25.
//

#ifndef MARKDOWNCELL_H
#define MARKDOWNCELL_H

#include <imgui.h>
#include <vector>
#include <iostream>
#include "BaseCell.h"

class MarkDownCell : public BaseCell {
public:
    MarkDownCell();
    bool Draw(int id) override;

    void setInput(std::string input);
    std::string getInput() const;

    void execMardown();

    std::string getType() const override {return "MarkDown";}

private:
    char text[1024];
};
#endif //MARKDOWNCELL_H
