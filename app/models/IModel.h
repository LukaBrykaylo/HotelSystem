#pragma once
#include <pqxx/pqxx>

class IModel {
public:
	virtual void from_row(const pqxx::row& row) = 0;

	virtual ~IModel() = default;
};
