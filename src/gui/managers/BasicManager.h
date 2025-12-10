//
// Created by tajbe on 09.12.2025.
//
#pragma once

class GUICore;



class BasicManager {
protected:
	GUICore* guiCore = nullptr;
public:
	void setGUICore(GUICore* gui);

	template<class Archive>
	void serialize(Archive& ar) {
		// klasa bazowa nie ma nic do serializacji, ale potrzebna jest metoda
	}
};