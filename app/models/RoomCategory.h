#pragma once
#include "IModel.h"
#include <string>

struct RoomCategory : public IModel {
	int id;
	std::string name;
	double price;
	int room_count;

	void from_row(const pqxx::row& row) override {
		id = row["category_id"].as<int>();
		name = row["name"].as<std::string>();
		price = row["base_price"].as<double>();
		room_count = row["number_of_rooms"].as<int>();
	}
};
