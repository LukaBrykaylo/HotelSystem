#pragma once
#include "IModel.h"
#include <string>

struct Service : public IModel {
	int service_id;
	std::string name;
	double price;

	void from_row(const pqxx::row& row) override {
		service_id = row["service_id"].as<int>();
		name = row["name"].as<std::string>();
		price = row["price"].as<double>();
	}
};
