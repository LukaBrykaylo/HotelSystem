#pragma once
#include "User.h"

class Administrator : public User {
public:
    Administrator(const pqxx::row& row) : User(row) {
        access_level = row["access_level"].as<int>(0);
    }

    int getAccessLevel() const { return access_level; }

private:
    int access_level;
};