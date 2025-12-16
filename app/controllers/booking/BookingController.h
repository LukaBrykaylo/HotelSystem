#pragma once
#include "crow.h"
#include "../../services/BookingService.h" 
#include "../../middleware/CorsMiddleware.h"

class BookingController {
public:
    BookingController(crow::App<CorsMiddleware>& app, BookingService& service);

private:
    void registerRoutes();
    crow::App<CorsMiddleware>& m_app;
    BookingService& m_service;
};
