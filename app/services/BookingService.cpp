#include "BookingService.h"
#include "../db/DatabaseManager.h"
#include <iostream>
#include <sstream> 
#include <iomanip> 
#include <chrono>  

BookingService::BookingService(
	BookingRepository& bookingRepo,
	RoomRepository& roomRepo,
	GenericRepository<Service>& serviceRepo,
	UserRepository& userRepo
) : m_bookingRepo(bookingRepo), m_roomRepo(roomRepo), m_serviceRepo(serviceRepo), m_userRepo(userRepo) {
}

std::unique_ptr<Booking> BookingService::createNewBooking(
	int userId, int roomId, const std::string& checkIn, const std::string& checkOut,
	const std::vector<int>& serviceIds, bool useBonuses
) {
	pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
	pqxx::work T(C);
	try {
		if (!isRoomAvailable(T, roomId, checkIn, checkOut)) {
			throw BookingException("Room is not available for the selected dates");
		}

		double totalCost = calculateTotalCost(T, roomId, checkIn, checkOut, serviceIds);

		if (useBonuses) {
			auto user = m_userRepo.getUserById(userId);
			if (user) {
				pqxx::result R = T.exec("SELECT bonuses FROM users WHERE user_id = " + T.quote(userId));
				if (!R.empty() && !R[0][0].is_null()) {
					int currentBonuses = R[0][0].as<int>();
					if (currentBonuses > 0) {
						double discount = static_cast<double>(currentBonuses);
						if (discount > totalCost) discount = totalCost;
						totalCost -= discount;
						m_userRepo.addBonuses(T, userId, -static_cast<int>(discount));
					}
				}
			}
		}

		auto booking = m_bookingRepo.createBooking(T, userId, roomId, checkIn, checkOut, totalCost);
		if (!booking) throw BookingException("Failed to create booking record");

		addServicesToBooking(T, booking->booking_id, serviceIds);
		T.commit();
		return booking;
	}
	catch (const std::exception& e) {
		T.abort();
		std::cerr << "BookingService Error: " << e.what() << std::endl;
		throw;
	}
}

std::unique_ptr<Booking> BookingService::getBookingDetails(int bookingId) {
	return m_bookingRepo.getBookingById(bookingId);
}
bool BookingService::deleteBooking(int bookingId) {
	return m_bookingRepo.deleteBooking(bookingId);
}
std::vector<Booking> BookingService::getAllBookings() {
	return m_bookingRepo.getAllBookings();
}
pqxx::result BookingService::getAllBookingDetails() {
	return m_bookingRepo.getAllBookingDetails();
}

std::unique_ptr<Booking> BookingService::adminCreateBooking(
	int userId, int roomId, const std::string& checkIn,
	const std::string& checkOut, const std::string& status
) {
	pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
	pqxx::work T(C);

	try {
		std::vector<int> emptyServices;
		double totalCost = calculateTotalCost(T, roomId, checkIn, checkOut, emptyServices);
		auto booking = m_bookingRepo.adminCreateBooking(T, userId, roomId, checkIn, checkOut, totalCost, status);
		T.commit();
		return booking;
	}
	catch (const std::exception& e) {
		T.abort();
		std::cerr << "BookingService::adminCreateBooking Error: " << e.what() << std::endl;
		throw;
	}
}

std::unique_ptr<Booking> BookingService::adminUpdateBooking(
	int bookingId, int userId, int roomId, const std::string& checkIn,
	const std::string& checkOut, const std::string& newStatus
) {
	pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
	pqxx::work T(C);

	try {
		auto oldBooking = m_bookingRepo.getBookingById(bookingId);
		if (!oldBooking) throw BookingException("Booking not found");
		std::string oldStatus = oldBooking->status;

		std::vector<int> emptyServices;
		double totalCost = calculateTotalCost(T, roomId, checkIn, checkOut, emptyServices);

		auto booking = m_bookingRepo.adminUpdateBooking(T, bookingId, userId, roomId, checkIn, checkOut, totalCost, newStatus);

		if (newStatus == "completed" && oldStatus != "completed") {
			int bonusGained = static_cast<int>(totalCost * 0.10);
			m_userRepo.addBonuses(T, userId, bonusGained);
		}

		T.commit();
		return booking;
	}
	catch (const std::exception& e) {
		T.abort();
		std::cerr << "BookingService::adminUpdateBooking Error: " << e.what() << std::endl;
		throw;
	}
}

void BookingService::cancelBookingByUser(int bookingId, int userId) {
	pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
	pqxx::work T(C);

	try {
		auto booking = m_bookingRepo.getBookingById(bookingId);
		if (!booking) throw BookingException("Booking not found");

		if (booking->user_id != userId) throw BookingException("Unauthorized access");

		m_bookingRepo.updateStatus(T, bookingId, "cancelled");
		T.commit();
	}
	catch (const std::exception& e) {
		T.abort();
		throw;
	}
}

void BookingService::extendBooking(int bookingId, int userId, const std::string& newCheckOut) {
	pqxx::connection C(DatabaseManager::getInstance().getConnectionString());
	pqxx::work T(C);

	try {
		auto booking = m_bookingRepo.getBookingById(bookingId);
		if (!booking) throw BookingException("Booking not found");
		if (booking->user_id != userId) throw BookingException("Unauthorized access");

		std::vector<int> serviceIds;
		for (const auto& s : booking->services) serviceIds.push_back(s.service_id);

		double newTotalCost = calculateTotalCost(T, booking->room_id, booking->check_in_date, newCheckOut, serviceIds);

		m_bookingRepo.updateDateAndCost(T, bookingId, newCheckOut, newTotalCost);
		T.commit();

	}
	catch (const std::exception& e) {
		T.abort();
		throw;
	}
}

int BookingService::getDaysBetween(const std::string& date1_str, const std::string& date2_str) {
	std::tm tm1 = {}; std::tm tm2 = {};
	std::stringstream ss1(date1_str); std::stringstream ss2(date2_str);
	ss1 >> std::get_time(&tm1, "%Y-%m-%d"); ss2 >> std::get_time(&tm2, "%Y-%m-%d");
	auto tp1 = std::chrono::system_clock::from_time_t(std::mktime(&tm1));
	auto tp2 = std::chrono::system_clock::from_time_t(std::mktime(&tm2));
	auto duration = std::chrono::duration_cast<std::chrono::hours>(tp2 - tp1);
	return duration.count() / 24;
}

double BookingService::calculateTotalCost(
	pqxx::transaction_base& T, int roomId, const std::string& checkIn, const std::string& checkOut, const std::vector<int>& serviceIds
) {
	double roomCost = 0;
	std::string sql_room = "SELECT base_price FROM rooms JOIN room_categories ON rooms.category_id = room_categories.category_id WHERE rooms.room_id = " + T.quote(roomId);
	pqxx::result R_room = T.exec(sql_room);
	if (R_room.empty()) throw BookingException("Room not found");

	int numberOfDays = getDaysBetween(checkIn, checkOut);
	if (numberOfDays <= 0) numberOfDays = 1;

	roomCost = R_room[0][0].as<double>() * numberOfDays;

	double servicesCost = 0;
	if (!serviceIds.empty()) {
		std::string serviceList;
		for (size_t i = 0; i < serviceIds.size(); ++i) {
			serviceList += T.quote(serviceIds[i]);
			if (i < serviceIds.size() - 1) serviceList += ",";
		}
		std::string sql_services = "SELECT SUM(price) FROM services WHERE service_id IN (" + serviceList + ")";
		pqxx::result R_services = T.exec(sql_services);
		if (!R_services.empty() && !R_services[0][0].is_null()) {
			servicesCost = R_services[0][0].as<double>();
		}
	}
	return roomCost + servicesCost;
}

void BookingService::addServicesToBooking(
	pqxx::work& T, int bookingId, const std::vector<int>& serviceIds
) {
	if (serviceIds.empty()) return;
	std::string sql = "INSERT INTO booking_services (booking_id, service_id) VALUES ";
	for (size_t i = 0; i < serviceIds.size(); ++i) {
		sql += "(" + T.quote(bookingId) + ", " + std::to_string(serviceIds[i]) + ")";
		if (i < serviceIds.size() - 1) sql += ",";
	}
	T.exec(sql);
}
bool BookingService::isRoomAvailable(
	pqxx::transaction_base& T, int roomId, const std::string& checkIn, const std::string& checkOut
) {
	std::string sql = "SELECT COUNT(*) FROM bookings WHERE room_id = " + T.quote(roomId) + " AND status != " + T.quote("cancelled") + " AND check_in_date < " + T.quote(checkOut) + " AND check_out_date > " + T.quote(checkIn);
	pqxx::result R = T.exec(sql);
	int overlapCount = R[0][0].as<int>();
	return overlapCount == 0;
}
