//
// Created by patryk on 10.02.26.
//

#ifndef MARDOWNCELL_H
#define MARDOWNCELL_H

#include "baseCell.h"

class MardownCell : public BaseCell {
public:
    MardownCell();

    bool Draw(int id) override;
    void setInputText(const std::string& input) override;
    std::string getInputText() const override;

    CellType getType() const override { return CellType::MarkdownCell; }

    void execMardown();

private:
    std::string markdown_text;

    bool is_preview = false;
};

#endif //MARDOWNCELL_H
