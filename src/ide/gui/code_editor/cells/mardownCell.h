//
// Created by patryk on 10.02.26.
//

#ifndef MARDOWNCELL_H
#define MARDOWNCELL_H

#include "baseCell.h" // Poprawione z codeCell.h

class MardownCell : public BaseCell {
public:
    MardownCell();

    // Implementacja interfejsu BaseCell
    bool Draw(int id) override;
    void setInputText(const std::string& input) override;
    std::string getInputText() const override;

    CellType getType() const override { return CellType::MarkdownCell; }

    // Twoja własna metoda
    void execMardown();

private:
    // Używamy std::string zamiast char[], co ułatwia współpracę z setInputText
    std::string markdown_text;
};

#endif //MARDOWNCELL_H
