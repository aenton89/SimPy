#ifndef BASECELL_H
#define BASECELL_H

#include <string>

enum class CellType {
    CodeCell,
    MarkdownCell
};

class BaseCell {
public:
    virtual ~BaseCell() = default;
    
    // Zwraca true, jeśli użytkownik kliknął "Usuń"
    virtual bool Draw(int id) = 0;

    // Interfejs do zarządzania treścią
    virtual void setInputText(const std::string& input) = 0;
    virtual std::string getInputText() const = 0;

    virtual CellType getType() const = 0;

    bool focused = false;
    bool focusedPrev = false;
};

#endif //BASECELL_H