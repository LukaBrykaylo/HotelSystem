#pragma once

#include "crow.h"
#include "../../middleware/CorsMiddleware.h"
#include "../../db/DatabaseManager.h"

class ReportController {
public:
    ReportController(crow::App<CorsMiddleware>& app)
        : m_app(app)
    {
        registerRoutes();
    }

private:
    void registerRoutes();
    crow::App<CorsMiddleware>& m_app;
};