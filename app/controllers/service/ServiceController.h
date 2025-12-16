#pragma once

#include "crow.h"
#include "../../models/Service.h"
#include "../../repositories/GenericRepository.h"
#include "../../middleware/CorsMiddleware.h"

class ServiceController {
public:
	ServiceController(crow::App<CorsMiddleware>& app, GenericRepository<Service>& repo)
		: m_app(app), m_repo(repo)
	{
		registerRoutes();
	}

private:
	void registerRoutes();
	crow::App<CorsMiddleware>& m_app;
	GenericRepository<Service>& m_repo;
};
