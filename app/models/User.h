#pragma once
#include <string>
#include <pqxx/pqxx>

class User {
public:
	enum class Role { GUEST, ADMIN };

	int getUserId() const { return user_id; }
	const std::string& getUsername() const { return username; }
	const std::string& getEmail() const { return email; }
	const std::string& getPhoneNumber() const { return phone_number; }
	Role getRole() const { return role; }

	virtual ~User() = default;

protected:
	User(const pqxx::row& row) {
		user_id = row["user_id"].as<int>();
		username = row["username"].as<std::string>();
		email = row["email"].as<std::string>();

		if (!row["phone_number"].is_null()) {
			phone_number = row["phone_number"].as<std::string>();
		}
		else {
			phone_number = "";
		}

		if (row["role"].as<std::string>() == "admin") {
			role = Role::ADMIN;
		}
		else {
			role = Role::GUEST;
		}
	}

	int user_id;
	std::string username;
	std::string email;
	std::string phone_number;
	Role role;
};
