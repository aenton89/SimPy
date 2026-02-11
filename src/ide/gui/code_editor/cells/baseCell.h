//
// Created by patryk on 10.02.26.
//

#include <string>
#include <glad/glad.h>

#ifndef BASECELL_H
#define BASECELL_H

#endif //BASECELL_H

enum class CellType
{
    CodeCell,
    MarkdownCell
};

class BaseCell {
public:
    virtual ~BaseCell() = default;
    virtual bool Draw(int id) = 0;

    // do zapsu i do odpalania kodu
    virtual void setInputText(std::string input) = 0;
    virtual std::string getInputText() const = 0;

    virtual CellType getType() = 0;



    bool focused = false;

    std::string input_text;
};