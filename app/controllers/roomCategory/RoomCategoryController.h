#pragma once

#include "crow.h"
#include "../../models/RoomCategory.h" 
#include "../../repositories/GenericRepository.h"
#include "../../middleware/CorsMiddleware.h"

class RoomCategoryController {
public:
	RoomCategoryController(
		crow::App<CorsMiddleware>& app,
		GenericRepository<RoomCategory>& repo
	)
		: m_app(app), m_repo(repo)
	{
		registerRoutes();
	}

private:
	void registerRoutes();

	crow::App<CorsMiddleware>& m_app;

	GenericRepository<RoomCategory>& m_repo;
};
