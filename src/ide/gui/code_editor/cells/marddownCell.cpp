//
// Created by patryk on 10.02.26.
//

#include "mardownCell.h"


MardownCell::MardownCell() {

}

bool MardownCell::Draw(int id) {

}

void MardownCell::setInputText(const std::string& input) {
    this->markdown_text = input;
}

std::string MardownCell::getInputText() const {
    return this->markdown_text;
}

void MardownCell::execMardown() {

}




