#pragma once

#include "IModel.h"
#include "Service.h"
#include <string>
#include <vector>

struct Booking : public IModel {
	int booking_id;
	int user_id;
	int room_id;
	std::string check_in_date;
	std::string check_out_date;
	double total_cost;
	std::string status;

	std::vector<Service> services;

	void from_row(const pqxx::row& row) override {
		booking_id = row["booking_id"].as<int>();
		user_id = row["user_id"].as<int>();
		room_id = row["room_id"].as<int>();
		check_in_date = row["check_in_date"].as<std::string>();
		check_out_date = row["check_out_date"].as<std::string>();
		total_cost = row["total_cost"].as<double>();
		status = row["status"].as<std::string>();
	}
};
