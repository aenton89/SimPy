//
// Created by tajbe on 30.12.2025.
//
#pragma once
// #include <string>
#include "../files/FileManager.h"

class TabManager;



/*
 * bazowa klasa abstrakcyjna dla wszystkich modułów wyświetlanych w zakładkach
 * blueprintTab i inne moduły dziedziczą po tej klasie
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
	virtual void update();
	void drawMenuBar();
	virtual void menuBarFile() {};
	virtual void menuBarEdit() {};
	virtual void menuBarSettings() {};
	virtual void menuBarOther() {};

	void setTabManager(TabManager* gui);

	// rysowanie kontetnu w topbar dla konetenego taba
	virtual void drawTopBarContent() {};
	void drawTopBar();
	float topBarHeight = 50.0f;

	// Reagowanie na otwarcie workspace
	ImVec2 GetPose();
	ImVec2 GetSize();

	// TODO: serializacja stanu modułu - ALE NARAZIE BEZ TEGO (bo blueprintTab ma swoją a IDE nie potrzebuje)

	// File manager tutaj (jeden w spolny dla wszyskich)
	FileManager* fileManager = nullptr;
	void setFileManager(FileManager* gui);
};