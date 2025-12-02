#pragma once

#include "../db/DatabaseManager.h"
#include "../models/IModel.h"
#include <vector>
#include <string>
#include <iostream>

template <typename T>
class GenericRepository {
public:
	GenericRepository(const std::string& tableName) : m_tableName(tableName) {}

	std::vector<T> getAll() {
		std::vector<T> results;
		try {
			pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
			pqxx::nontransaction N(C);

			std::string sql = "SELECT * FROM " + N.quote_name(m_tableName);

			pqxx::result R = N.exec(sql);

			for (auto row : R) {
				T model;
				model.from_row(row);
				results.push_back(model);
			}
		}
		catch (const std::exception& e) {
			std::cerr << "GenericRepository::getAll Error ("
				<< m_tableName << "): " << e.what() << std::endl;
		}
		return results;
	}

private:
	std::string m_tableName;
};
