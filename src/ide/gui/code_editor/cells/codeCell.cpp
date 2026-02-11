//
// Created by patryk on 10.02.26.
//

#include "codeCell.h"

bool CodeCell::Draw(int id) {

}

void CodeCell::setInputText(std::string input) {
    this->Editor.SetText(input);
}

std::string CodeCell::getInputText() const {
    return this->Editor.GetText();
}




