#pragma once

#include "crow.h"
#include "../../models/RoomCategory.h" 
#include "../../repositories/GenericRepository.h"

class RoomCategoryController {
public:
    RoomCategoryController(
        crow::SimpleApp& app,
        GenericRepository<RoomCategory>& repo
    )
        : m_app(app), m_repo(repo)
    {
        registerRoutes();
    }

private:
    void registerRoutes();

    crow::SimpleApp& m_app;

    GenericRepository<RoomCategory>& m_repo;
};