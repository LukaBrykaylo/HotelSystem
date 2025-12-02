#include "RoomController.h"
#include <vector>
#include <string>
#include <iostream>
#include <sstream> 
#include <optional> 

void RoomController::registerRoutes() {

	CROW_ROUTE(m_app, "/api/rooms").methods("GET"_method)
		([this](const crow::request& req) {

		std::vector<int> amenityIds;
		std::optional<double> minPrice;
		std::optional<double> maxPrice;
		std::optional<int> categoryId;
		std::optional<int> capacity;

		std::string url = req.raw_url;
		std::string queryString = "";

		size_t q_pos = url.find('?');
		if (q_pos != std::string::npos) {
			queryString = url.substr(q_pos + 1);
		}

		std::stringstream ss(queryString);
		std::string segment;

		while (std::getline(ss, segment, '&'))
		{
			std::string key;
			std::string value;
			size_t eqPos = segment.find('=');

			if (eqPos != std::string::npos) {
				key = segment.substr(0, eqPos);
				value = segment.substr(eqPos + 1);

				if (value.empty()) continue;

				if (key == "amenity") {
					try {
						amenityIds.push_back(std::stoi(value));
					}
					catch (...) {}
				}
				else if (key == "min_price") {
					try {
						minPrice = std::stod(value);
					}
					catch (...) {}
				}
				else if (key == "max_price") {
					try {
						maxPrice = std::stod(value);
					}
					catch (...) {}
				}
				else if (key == "category_id") {
					try {
						categoryId = std::stoi(value);
					}
					catch (...) {}
				}
				else if (key == "capacity") {
					try {
						capacity = std::stoi(value);
					}
					catch (...) {}
				}
			}
		}

		std::vector<Room> rooms = m_repo.getAllRooms(amenityIds, minPrice, maxPrice, categoryId, capacity);

		crow::json::wvalue response;
		response["rooms"] = crow::json::wvalue::list();

		int index = 0;
		for (const auto& room : rooms) {
			response["rooms"][index]["room_id"] = room.room_id;
			response["rooms"][index]["category_id"] = room.category_id;
			response["rooms"][index]["room_number"] = room.room_number;
			response["rooms"][index]["capacity"] = room.capacity;
			response["rooms"][index]["comments"] = room.comments;
			response["rooms"][index]["amenities"] = crow::json::wvalue::list();
			int a_index = 0;
			for (const auto& amenityName : room.amenities) {
				response["rooms"][index]["amenities"][a_index] = amenityName;
				a_index++;
			}

			index++;
		}

		return crow::response(200, response);
			});

	CROW_ROUTE(m_app, "/api/rooms").methods("POST"_method)
		([this](const crow::request& req) {
		auto body = crow::json::load(req.body);
		if (!body || !body.has("category_id") || !body.has("room_number") || !body.has("capacity")) {
			return crow::response(400, "Missing required fields");
		}
		try {
			auto newRoom = m_repo.createRoom(
				body["category_id"].i(),
				body["room_number"].s(),
				body["capacity"].i()
			);
			if (!newRoom) return crow::response(500, "Failed to create room");

			crow::json::wvalue response;
			response["room_id"] = newRoom->room_id;
			response["category_id"] = newRoom->category_id;
			response["room_number"] = newRoom->room_number;
			response["capacity"] = newRoom->capacity;
			return crow::response(201, response);
		}
		catch (const std::exception& e) { return crow::response(409, e.what()); }
			});

	CROW_ROUTE(m_app, "/api/rooms/<int>").methods("PUT"_method)
		([this](const crow::request& req, int roomId) {
		auto body = crow::json::load(req.body);
		if (!body || !body.has("category_id") || !body.has("room_number") || !body.has("capacity")) {
			return crow::response(400, "Missing required fields");
		}
		try {
			auto updatedRoom = m_repo.updateRoom(
				roomId,
				body["category_id"].i(),
				body["room_number"].s(),
				body["capacity"].i()
			);
			if (!updatedRoom) return crow::response(404, "Room not found");

			crow::json::wvalue response;
			response["room_id"] = updatedRoom->room_id;
			response["category_id"] = updatedRoom->category_id;
			response["room_number"] = updatedRoom->room_number;
			response["capacity"] = updatedRoom->capacity;
			return crow::response(200, response);
		}
		catch (const std::exception& e) { return crow::response(409, e.what()); }
			});

	CROW_ROUTE(m_app, "/api/rooms/<int>").methods("DELETE"_method)
		([this](int roomId) {
		try {
			if (m_repo.deleteRoom(roomId)) {
				return crow::response(204);
			}
			else {
				return crow::response(404, "Room not found");
			}
		}
		catch (const std::exception& e) { return crow::response(409, e.what()); }
			});
}
