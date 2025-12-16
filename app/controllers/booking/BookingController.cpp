#include "BookingController.h"
#include "../../services/BookingService.h" 
#include <iostream>

BookingController::BookingController(crow::App<CorsMiddleware>& app, BookingService& service)
    : m_app(app), m_service(service)
{
    registerRoutes();
}

void BookingController::registerRoutes() {

    CROW_ROUTE(m_app, "/api/bookings").methods("POST"_method)
        ([this](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        if (!body.has("user_id") || !body.has("room_id") || !body.has("check_in") || !body.has("check_out")) {
            return crow::response(400, "Missing required fields");
        }

        int userId = body["user_id"].i();
        int roomId = body["room_id"].i();
        std::string checkIn = body["check_in"].s();
        std::string checkOut = body["check_out"].s();

        bool useBonuses = false;
        if (body.has("use_bonuses")) useBonuses = body["use_bonuses"].b();

        std::vector<int> serviceIds;
        if (body.has("services")) {
            for (auto& service_id : body["services"]) {
                serviceIds.push_back(service_id.i());
            }
        }

        try {
            auto booking = m_service.createNewBooking(userId, roomId, checkIn, checkOut, serviceIds, useBonuses);

            crow::json::wvalue response;
            response["message"] = "Booking created successfully";
            response["booking_id"] = booking->booking_id;
            response["total_cost"] = booking->total_cost;

            return crow::response(201, response);
        }
        catch (const BookingException& e) {
            return crow::response(409, e.what());
        }
        catch (const std::exception& e) {
            return crow::response(500, e.what());
        }
            });

    CROW_ROUTE(m_app, "/api/bookings").methods("GET"_method)
        ([this] {
        try {
            auto R = m_service.getAllBookingDetails();
            crow::json::wvalue response;
            response["bookings"] = crow::json::wvalue::list();
            int index = 0;
            for (const auto& row : R) {
                response["bookings"][index]["booking_id"] = row["booking_id"].as<int>();
                response["bookings"][index]["user_id"] = row["user_id"].as<int>();
                response["bookings"][index]["room_id"] = row["room_id"].as<int>();
                response["bookings"][index]["username"] = row["username"].c_str();
                response["bookings"][index]["room_number"] = row["room_number"].c_str();
                response["bookings"][index]["check_in_date"] = row["check_in_date"].c_str();
                response["bookings"][index]["check_out_date"] = row["check_out_date"].c_str();
                response["bookings"][index]["total_cost"] = row["total_cost"].as<double>();
                response["bookings"][index]["status"] = row["status"].c_str();
                index++;
            }
            return crow::response(200, response);
        }
        catch (const std::exception& e) {
            return crow::response(500, e.what());
        }
            });

    CROW_ROUTE(m_app, "/api/bookings/<int>").methods("GET"_method)
        ([this](int bookingId) {
        try {
            auto booking = m_service.getBookingDetails(bookingId);
            if (!booking) return crow::response(404, "Booking not found");

            crow::json::wvalue response;
            response["booking_id"] = booking->booking_id;
            response["user_id"] = booking->user_id;
            response["room_id"] = booking->room_id;
            response["check_in_date"] = booking->check_in_date;
            response["check_out_date"] = booking->check_out_date;
            response["total_cost"] = booking->total_cost;
            response["status"] = booking->status;

            response["services"] = crow::json::wvalue::list();
            int service_index = 0;
            for (const auto& s : booking->services) {
                response["services"][service_index]["service_id"] = s.service_id;
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

    CROW_ROUTE(m_app, "/api/bookings/<int>").methods("PUT"_method)
        ([this](const crow::request& req, int bookingId) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        int userId = body["user_id"].i();
        int roomId = body["room_id"].i();
        std::string checkIn = body["check_in_date"].s();
        std::string checkOut = body["check_out_date"].s();
        std::string status = body["status"].s();

        try {
            auto booking = m_service.adminUpdateBooking(bookingId, userId, roomId, checkIn, checkOut, status);
            if (!booking) return crow::response(404, "Booking not found");
            return crow::response(200, "Booking updated");
        }
        catch (const std::exception& e) {
            return crow::response(500, e.what());
        }
            });

    CROW_ROUTE(m_app, "/api/bookings/<int>").methods("DELETE"_method)
        ([this](int bookingId) {
        try {
            if (m_service.deleteBooking(bookingId)) {
                return crow::response(204);
            }
            else {
                return crow::response(404, "Booking not found");
            }
        }
        catch (const std::exception& e) {
            return crow::response(500, e.what());
        }
            });

    CROW_ROUTE(m_app, "/api/bookings/<int>/cancel").methods("PUT"_method)
        ([this](const crow::request& req, int bookingId) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("user_id")) return crow::response(400, "Missing user_id");
        int userId = body["user_id"].i();

        try {
            m_service.cancelBookingByUser(bookingId, userId);
            return crow::response(200, "Booking cancelled");
        }
        catch (const std::exception& e) {
            return crow::response(500, e.what());
        }
            });

    CROW_ROUTE(m_app, "/api/bookings/<int>/extend").methods("PUT"_method)
        ([this](const crow::request& req, int bookingId) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("user_id") || !body.has("new_check_out")) return crow::response(400, "Missing fields");
        int userId = body["user_id"].i();
        std::string newCheckOut = body["new_check_out"].s();

        try {
            m_service.extendBooking(bookingId, userId, newCheckOut);
            return crow::response(200, "Booking extended");
        }
        catch (const std::exception& e) {
            return crow::response(500, e.what());
        }
            });
}
