#define _WIN32_WINNT 0x0A00 

#include "crow.h"
#include <iostream>
#include <string>

// --- Базові сервіси ---
#include "app/db/DatabaseManager.h"
#include "app/repositories/GenericRepository.h"
#include "app/repositories/UserRepository.h"
#include "app/repositories/BookingRepository.h" 

// --- Моделі ---
#include "app/models/RoomCategory.h"
#include "app/models/Room.h"
#include "app/models/User.h"
#include "app/models/Service.h" 
#include "app/models/Booking.h" 

// --- Сервіси (Бізнес-логіка) ---
#include "app/services/BookingService.h" // <-- ДОДАЄМО

// --- Контролери ---
#include "app/controllers/roomCategory/RoomCategoryController.h"
#include "app/controllers/room/RoomController.h"
#include "app/controllers/user/UserController.h"
#include "app/controllers/service/ServiceController.h" 
#include "app/controllers/booking/BookingController.h" 

const std::string DB_CONNECTION_STRING =
"dbname=hotelsys user=postgres password=1111 hostaddr=127.0.0.1 port=5432";

int main()
{
    try {
        std::cout << "Initializing Database Manager..." << std::endl;
        DatabaseManager::getInstance().init(DB_CONNECTION_STRING);

        crow::SimpleApp app;

        // --- Створюємо Репозиторії (Шар даних) ---
        GenericRepository<RoomCategory> categoryRepo("room_categories");
        GenericRepository<Room> roomRepo("rooms");
        UserRepository userRepo;
        GenericRepository<Service> serviceRepo("services");
        BookingRepository bookingRepo;

        // --- Створюємо Сервіси (Шар логіки) ---
        BookingService bookingService(bookingRepo, roomRepo, serviceRepo);

        // --- Створюємо Контролери (Шар API) ---
        RoomCategoryController categoryController(app, categoryRepo);
        RoomController roomController(app, roomRepo);
        UserController userController(app, userRepo);
        ServiceController serviceController(app, serviceRepo);

        // "Впорскуємо" BookingService у контролер
        BookingController bookingController(app, bookingService);

        CROW_ROUTE(app, "/")
            ([] {
            return "Hotel API is running! (v6.0 Booking Service)";
                });

        std::cout << "Starting Crow server on port 8080..." << std::endl;
        app.port(8080).multithreaded().run();
    }
    catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}