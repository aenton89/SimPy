//
// Created by patryk on 10.02.26.
//

#ifndef CODECELL_H
#define CODECELL_H

#include "baseCell.h"
#include "../../../include/TextEditor.h"

class CodeCell : public BaseCell {
public:
    virtual ~CodeCell() = default;

    bool Draw(int id) override;

    // pobranie i ustaienie texu w editor
    void setInputText(std::string input) override;
    std::string getInputText() const override;

    CellType getType() override {return CellType::CodeCell;};

private:
    TextEditor Editor;
};


#endif //CODECELL_H
