#pragma once

#include "../models/Booking.h"
#include <vector>
#include <memory>
#include <pqxx/pqxx>

class BookingRepository {
public:
	BookingRepository() = default;

	std::unique_ptr<Booking> getBookingById(int bookingId);
	std::vector<Booking> getAllBookings();
	std::vector<Booking> getAllBookingsByUserId(int userId);

	pqxx::result getAllBookingDetails();

	std::unique_ptr<Booking> createBooking(
		pqxx::work& T, int userId, int roomId,
		const std::string& checkIn, const std::string& checkOut, double totalCost
	);

	std::unique_ptr<Booking> adminCreateBooking(
		pqxx::work& T,
		int userId, int roomId, const std::string& checkIn,
		const std::string& checkOut, double totalCost, const std::string& status
	);

	std::unique_ptr<Booking> adminUpdateBooking(
		pqxx::work& T,
		int bookingId, int userId, int roomId, const std::string& checkIn,
		const std::string& checkOut, double totalCost, const std::string& status
	);

	bool deleteBooking(int bookingId);

	void updateStatus(pqxx::work& T, int bookingId, const std::string& status);
	void updateDateAndCost(pqxx::work& T, int bookingId, const std::string& newCheckOut, double newTotalCost);
};
