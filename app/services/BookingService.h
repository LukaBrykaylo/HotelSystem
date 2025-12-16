#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../models/Booking.h"
#include "../repositories/BookingRepository.h"
#include "../repositories/RoomRepository.h" 
#include "../repositories/GenericRepository.h"
#include "../repositories/UserRepository.h" 
#include "../models/Room.h"
#include "../models/Service.h"

class BookingException : public std::runtime_error {
public:
	BookingException(const std::string& msg) : std::runtime_error(msg) {}
};

class BookingService {
public:
	BookingService(
		BookingRepository& bookingRepo,
		RoomRepository& roomRepo,
		GenericRepository<Service>& serviceRepo,
		UserRepository& userRepo
	);

	std::unique_ptr<Booking> createNewBooking(
		int userId, int roomId, const std::string& checkIn,
		const std::string& checkOut, const std::vector<int>& serviceIds,
		bool useBonuses
	);

	std::unique_ptr<Booking> getBookingDetails(int bookingId);
	bool deleteBooking(int bookingId);
	std::vector<Booking> getAllBookings();
	pqxx::result getAllBookingDetails();

	std::unique_ptr<Booking> adminCreateBooking(
		int userId, int roomId, const std::string& checkIn,
		const std::string& checkOut, const std::string& status
	);

	std::unique_ptr<Booking> adminUpdateBooking(
		int bookingId, int userId, int roomId, const std::string& checkIn,
		const std::string& checkOut, const std::string& status
	);

	void cancelBookingByUser(int bookingId, int userId);
	void extendBooking(int bookingId, int userId, const std::string& newCheckOut);

private:
	bool isRoomAvailable(pqxx::transaction_base& T, int roomId, const std::string& checkIn, const std::string& checkOut);
	double calculateTotalCost(pqxx::transaction_base& T, int roomId, const std::string& checkIn, const std::string& checkOut, const std::vector<int>& serviceIds);
	int getDaysBetween(const std::string& date1, const std::string& date2);
	void addServicesToBooking(pqxx::work& T, int bookingId, const std::vector<int>& serviceIds);

	BookingRepository& m_bookingRepo;
	RoomRepository& m_roomRepo;
	GenericRepository<Service>& m_serviceRepo;
	UserRepository& m_userRepo;
};
