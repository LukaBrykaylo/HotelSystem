#include "DatabaseManager.h"
#include <stdexcept>
#include <iostream>

DatabaseManager& DatabaseManager::getInstance() {
	static DatabaseManager instance;
	return instance;
}

void DatabaseManager::init(const std::string& connectionString) {
	m_connectionString = connectionString;
	try {
		pqxx::connection C(m_connectionString);
		if (C.is_open()) {
			std::cout << "Database connection string verified." << std::endl;
		}
		else {
			throw std::runtime_error("DB connection test failed.");
		}
	}
	catch (const std::exception& e) {
		throw std::runtime_error(std::string("Failed to init DB connection: ") + e.what());
	}
}

const std::string& DatabaseManager::getConnectionString() const {
	return m_connectionString;
}
