#pragma once

#include "../models/Room.h"
#include <vector>
#include <memory>
#include <string>
#include <optional> 

class RoomRepository {
public:
	RoomRepository() = default;

	std::vector<Room> getAllRooms(
		const std::vector<int>& amenityIds,
		std::optional<double> minPrice,
		std::optional<double> maxPrice,
		std::optional<int> categoryId,
		std::optional<int> capacity
	);

	std::unique_ptr<Room> createRoom(
		int categoryId,
		const std::string& roomNumber,
		int capacity
	);

	std::unique_ptr<Room> updateRoom(
		int roomId,
		int categoryId,
		const std::string& roomNumber,
		int capacity
	);

	bool deleteRoom(int roomId);
};
