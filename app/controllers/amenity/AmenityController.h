#pragma once

#include "crow.h"
#include "../../models/Amenity.h"
#include "../../repositories/GenericRepository.h"
#include "../../middleware/CorsMiddleware.h"

class AmenityController {
public:
    AmenityController(crow::App<CorsMiddleware>& app, GenericRepository<Amenity>& repo)
        : m_app(app), m_repo(repo)
    {
        registerRoutes();
    }

private:
    void registerRoutes();
    crow::App<CorsMiddleware>& m_app;
    GenericRepository<Amenity>& m_repo;
};
