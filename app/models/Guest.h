#pragma once
#include "User.h"

class Guest : public User {
public:
	Guest(const pqxx::row& row) : User(row) {
		if (!row["bonuses"].is_null()) {
			bonuses = row["bonuses"].as<int>();
		}
		else {
			bonuses = 0;
		}

		if (!row["payment_info"].is_null()) {
			payment_info = row["payment_info"].as<std::string>();
		}
		else {
			payment_info = "";
		}
	}

	int getBonuses() const { return bonuses; }
	const std::string& getPaymentInfo() const { return payment_info; }

private:
	int bonuses;
	std::string payment_info;
};
