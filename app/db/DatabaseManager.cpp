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
        m_connection = std::make_unique<pqxx::connection>(m_connectionString);

        if (m_connection->is_open()) {
            std::cout << "Database connection established successfully." << std::endl;
        }
        else {
            std::cerr << "Database connection failed (not open)." << std::endl;
        }
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to connect to DB: ") + e.what());
    }
}

pqxx::connection& DatabaseManager::getConnection() {
    if (!m_connection || !m_connection->is_open()) {
        std::cerr << "Database connection lost. Reconnecting..." << std::endl;
        try {
            m_connection = std::make_unique<pqxx::connection>(m_connectionString);
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to reconnect: " << e.what() << std::endl;
            throw;
        }
    }
    return *m_connection;
}