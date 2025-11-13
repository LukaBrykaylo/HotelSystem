#include "BookingService.h"
#include "../db/DatabaseManager.h"
#include <iostream>

BookingService::BookingService(
    BookingRepository& bookingRepo,
    GenericRepository<Room>& roomRepo,
    GenericRepository<Service>& serviceRepo
) : m_bookingRepo(bookingRepo), m_roomRepo(roomRepo), m_serviceRepo(serviceRepo) {
}

// --- Головний публічний метод ---
std::unique_ptr<Booking> BookingService::createNewBooking(
    int userId, int roomId, const std::string& checkIn, const std::string& checkOut, const std::vector<int>& serviceIds
) {
    // Ми загортаємо ВСЕ в одну транзакцію.
    // Якщо щось піде не так (напр., кімната зайнята), T.abort() скасує ВСІ зміни.
    pqxx::connection& C = DatabaseManager::getInstance().getConnection();
    pqxx::work T(C); // 'T' - це наша транзакція

    try {
        // --- Крок 1: Перевірка доступності ---
        if (!isRoomAvailable(T, roomId, checkIn, checkOut)) {
            throw BookingException("Room is not available for the selected dates");
        }

        // --- Крок 2: Розрахунок вартості ---
        double totalCost = calculateTotalCost(T, roomId, checkIn, checkOut, serviceIds);

        // --- Крок 3: Створення бронювання ---
        auto booking = m_bookingRepo.createBooking(T, userId, roomId, checkIn, checkOut, totalCost);
        if (!booking) {
            throw BookingException("Failed to create booking record");
        }

        // --- Крок 4: Додавання послуг до бронювання ---
        addServicesToBooking(T, booking->booking_id, serviceIds);

        // --- Крок 5: Успіх! Зберігаємо всі зміни ---
        T.commit();

        // Повертаємо створене бронювання
        return booking;

    }
    catch (const std::exception& e) {
        T.abort(); // !! ВАЖЛИВО: Скасовуємо всі зміни !!
        std::cerr << "BookingService Error: " << e.what() << std::endl;
        throw; // Кидаємо помилку далі, щоб контролер її спіймав
    }
}

std::unique_ptr<Booking> BookingService::getBookingDetails(int bookingId) {
    // Наразі логіка проста, тому просто викликаємо репозиторій.
    // У майбутньому тут можна додати перевірку прав доступу.
    return m_bookingRepo.getBookingById(bookingId);
}

// --- Приватні допоміжні методи ---

bool BookingService::isRoomAvailable(
    pqxx::transaction_base& T, int roomId, const std::string& checkIn, const std::string& checkOut
) {
    // Цей SQL-запит шукає БУДЬ-ЯКЕ бронювання, яке "перетинається" з нашими датами
    // (A, B) - нове бронювання; [X, Y] - існуюче бронювання
    // Логіка: (A < Y) AND (B > X)
    std::string sql =
        "SELECT COUNT(*) FROM bookings "
        "WHERE room_id = " + T.quote(roomId) + " "
        "AND status != " + T.quote("cancelled") + " "
        "AND check_in_date < " + T.quote(checkOut) + " "
        "AND check_out_date > " + T.quote(checkIn);

    pqxx::result R = T.exec(sql);
    int overlapCount = R[0][0].as<int>();

    return overlapCount == 0; // Доступна, якщо 0 перетинів
}

double BookingService::calculateTotalCost(
    pqxx::transaction_base& T, int roomId, const std::string& checkIn, const std::string& checkOut, const std::vector<int>& serviceIds
) {
    double roomCost = 0;

    // 1. Отримуємо ціну кімнати
    // (Це не Generic, тому робимо простий запит)
    std::string sql_room = "SELECT base_price FROM rooms "
        "JOIN room_categories ON rooms.category_id = room_categories.category_id "
        "WHERE rooms.room_id = " + T.quote(roomId);

    pqxx::result R_room = T.exec(sql_room);
    if (R_room.empty()) throw BookingException("Room not found");

    // TODO: Розрахувати кількість днів (це складно, поки що беремо 1)
    int numberOfDays = 1; // TODO: (std::date(checkOut) - std::date(checkIn)).days()
    roomCost = R_room[0][0].as<double>() * numberOfDays;

    // 2. Отримуємо ціну послуг
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
    if (serviceIds.empty()) {
        return; // Нема чого додавати
    }

    std::string sql = "INSERT INTO booking_services (booking_id, service_id) VALUES ";
    for (size_t i = 0; i < serviceIds.size(); ++i) {
        sql += "(" + T.quote(bookingId) + ", " + T.quote(serviceIds[i]) + ")";
        if (i < serviceIds.size() - 1) sql += ",";
    }

    T.exec(sql);
}