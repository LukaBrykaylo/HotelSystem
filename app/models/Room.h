#pragma once

#include "IModel.h"
#include <string>
#include <vector>

struct Room : public IModel {
	int room_id;
	int category_id;
	std::string room_number;
	int capacity;

	std::string comments;

	std::vector<std::string> amenities;

	void from_row(const pqxx::row& row) override {
		room_id = row["room_id"].as<int>();
		category_id = row["category_id"].as<int>();
		room_number = row["room_number"].as<std::string>();
		capacity = row["capacity"].as<int>();

		if (!row["comments"].is_null()) {
			comments = row["comments"].as<std::string>();
		}
		else {
			comments = "";
		}
	}
};
