//
// Created by patryk on 24.06.26.
//

#include "BaseTab.h"

void BaseTag::SetSize(ImVec2 size) {
    floatingSize = size;
}

void BaseTag::SetPos(ImVec2 pos) {
    floatingPos = pos;
}

ImVec2 BaseTag::GetPos() {
    return floatingPos;
}

ImVec2 BaseTag::GetSize() {
    return floatingSize;
}
