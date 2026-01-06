//
// Created by tajbe on 30.12.2025.
//
#include "TabModule.h"
#include "TabManager.h"



TabModule::TabModule(const std::string& tabTitle) : title(tabTitle) {}

void TabModule::setTabManager(TabManager *manager) {
	tabManager = manager;
}