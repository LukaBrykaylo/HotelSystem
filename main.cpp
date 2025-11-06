#define _WIN32_WINNT 0x0A00 
#define ASIO_STANDALONE
#define PQXX_STATIC

#include "crow.h"
#include <pqxx/pqxx>
#include <iostream>
#include <string>

const std::string DB_CONNECTION_STRING =
"dbname=hotelsys user=postgres password=1111 hostaddr=127.0.0.1 port=5432";

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/categories")
        ([] {
        crow::json::wvalue response;
        response["categories"] = crow::json::wvalue::list();

        try
        {
            pqxx::connection C(DB_CONNECTION_STRING);
            pqxx::nontransaction N(C);

            pqxx::result R = N.exec("SELECT name, base_price, number_of_rooms FROM Room_Categories");

            int index = 0;
            for (auto row : R)
            {
                response["categories"][index]["name"] = row["name"].as<std::string>();
                response["categories"][index]["price"] = row["base_price"].as<double>();
                response["categories"][index]["room_count"] = row["number_of_rooms"].as<int>();
                index++;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "DB Query Error: " << e.what() << std::endl;
            return crow::response(500, std::string("Database query failed: ") + e.what());
        }

        return crow::response(200, response);
            });

    CROW_ROUTE(app, "/")
        ([] {
        return "Hotel API is running! Go to /api/categories to see data.";
            });


    std::cout << "Starting Crow server on port 8080..." << std::endl;
    app.port(8080).multithreaded().run();

    return 0;
}