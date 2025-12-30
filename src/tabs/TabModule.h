//
// Created by tajbe on 30.12.2025.
//
#pragma once

// TODO: jeszcze nie zsumowane do pch
// #include <string>



/*
 * bazowa klasa abstrakcyjna dla wszystkich modułów wyświetlanych w zakładkach
 * GUICore i inne moduły dziedziczą po tej klasie
 */
class TabModule {
public:
	TabModule(const std::string& tabTitle);
	virtual ~TabModule() = default;

	// TODO: przenieść część init() i całe update()

	const std::string& getTitle() const;
	void setTitle(const std::string& newTitle);

	// TODO: serializacja stanu modułu - ALE NARAZIE BEZ TEGO (bo GUICore ma swoją a IDE nie potrzebuje)
protected:
	std::string title;
	bool isActive = false;
};