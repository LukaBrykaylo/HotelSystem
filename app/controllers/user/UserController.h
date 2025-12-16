#pragma once

#include "crow.h"
#include "../../repositories/UserRepository.h"
#include "../../repositories/BookingRepository.h" 
#include "../../middleware/CorsMiddleware.h"

class UserController {
public:
	UserController(crow::App<CorsMiddleware>& app, UserRepository& userRepo, BookingRepository& bookingRepo);

private:
	void registerRoutes();
	crow::App<CorsMiddleware>& m_app;
	UserRepository& m_userRepo;
	BookingRepository& m_bookingRepo;
};
