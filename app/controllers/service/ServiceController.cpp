#include "ServiceController.h"

void ServiceController::registerRoutes() {
    CROW_ROUTE(m_app, "/api/services")
        ([this] {
        std::vector<Service> services = m_repo.getAll();
        crow::json::wvalue response;
        response["services"] = crow::json::wvalue::list();

        int index = 0;
        for (const auto& s : services) {
            response["services"][index]["id"] = s.service_id;
            response["services"][index]["name"] = s.name;
            response["services"][index]["price"] = s.price;
            index++;
        }
        return crow::response(200, response);
            });
}