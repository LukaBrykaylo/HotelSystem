#pragma once

#include <string>
#include <memory>   
#include <pqxx/pqxx>  

class DatabaseManager {
public:
    static DatabaseManager& getInstance();

    void init(const std::string& connectionString);

    pqxx::connection& getConnection();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

private:
    DatabaseManager() = default;

    std::string m_connectionString;

    std::unique_ptr<pqxx::connection> m_connection;
};