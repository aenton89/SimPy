//
// Created by tajbe on 24.10.2025.
//
#pragma once

#include <chrono>
// #include <string>
// #include <cereal/archives/xml.hpp>



/*
 * stores project metadata (version, creation date, etc.)
 * used for saving/loading files AND to indicate if file has unsaved changees
 * TODO: w ogóle to zaimplementować w użyciu xd
 * TODO: do sprawdzania czy plik był modyfikowany
 */
class ProjectMetadata {
private:
	std::string version;
	std::string creationDate;
	std::string lastModifiedDate;
	std::string author;
	std::string description;

	// helper to get current timestamp
	static std::string getCurrentTimestamp();

public:
	ProjectMetadata();

	// update timestamps
	void updateLastModified();
	void updateCreationDate();

	template<class Archive>
	void serialize(Archive& ar) {
		ar(cereal::make_nvp("version", version),
		   cereal::make_nvp("creationDate", creationDate),
		   cereal::make_nvp("lastModifiedDate", lastModifiedDate),
		   cereal::make_nvp("author", author),
		   cereal::make_nvp("description", description));
	}
};