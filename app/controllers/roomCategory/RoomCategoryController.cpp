#include "RoomCategoryController.h"

void RoomCategoryController::registerRoutes() {
    CROW_ROUTE(m_app, "/api/categories")
        ([this] {

        std::vector<RoomCategory> categories = m_repo.getAll();

        crow::json::wvalue response;
        response["categories"] = crow::json::wvalue::list();

        int index = 0;
        for (const auto& cat : categories) {
            response["categories"][index]["name"] = cat.name;
            response["categories"][index]["price"] = cat.price;
            response["categories"][index]["room_count"] = cat.room_count;
            index++;
        }

        return crow::response(200, response);
            });
}