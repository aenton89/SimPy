//
// Created by tajbe on 30.12.2025.
//
#pragma once



/*
 * manager zarządzający zakładkami w aplikacji
 * przechowuje moduły, obsługuje przełączanie między nimi i renderowanie paska zakładek
 */
class TabManager {
private:
    // TODO: wektor wskaźników do zakładek i rzeczy zwiazane z renderowaniem

public:
    TabManager();
    ~TabManager() = default;

    // TODO: przenieść render(), shutdown(), newframe() i część init()

    // TODO: dodać updateOpen() - aktualizacja tylko używanej zakładki

    // TODO: swoje update() - wywołuje updateOpen() i wykonuje własne UI:
    // TODO: lista z wyborem zakładek na górze, zamykanie i otwieranie zakładek (narazie defaultowo otwiera GUICore)

    // TODO: serializacja jakie zakładki są otwarte - ALE NARAZIE BEZ TEGO
};