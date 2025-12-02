#pragma once
#include "User.h"

class Administrator : public User {
public:
	Administrator(const pqxx::row& row) : User(row) {
		if (!row["access_level"].is_null()) {
			access_level = row["access_level"].as<int>();
		}
		else {
			access_level = 0;
		}
	}

	int getAccessLevel() const { return access_level; }

private:
	int access_level;
};
