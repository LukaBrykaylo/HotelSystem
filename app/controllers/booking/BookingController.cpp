#include "BookingController.h"
#include "../../services/BookingService.h" 
#include <iostream>

// --- КОНСТРУКТОР ---
BookingController::BookingController(crow::SimpleApp& app, BookingService& service)
    : m_app(app), m_service(service)
{
    registerRoutes();
}

void BookingController::registerRoutes() {

    // --- СТВОРЕННЯ БРОНЮВАННЯ (POST) ---
    CROW_ROUTE(m_app, "/api/bookings").methods("POST"_method)
        ([this](const crow::request& req) {

        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        // (Тут має бути валідація JSON)
        int userId = body["user_id"].i();
        int roomId = body["room_id"].i();
        std::string checkIn = body["check_in"].s();
        std::string checkOut = body["check_out"].s();

        std::vector<int> serviceIds;
        auto& services_json = body["services"];
        for (auto& service : services_json) {
            serviceIds.push_back(service.i());
        }

        try {
            // Викликаємо "мозок"
            auto booking = m_service.createNewBooking(userId, roomId, checkIn, checkOut, serviceIds);

            crow::json::wvalue response;
            response["message"] = "Booking created successfully";
            response["booking_id"] = booking->booking_id;
            response["total_cost"] = booking->total_cost;

            return crow::response(201, response);

        }
        catch (const BookingException& e) {
            return crow::response(409, e.what()); // 409 Conflict
        }
        catch (const std::exception& e) {
            return crow::response(500, e.what());
        }
            });

    // --- ОТРИМАННЯ ОДНОГО БРОНЮВАННЯ (GET) ---
    // (Ми додамо логіку в Service, щоб це працювало)
    CROW_ROUTE(m_app, "/api/bookings/<int>")
        ([this](int bookingId) {

        try {
            auto booking = m_service.getBookingDetails(bookingId); // Викликаємо Сервіс

            if (!booking) {
                return crow::response(404, "Booking not found");
            }

            crow::json::wvalue response;
            response["booking_id"] = booking->booking_id;
            response["user_id"] = booking->user_id;
            response["room_id"] = booking->room_id;
            response["total_cost"] = booking->total_cost;
            response["status"] = booking->status;

            response["services"] = crow::json::wvalue::list();
            int service_index = 0;
            for (const auto& s : booking->services) {
                response["services"][service_index]["name"] = s.name;
                response["services"][service_index]["price"] = s.price;
                service_index++;
            }
            return crow::response(200, response);

        }
        catch (const std::exception& e) {
            return crow::response(500, e.what());
        }
            });
}