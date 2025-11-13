#include "UserRepository.h"
#include "../models/Guest.h"
#include "../models/Administrator.h"
#include "../db/DatabaseManager.h"
#include <iostream>

// --- Старий метод (без змін) ---
std::unique_ptr<User> UserRepository::getUserById(int userId) {
    try {
        pqxx::connection& C = DatabaseManager::getInstance().getConnection();
        pqxx::nontransaction N(C);
        std::string sql = "SELECT * FROM users WHERE user_id = " + N.quote(userId);
        pqxx::result R = N.exec(sql);

        if (R.empty()) {
            return nullptr;
        }

        auto row = R[0];
        std::string role = row["role"].as<std::string>();

        if (role == "guest") {
            return std::make_unique<Guest>(row);
        }
        else if (role == "admin") {
            return std::make_unique<Administrator>(row);
        }

    }
    catch (const std::exception& e) {
        std::cerr << "UserRepository::getUserById Error: " << e.what() << std::endl;
    }
    return nullptr;
}

// --- НОВИЙ МЕТОД ---
std::unique_ptr<User> UserRepository::createUser(
    const std::string& username,
    const std::string& email,
    const std::string& password
) {
    try {
        pqxx::connection& C = DatabaseManager::getInstance().getConnection();

        // Використовуємо pqxx::work, оскільки це транзакція "читання-запис"
        pqxx::work T(C);

        // Будуємо SQL-запит
        // (Ми зберігаємо "чистий" пароль у поле 'password_hash' - це небезпечно!)
        std::string sql =
            "INSERT INTO users (username, email, password_hash, role) "
            "VALUES (" +
            T.quote(username) + ", " +
            T.quote(email) + ", " +
            T.quote(password) + ", " + 
            T.quote("guest") +
            ") "
            "RETURNING *"; // <-- Повертає нам рядок, який ми щойно вставили

        // Виконуємо запит
        pqxx::result R = T.exec(sql);

        // "Зберігаємо" зміни в БД
        T.commit();

        if (R.empty()) {
            return nullptr; // Щось пішло не так
        }

        // Повертаємо нового Guest, створеного з рядка БД
        return std::make_unique<Guest>(R[0]);

    }
    catch (const pqxx::unique_violation& e) {
        // Ловимо помилку, якщо username або email вже існують
        std::cerr << "UserRepository::createUser Error (Duplicate): " << e.what() << std::endl;
        return nullptr;
    }
    catch (const std::exception& e) {
        // Ловимо всі інші помилки
        std::cerr << "UserRepository::createUser Error: " << e.what() << std::endl;
        return nullptr;
    }
}

std::unique_ptr<User> UserRepository::findUserByCredentials(
    const std::string& email,
    const std::string& password
) {
    try {
        pqxx::connection& C = DatabaseManager::getInstance().getConnection();
        pqxx::nontransaction N(C);

        // Шукаємо користувача з таким email І паролем
        // (Ми шукаємо "чистий" пароль у полі 'password_hash')
        std::string sql =
            "SELECT * FROM users WHERE email = " + N.quote(email) +
            " AND password_hash = " + N.quote(password);

        pqxx::result R = N.exec(sql);

        if (R.empty()) {
            return nullptr; // Комбінація не знайдена
        }

        // --- Фабрика (як у getUserById) ---
        auto row = R[0];
        std::string role = row["role"].as<std::string>();

        if (role == "guest") {
            return std::make_unique<Guest>(row);
        }
        else if (role == "admin") {
            return std::make_unique<Administrator>(row);
        }

    }
    catch (const std::exception& e) {
        std::cerr << "UserRepository::findUserByCredentials Error: " << e.what() << std::endl;
    }

    return nullptr;
}