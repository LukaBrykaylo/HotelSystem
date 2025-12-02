#pragma once

#include "IModel.h"
#include <string>

struct Amenity : public IModel {
	int amenity_id;
	std::string name;
	std::string icon_name;

	void from_row(const pqxx::row& row) override {
		amenity_id = row["amenity_id"].as<int>();
		name = row["name"].as<std::string>();
		if (!row["icon_name"].is_null()) {
			icon_name = row["icon_name"].as<std::string>();
		}
		else {
			icon_name = "";
		}
	}
};
