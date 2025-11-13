#pragma once
#include "crow.h"
#include "../../services/BookingService.h" // <-- Правильна залежність

class BookingController {
public:
    // Конструктор тепер приймає BookingService
    BookingController(crow::SimpleApp& app, BookingService& service);

private:
    void registerRoutes();
    crow::SimpleApp& m_app;
    BookingService& m_service; // <-- Правильна залежність
};