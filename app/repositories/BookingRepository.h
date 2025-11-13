#pragma once
#include "../models/Booking.h"
#include <vector>
#include <memory>

class BookingRepository {
public:
    BookingRepository() = default;

    // Отримує бронювання разом з усіма його послугами
    std::unique_ptr<Booking> getBookingById(int bookingId);

    std::unique_ptr<Booking> createBooking(
        pqxx::work& T, // Приймає транзакцію, щоб не комітити зарано
        int userId,
        int roomId,
        const std::string& checkIn,
        const std::string& checkOut,
        double totalCost
    );

    // Отримує всі бронювання (але БЕЗ послуг, для економії)
    std::vector<Booking> getAllBookings();

    // ... Тут згодом буде createBooking, updateBooking...
};