#pragma once

#include <string>
#include <memory>
#include <pqxx/pqxx>

class DatabaseManager {
public:
	DatabaseManager(const DatabaseManager&) = delete;
	DatabaseManager& operator=(const DatabaseManager&) = delete;

	static DatabaseManager& getInstance();

	void init(const std::string& connectionString);

	const std::string& getConnectionString() const;

private:
	DatabaseManager() = default;

	std::string m_connectionString;

};
