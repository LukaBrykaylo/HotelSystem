#pragma once
#include "User.h"

class Guest : public User {
public:
    Guest(const pqxx::row& row) : User(row) {
        bonuses = row["bonuses"].as<int>(0);
        payment_info = row["payment_info"].as<std::string>("");
    }

    int getBonuses() const { return bonuses; }

private:
    int bonuses;
    std::string payment_info;
};