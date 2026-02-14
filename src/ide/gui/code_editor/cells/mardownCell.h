//
// Created by patryk on 10.02.26.
//

#ifndef MARDOWNCELL_H
#define MARDOWNCELL_H
#include "codeCell.h"

class MardownCell : public BaseCell {
public:
    MardownCell();
    bool Draw(int id) override;

    void setInput(std::string input);
    std::string getInput() const;

    void execMardown();

    CellType getType() const override {return CellType::MarkdownCell;}

private:
    char text[10*1024];
};

#endif //MARDOWNCELL_H
