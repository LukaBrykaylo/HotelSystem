#include "AmenityController.h"

void AmenityController::registerRoutes() {

    CROW_ROUTE(m_app, "/api/amenities").methods("GET"_method)
        ([this] {

        auto amenities = m_repo.getAll();

        crow::json::wvalue response;
        response["amenities"] = crow::json::wvalue::list();

        int index = 0;
        for (const auto& amenity : amenities) {
            response["amenities"][index]["amenity_id"] = amenity.amenity_id;
            response["amenities"][index]["name"] = amenity.name;
            response["amenities"][index]["icon_name"] = amenity.icon_name;
            index++;
        }

        return crow::response(200, response);
            });
}
