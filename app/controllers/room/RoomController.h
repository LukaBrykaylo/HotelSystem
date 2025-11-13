#pragma once

#include "crow.h"
#include "../../models/Room.h"
#include "../../repositories/GenericRepository.h"

class RoomController {
public:
    RoomController(crow::SimpleApp& app, GenericRepository<Room>& repo)
        : m_app(app), m_repo(repo)
    {
        registerRoutes();
    }

private:
    void registerRoutes();

    crow::SimpleApp& m_app;
    GenericRepository<Room>& m_repo;
};