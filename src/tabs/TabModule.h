//
// Created by tajbe on 30.12.2025.
//
#pragma once

// TODO: jeszcze nie zsumowane do pch
// #include <string>

class TabManager;



/*
 * bazowa klasa abstrakcyjna dla wszystkich modułów wyświetlanych w zakładkach
 * GUICore i inne moduły dziedziczą po tej klasie
 */
class TabModule {
public:
	std::string title = "unnamed tab";
	bool isActive = false;

	// TODO: żeby do np. window mieć dostęp
	TabManager* tabManager = nullptr;

	TabModule(const std::string& tabTitle);
	virtual ~TabModule() = default;

	// TODO: przenieść część init() [albo do konstruktora?] i całe update()
	virtual void update() = 0;

	void setTabManager(TabManager* gui);

	// TODO: serializacja stanu modułu - ALE NARAZIE BEZ TEGO (bo GUICore ma swoją a IDE nie potrzebuje)
};