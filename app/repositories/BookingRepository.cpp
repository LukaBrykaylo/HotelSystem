#include "BookingRepository.h"
#include "../db/DatabaseManager.h"
#include <iostream>
#include <string>

std::vector<Booking> BookingRepository::getAllBookings() {
	std::vector<Booking> bookings;
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::nontransaction N(C);
		pqxx::result R = N.exec("SELECT * FROM bookings");
		for (auto row : R) {
			Booking b; b.from_row(row); bookings.push_back(b);
		}
	}
	catch (const std::exception& e) {
		std::cerr << "BookingRepository::getAllBookings Error: " << e.what() << std::endl;
	}
	return bookings;
}
std::unique_ptr<Booking> BookingRepository::getBookingById(int bookingId) {
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::nontransaction N(C);
		pqxx::result R_booking = N.exec("SELECT * FROM bookings WHERE booking_id = " + N.quote(bookingId));
		if (R_booking.empty()) return nullptr;
		auto booking = std::make_unique<Booking>();
		booking->from_row(R_booking[0]);
		std::string sql_services = "SELECT s.* FROM services s JOIN booking_services bs ON s.service_id = bs.service_id WHERE bs.booking_id = " + N.quote(bookingId);
		pqxx::result R_services = N.exec(sql_services);
		for (auto row : R_services) {
			Service s; s.from_row(row); booking->services.push_back(s);
		}
		return booking;
	}
	catch (const std::exception& e) {
		std::cerr << "BookingRepository::getBookingById Error: " << e.what() << std::endl;
		return nullptr;
	}
}
std::unique_ptr<Booking> BookingRepository::createBooking(pqxx::work& T, int userId, int roomId, const std::string& checkIn, const std::string& checkOut, double totalCost) {
	std::string sql = "INSERT INTO bookings (user_id, room_id, check_in_date, check_out_date, total_cost, status) VALUES (" +
		std::to_string(userId) + ", " +
		std::to_string(roomId) + ", " +
		T.quote(checkIn) + ", " +
		T.quote(checkOut) + ", " +
		std::to_string(totalCost) + ", " +
		T.quote("confirmed") +
		") RETURNING *";

	pqxx::result R = T.exec(sql);
	if (R.empty()) return nullptr;
	auto booking = std::make_unique<Booking>();
	booking->from_row(R[0]);
	return booking;
}
std::vector<Booking> BookingRepository::getAllBookingsByUserId(int userId) {
	std::vector<Booking> bookings;
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::nontransaction N(C);
		std::string sql = "SELECT * FROM bookings WHERE user_id = " + N.quote(userId) + " ORDER BY created_at DESC";
		pqxx::result R = N.exec(sql);
		for (auto row : R) {
			Booking b; b.from_row(row); bookings.push_back(b);
		}
	}
	catch (const std::exception& e) {
		std::cerr << "BookingRepository::getAllBookingsByUserId Error: " << e.what() << std::endl;
	}
	return bookings;
}
bool BookingRepository::deleteBooking(int bookingId) {
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::work T(C);
		std::string sql = "DELETE FROM bookings WHERE booking_id = " + std::to_string(bookingId);
		pqxx::result R = T.exec(sql);
		T.commit();
		return R.affected_rows() > 0;
	}
	catch (const std::exception& e) {
		std::cerr << "BookingRepository::deleteBooking Error: " << e.what() << std::endl;
		throw;
	}
}

pqxx::result BookingRepository::getAllBookingDetails() {
	try {
		pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
		pqxx::nontransaction N(C);
		std::string sql =
			"SELECT b.*, u.username, r.room_number "
			"FROM bookings b "
			"LEFT JOIN users u ON b.user_id = u.user_id "
			"LEFT JOIN rooms r ON b.room_id = r.room_id "
			"ORDER BY b.booking_id DESC";

		return N.exec(sql);
	}
	catch (const std::exception& e) {
		std::cerr << "BookingRepository::getAllBookingDetails Error: " << e.what() << std::endl;
		throw;
	}
}

std::unique_ptr<Booking> BookingRepository::adminCreateBooking(
	pqxx::work& T,
	int userId, int roomId, const std::string& checkIn,
	const std::string& checkOut, double totalCost, const std::string& status
) {
	std::string sql = "INSERT INTO bookings (user_id, room_id, check_in_date, check_out_date, total_cost, status) VALUES (" +
		std::to_string(userId) + ", " +
		std::to_string(roomId) + ", " +
		T.quote(checkIn) + ", " +
		T.quote(checkOut) + ", " +
		std::to_string(totalCost) + ", " +
		T.quote(status) +
		") RETURNING *";

	pqxx::result R = T.exec(sql);
	if (R.empty()) return nullptr;
	auto booking = std::make_unique<Booking>();
	booking->from_row(R[0]);
	return booking;
}

std::unique_ptr<Booking> BookingRepository::adminUpdateBooking(
	pqxx::work& T,
	int bookingId, int userId, int roomId, const std::string& checkIn,
	const std::string& checkOut, double totalCost, const std::string& status
) {
	std::string sql = "UPDATE bookings SET "
		"user_id = " + std::to_string(userId) + ", "
		"room_id = " + std::to_string(roomId) + ", "
		"check_in_date = " + T.quote(checkIn) + ", "
		"check_out_date = " + T.quote(checkOut) + ", "
		"total_cost = " + std::to_string(totalCost) + ", "
		"status = " + T.quote(status) + " "
		"WHERE booking_id = " + std::to_string(bookingId) + " "
		"RETURNING *";

	pqxx::result R = T.exec(sql);
	if (R.empty()) return nullptr;
	auto booking = std::make_unique<Booking>();
	booking->from_row(R[0]);
	return booking;
}

void BookingRepository::updateStatus(pqxx::work& T, int bookingId, const std::string& status) {
	std::string sql = "UPDATE bookings SET status = " + T.quote(status) +
		" WHERE booking_id = " + std::to_string(bookingId);
	T.exec(sql);
}

void BookingRepository::updateDateAndCost(pqxx::work& T, int bookingId, const std::string& newCheckOut, double newTotalCost) {
	std::string sql = "UPDATE bookings SET check_out_date = " + T.quote(newCheckOut) +
		", total_cost = " + std::to_string(newTotalCost) +
		" WHERE booking_id = " + std::to_string(bookingId);
	T.exec(sql);
}
