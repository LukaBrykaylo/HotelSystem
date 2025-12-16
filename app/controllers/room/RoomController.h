#pragma once

#include "crow.h"
#include "../../models/Room.h"
#include "../../repositories/RoomRepository.h"
#include "../../middleware/CorsMiddleware.h" 

class RoomController {
public:
	RoomController(crow::App<CorsMiddleware>& app, RoomRepository& repo)
		: m_app(app), m_repo(repo)
	{
		registerRoutes();
	}

private:
	void registerRoutes();
	crow::App<CorsMiddleware>& m_app;
	RoomRepository& m_repo;
};
