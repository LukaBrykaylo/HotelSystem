#pragma once

#include "crow.h"
#include "../../repositories/UserRepository.h"

class UserController {
public:
    UserController(crow::SimpleApp& app, UserRepository& repo)
        : m_app(app), m_repo(repo)
    {
        registerRoutes();
    }

private:
    void registerRoutes();

    crow::SimpleApp& m_app;
    UserRepository& m_repo;
};