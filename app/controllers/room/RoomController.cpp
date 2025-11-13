#include "RoomController.h"

void RoomController::registerRoutes() {

    CROW_ROUTE(m_app, "/api/rooms")
        ([this] {

            std::vector<Room> rooms = m_repo.getAll();

            crow::json::wvalue response;
            response["rooms"] = crow::json::wvalue::list();

            int index = 0;
            for (const auto& room : rooms) {
                response["rooms"][index]["room_id"] = room.room_id;
                response["rooms"][index]["category_id"] = room.category_id;
                response["rooms"][index]["room_number"] = room.room_number;
                response["rooms"][index]["capacity"] = room.capacity;
                index++;
            }

            return crow::response(200, response);
        });
}