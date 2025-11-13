#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../models/Booking.h"
#include "../repositories/BookingRepository.h"
#include "../repositories/GenericRepository.h"
#include "../models/Room.h"
#include "../models/Service.h"

// Клас-виняток для нашої бізнес-логіки
class BookingException : public std::runtime_error {
public:
    BookingException(const std::string& msg) : std::runtime_error(msg) {}
};

class BookingService {
public:
    // Впровадження всіх залежностей, які нам потрібні
    BookingService(
        BookingRepository& bookingRepo,
        GenericRepository<Room>& roomRepo,
        GenericRepository<Service>& serviceRepo
    );

    // Головний "мозковий" метод
    std::unique_ptr<Booking> createNewBooking(
        int userId,
        int roomId,
        const std::string& checkIn,
        const std::string& checkOut,
        const std::vector<int>& serviceIds
    );

    std::unique_ptr<Booking> getBookingDetails(int bookingId);

private:
    // Внутрішні методи для логіки
    bool isRoomAvailable(pqxx::transaction_base& T, int roomId, const std::string& checkIn, const std::string& checkOut);
    double calculateTotalCost(pqxx::transaction_base& T, int roomId, const std::string& checkIn, const std::string& checkOut, const std::vector<int>& serviceIds);
    void addServicesToBooking(pqxx::work& T, int bookingId, const std::vector<int>& serviceIds);

    // Зберігаємо посилання на репозиторії
    BookingRepository& m_bookingRepo;
    GenericRepository<Room>& m_roomRepo;
    GenericRepository<Service>& m_serviceRepo;
};