#include "UserController.h"
#include "../../models/Guest.h"
#include "../../models/Administrator.h"

void UserController::registerRoutes() {

    // --- Старий GET маршрут ---
    CROW_ROUTE(m_app, "/api/users/<int>")
        ([this](int userId) {
        std::unique_ptr<User> user = m_repo.getUserById(userId);
        if (!user) {
            return crow::response(404, "User not found");
        }

        crow::json::wvalue response;
        response["user_id"] = user->getUserId();
        response["username"] = user->getUsername();
        response["email"] = user->getEmail();

        if (user->getRole() == User::Role::GUEST) {
            Guest* guest = static_cast<Guest*>(user.get());
            response["role"] = "guest";
            response["bonuses"] = guest->getBonuses();
        }
        else if (user->getRole() == User::Role::ADMIN) {
            Administrator* admin = static_cast<Administrator*>(user.get());
            response["role"] = "admin";
            response["access_level"] = admin->getAccessLevel();
        }
        return crow::response(200, response);
            });

    // --- НОВИЙ POST МАРШРУТ (РЕЄСТРАЦІЯ) ---
    CROW_ROUTE(m_app, "/api/register").methods("POST"_method)
        ([this](const crow::request& req) {

        // 1. Парсимо вхідний JSON
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        // 2. Валідація (перевірка, чи є всі поля)
        if (!body.has("username") || !body.has("email") || !body.has("password")) {
            return crow::response(400, "Missing required fields (username, email, password)");
        }

        std::string username = body["username"].s();
        std::string email = body["email"].s();
        std::string password = body["password"].s();

        if (username.empty() || email.empty() || password.empty()) {
            return crow::response(400, "Fields cannot be empty");
        }

        // 3. Викликаємо репозиторій для створення користувача
        auto newUser = m_repo.createUser(username, email, password);

        // 4. Обробка помилок (напр., такий email вже є)
        if (!newUser) {
            return crow::response(409, "Username or email already exists"); // 409 Conflict
        }

        // 5. Успіх! Повертаємо дані нового користувача (але без пароля)
        crow::json::wvalue response;
        response["user_id"] = newUser->getUserId();
        response["username"] = newUser->getUsername();
        response["email"] = newUser->getEmail();
        response["role"] = "guest";

        // Повертаємо 201 Created - стандарт для створення нового ресурсу
        return crow::response(201, response);
            });

    CROW_ROUTE(m_app, "/api/login").methods("POST"_method)
        ([this](const crow::request& req) {

        auto body = crow::json::load(req.body);
        if (!body || !body.has("email") || !body.has("password")) {
            return crow::response(400, "Missing required fields (email, password)");
        }

        std::string email = body["email"].s();
        std::string password = body["password"].s();

        // 1. Викликаємо репозиторій
        auto user = m_repo.findUserByCredentials(email, password);

        // 2. Якщо логін/пароль неправильні
        if (!user) {
            // 401 Unauthorized - стандарт для невдалого логіну
            return crow::response(401, "Invalid email or password");
        }

        // 3. Успіх!
        crow::json::wvalue response;
        response["message"] = "Login successful";
        response["username"] = user->getUsername();

        // У реальному житті ми б тут повернули JWT-токен
        response["token"] = "DUMMY_TOKEN_USER_ID_" + std::to_string(user->getUserId());

        return crow::response(200, response);
            });
}