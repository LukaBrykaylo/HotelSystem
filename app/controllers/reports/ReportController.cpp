#include "ReportController.h"
#include <pqxx/pqxx>
#include <iostream>

void ReportController::registerRoutes() {
    CROW_ROUTE(m_app, "/api/admin/reports/financials").methods("GET"_method)
        ([this] {
        try {
            pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
            pqxx::nontransaction N(C);

            std::string sql =
                "SELECT TO_CHAR(check_in_date, 'YYYY-MM') AS month, "
                "SUM(total_cost) AS total_revenue "
                "FROM bookings "
                "WHERE status = 'completed' "
                "GROUP BY month "
                "ORDER BY month DESC";

            pqxx::result R = N.exec(sql);

            crow::json::wvalue response;
            response["monthly_revenue"] = crow::json::wvalue::list();
            int index = 0;
            for (auto row : R) {
                response["monthly_revenue"][index]["month"] = row["month"].c_str();
                response["monthly_revenue"][index]["revenue"] = row["total_revenue"].as<double>();
                index++;
            }
            return crow::response(200, response);

        }
        catch (const std::exception& e) {
            std::cerr << "ReportController::financials Error: " << e.what() << std::endl;
            return crow::response(500, e.what());
        }
            });

    CROW_ROUTE(m_app, "/api/admin/reports/top_clients").methods("GET"_method)
        ([this] {
        try {
            pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
            pqxx::nontransaction N(C);

            std::string sql =
                "SELECT u.user_id, u.username, u.email, "
                "COUNT(b.booking_id) AS total_bookings, "
                "SUM(b.total_cost) AS total_spent "
                "FROM bookings b "
                "JOIN users u ON b.user_id = u.user_id "
                "WHERE b.status = 'completed' "
                "GROUP BY u.user_id, u.username, u.email "
                "ORDER BY total_spent DESC "
                "LIMIT 5";

            pqxx::result R = N.exec(sql);

            crow::json::wvalue response;
            response["top_clients"] = crow::json::wvalue::list();
            int index = 0;
            for (auto row : R) {
                response["top_clients"][index]["user_id"] = row["user_id"].as<int>();
                response["top_clients"][index]["username"] = row["username"].c_str();
                response["top_clients"][index]["email"] = row["email"].c_str();
                response["top_clients"][index]["total_bookings"] = row["total_bookings"].as<int>();
                response["top_clients"][index]["total_spent"] = row["total_spent"].as<double>();
                index++;
            }
            return crow::response(200, response);

        }
        catch (const std::exception& e) {
            std::cerr << "ReportController::top_clients Error: " << e.what() << std::endl;
            return crow::response(500, e.what());
        }
            });

    CROW_ROUTE(m_app, "/api/admin/reports/category_popularity").methods("GET"_method)
        ([this] {
        try {
            pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
            pqxx::nontransaction N(C);

            std::string sql =
                "SELECT rc.name, COUNT(b.booking_id) AS booking_count "
                "FROM room_categories rc "
                "LEFT JOIN rooms r ON rc.category_id = r.category_id "
                "LEFT JOIN bookings b ON r.room_id = b.room_id "
                "GROUP BY rc.name "
                "ORDER BY booking_count DESC";

            pqxx::result R = N.exec(sql);

            crow::json::wvalue response;
            response["popularity"] = crow::json::wvalue::list();
            int index = 0;
            for (auto row : R) {
                response["popularity"][index]["category"] = row["name"].c_str();
                response["popularity"][index]["count"] = row["booking_count"].as<int>();
                index++;
            }
            return crow::response(200, response);

        }
        catch (const std::exception& e) {
            std::cerr << "ReportController::category_popularity Error: " << e.what() << std::endl;
            return crow::response(500, e.what());
        }
            });

    CROW_ROUTE(m_app, "/api/admin/reports/occupancy").methods("GET"_method)
        ([this] {
        try {
            pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
            pqxx::nontransaction N(C);

            std::string sql =
                "SELECT "
                "  r.room_id, r.room_number, rc.name as category, "
                "  b.booking_id, u.username, b.check_out_date, "
                "  CASE WHEN b.booking_id IS NOT NULL THEN 'occupied' ELSE 'available' END as status "
                "FROM rooms r "
                "JOIN room_categories rc ON r.category_id = rc.category_id "
                "LEFT JOIN bookings b ON r.room_id = b.room_id "
                "  AND b.status != 'cancelled' "
                "  AND CURRENT_DATE >= b.check_in_date "
                "  AND CURRENT_DATE < b.check_out_date "
                "LEFT JOIN users u ON b.user_id = u.user_id "
                "ORDER BY r.room_number";

            pqxx::result R = N.exec(sql);

            crow::json::wvalue response;
            response["occupancy"] = crow::json::wvalue::list();
            int index = 0;
            for (auto row : R) {
                response["occupancy"][index]["room_id"] = row["room_id"].as<int>();
                response["occupancy"][index]["room_number"] = row["room_number"].c_str();
                response["occupancy"][index]["category"] = row["category"].c_str();
                response["occupancy"][index]["status"] = row["status"].c_str();

                if (!row["booking_id"].is_null()) {
                    response["occupancy"][index]["guest_name"] = row["username"].c_str();
                    response["occupancy"][index]["check_out"] = row["check_out_date"].c_str();
                }
                else {
                    response["occupancy"][index]["guest_name"] = "";
                    response["occupancy"][index]["check_out"] = "";
                }
                index++;
            }
            return crow::response(200, response);

        }
        catch (const std::exception& e) {
            std::cerr << "ReportController::occupancy Error: " << e.what() << std::endl;
            return crow::response(500, e.what());
        }
            });
}
