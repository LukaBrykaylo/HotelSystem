#pragma once
#include <memory>
#include <string>
#include "../models/User.h"

class UserRepository {
public:
    UserRepository() = default;

    std::unique_ptr<User> getUserById(int userId);

    // --- НОВИЙ МЕТОД ---
    // Приймає дані, повертає створеного користувача (або nullptr, якщо помилка)
    std::unique_ptr<User> createUser(
        const std::string& username,
        const std::string& email,
        const std::string& password // У нашому випадку - "чистий" пароль
    );

    std::unique_ptr<User> findUserByCredentials(
        const std::string& email,
        const std::string& password
    );
};