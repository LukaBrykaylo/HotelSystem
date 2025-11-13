#pragma once
#include "crow.h"
#include "../../models/Service.h"
#include "../../repositories/GenericRepository.h"

class ServiceController {
public:
    // Конструктор має приймати GenericRepository<Service>
    ServiceController(crow::SimpleApp& app, GenericRepository<Service>& repo)
        : m_app(app), m_repo(repo)
    {
        registerRoutes();
    }

private:
    void registerRoutes();
    crow::SimpleApp& m_app;
    GenericRepository<Service>& m_repo;
};