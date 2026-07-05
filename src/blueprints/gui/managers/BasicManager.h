//
// Created by tajbe on 09.12.2025.
//
#pragma once

class BluePrintTab;


class BasicManager {
protected:
	BluePrintTab* blueprintTab = nullptr;
public:
	void setBluePrintTab(BluePrintTab* gui);

	template<class Archive>
	void serialize(Archive& ar) {
		// klasa bazowa nie ma nic do serializacji, ale potrzebna jest metoda
	}
};