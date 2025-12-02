#define _WIN32_WINNT 0x0A00 

#include "crow.h"
#include <iostream>
#include <string>

#include "app/db/DatabaseManager.h"
#include "app/repositories/GenericRepository.h"
#include "app/repositories/UserRepository.h"
#include "app/repositories/BookingRepository.h" 
#include "app/repositories/RoomRepository.h" 

#include "app/models/RoomCategory.h"
#include "app/models/Room.h"
#include "app/models/User.h"
#include "app/models/Service.h" 
#include "app/models/Booking.h" 
#include "app/models/Amenity.h" 

#include "app/services/BookingService.h" 

#include "app/controllers/roomCategory/RoomCategoryController.h"
#include "app/controllers/room/RoomController.h"
#include "app/controllers/user/UserController.h"
#include "app/controllers/service/ServiceController.h" 
#include "app/controllers/booking/BookingController.h" 
#include "app/controllers/amenity/AmenityController.h" 
#include "app/controllers/reports/ReportController.h"
#include "app/middleware/CorsMiddleware.h"

const std::string DB_CONNECTION_STRING =
"dbname=hotelsys user=postgres password=1111 hostaddr=127.0.0.1 port=5432";

int main()
{
	try {
		std::cout << "Initializing Database Manager..." << std::endl;
		DatabaseManager::getInstance().init(DB_CONNECTION_STRING);

		crow::App<CorsMiddleware> app;

		// --- Repositories ---
		GenericRepository<RoomCategory> categoryRepo("room_categories");
		RoomRepository roomRepo;
		UserRepository userRepo;
		GenericRepository<Service> serviceRepo("services");
		BookingRepository bookingRepo;
		GenericRepository<Amenity> amenityRepo("amenities");

		// --- Services ---
		BookingService bookingService(bookingRepo, roomRepo, serviceRepo, userRepo);

		// --- Controllers ---
		RoomCategoryController categoryController(app, categoryRepo);
		RoomController roomController(app, roomRepo);
		UserController userController(app, userRepo, bookingRepo);
		ServiceController serviceController(app, serviceRepo);
		BookingController bookingController(app, bookingService);
		AmenityController amenityController(app, amenityRepo);
		ReportController reportController(app);

		CROW_ROUTE(app, "/")
			([] {
			return "Hotel API is running! (R01_00_00)";
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
