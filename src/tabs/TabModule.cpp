//
// Created by tajbe on 30.12.2025.
//
#include "TabModule.h"



TabModule::TabModule(const std::string& tabTitle) : title(tabTitle) {}

const std::string &TabModule::getTitle() const {
	return title;
}

void TabModule::setTitle(const std::string& newTitle) {
	title = newTitle;
}