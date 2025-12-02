#pragma once
#include <memory>
#include <string>
#include "../models/User.h"

class UserRepository {
public:
	UserRepository() = default;

	std::unique_ptr<User> getUserById(int userId);

	std::unique_ptr<User> createUser(
		const std::string& username,
		const std::string& email,
		const std::string& password
	);

	std::unique_ptr<User> findUserByCredentials(
		const std::string& email,
		const std::string& password
	);

	std::unique_ptr<User> updateUser(
		int userId,
		const std::string& username,
		const std::string& email,
		const std::string& phoneNumber
	);

	std::vector<std::unique_ptr<User>> getAllUsers();
	void addBonuses(pqxx::work& T, int userId, int bonusAmount);
};
