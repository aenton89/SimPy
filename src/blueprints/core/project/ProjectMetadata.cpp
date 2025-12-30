//
// Created by tajbe on 24.10.2025.
//
#include "ProjectMetadata.h"
#include <ctime>
#include <iomanip>
#include <sstream>



ProjectMetadata::ProjectMetadata(): version("0.0.1"), author("student_debil"), description("no description") {
	updateCreationDate();
	updateLastModified();
}

void ProjectMetadata::updateLastModified() {
	lastModifiedDate = getCurrentTimestamp();
}

void ProjectMetadata::updateCreationDate() {
	creationDate = getCurrentTimestamp();
}

std::string ProjectMetadata::getCurrentTimestamp() {
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
	return ss.str();
}