#include "RoomRepository.h"
#include "../db/DatabaseManager.h"
#include <iostream>
#include <string>
#include <map>

std::vector<Room> RoomRepository::getAllRooms(
	const std::vector<int>& amenityIds,
	std::optional<double> minPrice,
	std::optional<double> maxPrice,
	std::optional<int> categoryId,
	std::optional<int> capacity
) {
	std::vector<Room> rooms;
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::nontransaction N(C);

		std::string sql = "SELECT r.* FROM rooms r "
			"JOIN room_categories rc ON r.category_id = rc.category_id";

		std::string whereClause = " WHERE 1=1 ";

		if (minPrice.has_value()) {
			whereClause += " AND rc.base_price >= " + std::to_string(minPrice.value());
		}
		if (maxPrice.has_value()) {
			whereClause += " AND rc.base_price <= " + std::to_string(maxPrice.value());
		}
		if (categoryId.has_value()) {
			whereClause += " AND r.category_id = " + std::to_string(categoryId.value());
		}
		if (capacity.has_value()) {
			whereClause += " AND r.capacity >= " + std::to_string(capacity.value());
		}
		if (!amenityIds.empty()) {
			std::string idList;
			for (size_t i = 0; i < amenityIds.size(); ++i) {
				idList += std::to_string(amenityIds[i]);
				if (i < amenityIds.size() - 1) idList += ",";
			}

			whereClause += " AND r.room_id IN ("
				"  SELECT room_id FROM room_amenities "
				"  WHERE amenity_id IN (" + idList + ") "
				"  GROUP BY room_id "
				"  HAVING COUNT(DISTINCT amenity_id) = " + std::to_string(amenityIds.size()) +
				")";
		}

		sql += whereClause + " ORDER BY r.room_id";

		pqxx::result R = N.exec(sql);

		for (auto row : R) {
			Room r;
			r.from_row(row);
			rooms.push_back(r);
		}

		if (!rooms.empty()) {
			std::string roomIdsList;
			for (size_t i = 0; i < rooms.size(); ++i) {
				roomIdsList += std::to_string(rooms[i].room_id);
				if (i < rooms.size() - 1) roomIdsList += ",";
			}

			std::string amenSql =
				"SELECT ra.room_id, a.name "
				"FROM room_amenities ra "
				"JOIN amenities a ON ra.amenity_id = a.amenity_id "
				"WHERE ra.room_id IN (" + roomIdsList + ")";

			pqxx::result R_amen = N.exec(amenSql);

			std::map<int, std::vector<std::string>> amenitiesMap;
			for (auto row : R_amen) {
				int rId = row["room_id"].as<int>();
				std::string aName = row["name"].as<std::string>();
				amenitiesMap[rId].push_back(aName);
			}

			for (auto& room : rooms) {
				room.amenities = amenitiesMap[room.room_id];
			}
		}
		return rooms;
	}
	catch (const std::exception& e) {
		std::cerr << "RoomRepository::getAllRooms Error: " << e.what() << std::endl;
		return rooms;
	}
}

std::unique_ptr<Room> RoomRepository::createRoom(int categoryId, const std::string& roomNumber, int capacity) {
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::work T(C);
		std::string sql = "INSERT INTO rooms (category_id, room_number, capacity) VALUES (" +
			std::to_string(categoryId) + ", " + T.quote(roomNumber) + ", " + std::to_string(capacity) + ") RETURNING *";
		pqxx::result R = T.exec(sql);
		T.commit();
		if (R.empty()) return nullptr;
		auto room = std::make_unique<Room>();
		room->from_row(R[0]);
		return room;
	}
	catch (const std::exception& e) { std::cerr << "RoomRepository::createRoom Error: " << e.what() << std::endl; throw; }
}
std::unique_ptr<Room> RoomRepository::updateRoom(int roomId, int categoryId, const std::string& roomNumber, int capacity) {
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::work T(C);
		std::string sql = "UPDATE rooms SET category_id = " + std::to_string(categoryId) + ", room_number = " + T.quote(roomNumber) + ", capacity = " + std::to_string(capacity) + " WHERE room_id = " + std::to_string(roomId) + " RETURNING *";
		pqxx::result R = T.exec(sql);
		T.commit();
		if (R.empty()) return nullptr;
		auto room = std::make_unique<Room>();
		room->from_row(R[0]);
		return room;
	}
	catch (const std::exception& e) { std::cerr << "RoomRepository::updateRoom Error: " << e.what() << std::endl; throw; }
}
bool RoomRepository::deleteRoom(int roomId) {
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::work T(C);
		std::string sql = "DELETE FROM rooms WHERE room_id = " + std::to_string(roomId);
		pqxx::result R = T.exec(sql);
		T.commit();
		return R.affected_rows() > 0;
	}
	catch (const std::exception& e) { std::cerr << "RoomRepository::deleteRoom Error: " << e.what() << std::endl; throw; }
}
