//
// Created by patryk on 24.06.26.
//

#include "BaseTile.h"

void BaseTile::SetSize(ImVec2 size) {
    floatingSize = size;
}

void BaseTile::SetPos(ImVec2 pos) {
    floatingPos = pos;
}

ImVec2 BaseTile::GetPos() {
    return floatingPos;
}

ImVec2 BaseTile::GetSize() {
    return floatingSize;
}
