#include "UserRepository.h"
#include "../models/Guest.h"
#include "../models/Administrator.h"
#include "../db/DatabaseManager.h"
#include "../../libs/picosha2.h"
#include <iostream>
#include <string>

const std::string PASSWORD_SALT = "32jhsdfdjskdafc78634";

std::string hashPassword(const std::string& plainPassword) {
	return picosha2::hash256_hex_string(plainPassword + PASSWORD_SALT);
}

std::unique_ptr<User> UserRepository::getUserById(int userId) {
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::nontransaction N(C);
		std::string sql = "SELECT * FROM users WHERE user_id = " + N.quote(userId);
		pqxx::result R = N.exec(sql);
		if (R.empty()) { return nullptr; }
		auto row = R[0];
		std::string role = row["role"].as<std::string>();
		if (role == "guest") { return std::make_unique<Guest>(row); }
		else if (role == "admin") { return std::make_unique<Administrator>(row); }
	}
	catch (const std::exception& e) { std::cerr << "UserRepository::getUserById Error: " << e.what() << std::endl; }
	return nullptr;
}

std::unique_ptr<User> UserRepository::createUser(const std::string& username, const std::string& email, const std::string& password) {
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::work T(C);

		std::string hashedPassword = hashPassword(password);

		std::string sql = "INSERT INTO users (username, email, password_hash, role) VALUES (" +
			T.quote(username) + ", " + T.quote(email) + ", " +
			T.quote(hashedPassword) + ", " +
			T.quote("guest") + ") RETURNING *";

		pqxx::result R = T.exec(sql);
		T.commit();
		if (R.empty()) { return nullptr; }
		return std::make_unique<Guest>(R[0]);
	}
	catch (const pqxx::unique_violation& e) {
		std::cerr << "UserRepository::createUser Error (Duplicate): " << e.what() << std::endl; return nullptr;
	}
	catch (const std::exception& e) { std::cerr << "UserRepository::createUser Error: " << e.what() << std::endl; return nullptr; }
}

std::unique_ptr<User> UserRepository::findUserByCredentials(const std::string& email, const std::string& password) {
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::nontransaction N(C);

		std::string hashedPassword = hashPassword(password);

		std::string sql = "SELECT * FROM users WHERE email = " + N.quote(email) +
			" AND password_hash = " + N.quote(hashedPassword);

		pqxx::result R = N.exec(sql);
		if (R.empty()) { return nullptr; }

		auto row = R[0];
		std::string role = row["role"].as<std::string>();
		if (role == "guest") { return std::make_unique<Guest>(row); }
		else if (role == "admin") { return std::make_unique<Administrator>(row); }
	}
	catch (const std::exception& e) { std::cerr << "UserRepository::findUserByCredentials Error: " << e.what() << std::endl; }
	return nullptr;
}

std::unique_ptr<User> UserRepository::updateUser(int userId, const std::string& username, const std::string& email, const std::string& phoneNumber) {
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::work T(C);
		std::string sql = "UPDATE users SET "
			"username = " + T.quote(username) + ", "
			"email = " + T.quote(email) + ", "
			"phone_number = " + T.quote(phoneNumber) + " "
			"WHERE user_id = " + std::to_string(userId) + " "
			"RETURNING *";
		pqxx::result R = T.exec(sql);
		T.commit();
		if (R.empty()) { return nullptr; }
		return std::make_unique<Guest>(R[0]);
	}
	catch (const pqxx::unique_violation& e) {
		std::cerr << "UserRepository::updateUser Error (Duplicate): " << e.what() << std::endl; throw;
	}
	catch (const std::exception& e) { std::cerr << "UserRepository::updateUser Error: " << e.what() << std::endl; throw; }
}

std::vector<std::unique_ptr<User>> UserRepository::getAllUsers() {
	std::vector<std::unique_ptr<User>> users;
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::nontransaction N(C);
		std::string sql = "SELECT * FROM users ORDER BY role, user_id";
		pqxx::result R = N.exec(sql);
		for (auto row : R) {
			std::string role = row["role"].as<std::string>();
			if (role == "guest") { users.push_back(std::make_unique<Guest>(row)); }
			else if (role == "admin") { users.push_back(std::make_unique<Administrator>(row)); }
		}
	}
	catch (const std::exception& e) { std::cerr << "UserRepository::getAllUsers Error: " << e.what() << std::endl; }
	return users;
}

void UserRepository::addBonuses(pqxx::work& T, int userId, int bonusAmount) {
	try {
		std::string sql = "UPDATE users SET bonuses = bonuses + "
			+ std::to_string(bonusAmount)
			+ " WHERE user_id = " + std::to_string(userId);
		T.exec(sql);
	}
	catch (const std::exception& e) {
		std::cerr << "UserRepository::addBonuses Error: " << e.what() << std::endl;
		throw;
	}
}
