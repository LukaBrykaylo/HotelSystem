#include "BookingRepository.h"
#include "../db/DatabaseManager.h"
#include <iostream>

std::vector<Booking> BookingRepository::getAllBookings() {
    std::vector<Booking> bookings;
    try {
        pqxx::connection& C = DatabaseManager::getInstance().getConnection();
        pqxx::nontransaction N(C);

        // Простий запит, не чіпає послуги
        pqxx::result R = N.exec("SELECT * FROM bookings");

        for (auto row : R) {
            Booking b;
            b.from_row(row);
            bookings.push_back(b);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "BookingRepository::getAllBookings Error: " << e.what() << std::endl;
    }
    return bookings;
}

std::unique_ptr<Booking> BookingRepository::getBookingById(int bookingId) {
    try {
        pqxx::connection& C = DatabaseManager::getInstance().getConnection();
        pqxx::nontransaction N(C);

        // --- Крок 1: Отримати основні дані про бронювання ---
        pqxx::result R_booking = N.exec(
            "SELECT * FROM bookings WHERE booking_id = " + N.quote(bookingId)
        );

        if (R_booking.empty()) {
            return nullptr; // Бронювання не знайдено
        }

        // Створюємо об'єкт Booking
        auto booking = std::make_unique<Booking>();
        booking->from_row(R_booking[0]);

        // --- Крок 2: Отримати всі пов'язані послуги ---
        // Це і є та логіка, заради якої ми це робимо!
        std::string sql_services =
            "SELECT s.* "
            "FROM services s "
            "JOIN booking_services bs ON s.service_id = bs.service_id "
            "WHERE bs.booking_id = " + N.quote(bookingId);

        pqxx::result R_services = N.exec(sql_services);

        // --- Крок 3: Додати послуги до об'єкта Booking ---
        for (auto row : R_services) {
            Service s;
            s.from_row(row);
            booking->services.push_back(s);
        }

        return booking;

    }
    catch (const std::exception& e) {
        std::cerr << "BookingRepository::getBookingById Error: " << e.what() << std::endl;
        return nullptr;
    }
}

std::unique_ptr<Booking> BookingRepository::createBooking(
    pqxx::work& T, // Важливо: Приймаємо транзакцію!
    int userId,
    int roomId,
    const std::string& checkIn,
    const std::string& checkOut,
    double totalCost
) {
    std::string sql =
        "INSERT INTO bookings (user_id, room_id, check_in_date, check_out_date, total_cost, status) "
        "VALUES (" +
        T.quote(userId) + ", " +
        T.quote(roomId) + ", " +
        T.quote(checkIn) + ", " +
        T.quote(checkOut) + ", " +
        T.quote(totalCost) + ", " +
        T.quote("confirmed") +
        ") "
        "RETURNING *";

    pqxx::result R = T.exec(sql);

    if (R.empty()) {
        return nullptr;
    }

    auto booking = std::make_unique<Booking>();
    booking->from_row(R[0]);
    return booking;
}