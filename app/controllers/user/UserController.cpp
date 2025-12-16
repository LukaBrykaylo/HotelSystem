#include "UserController.h"
#include "../../models/Guest.h"
#include "../../models/Administrator.h"
#include <vector>
#include <string>
#include <iostream>
#include <sstream> 
#include <optional> 

UserController::UserController(crow::App<CorsMiddleware>& app, UserRepository& userRepo, BookingRepository& bookingRepo)
	: m_app(app), m_userRepo(userRepo), m_bookingRepo(bookingRepo)
{
	registerRoutes();
}

void UserController::registerRoutes() {

	CROW_ROUTE(m_app, "/api/users/<int>").methods("GET"_method)
		([this](int userId) {

		std::unique_ptr<User> user = m_userRepo.getUserById(userId);

		if (!user) {
			return crow::response(404, "User not found");
		}

		crow::json::wvalue response;
		response["userId"] = user->getUserId();
		response["username"] = user->getUsername();
		response["email"] = user->getEmail();
		response["phoneNumber"] = user->getPhoneNumber();

		if (user->getRole() == User::Role::GUEST) {
			Guest* guest = static_cast<Guest*>(user.get());
			response["role"] = "guest";
			response["bonuses"] = guest->getBonuses();
			response["paymentInfo"] = guest->getPaymentInfo();

		}
		else if (user->getRole() == User::Role::ADMIN) {
			Administrator* admin = static_cast<Administrator*>(user.get());
			response["role"] = "admin";
			response["accessLevel"] = admin->getAccessLevel();
		}

		return crow::response(200, response);
			});

	CROW_ROUTE(m_app, "/api/register").methods("POST"_method)
		([this](const crow::request& req) {

		auto body = crow::json::load(req.body);
		if (!body || !body.has("username") || !body.has("email") || !body.has("password")) {
			return crow::response(400, "Missing fields");
		}

		std::string username = body["username"].s();
		std::string email = body["email"].s();
		std::string password = body["password"].s();

		if (username.empty() || email.empty() || password.empty()) {
			return crow::response(400, "Fields cannot be empty");
		}

		auto newUser = m_userRepo.createUser(username, email, password);

		if (!newUser) {
			return crow::response(409, "Username or email already exists");
		}

		crow::json::wvalue response;
		response["userId"] = newUser->getUserId();
		response["username"] = newUser->getUsername();
		response["email"] = newUser->getEmail();
		response["role"] = "guest";

		return crow::response(201, response);
			});

	CROW_ROUTE(m_app, "/api/login").methods("POST"_method)
		([this](const crow::request& req) {

		auto body = crow::json::load(req.body);
		if (!body || !body.has("email") || !body.has("password")) {
			return crow::response(400, "Missing fields");
		}

		std::string email = body["email"].s();
		std::string password = body["password"].s();

		auto user = m_userRepo.findUserByCredentials(email, password);

		if (!user) {
			return crow::response(401, "Invalid email or password");
		}

		crow::json::wvalue response;
		response["message"] = "Login successful";
		response["token"] = "DUMMY_TOKEN_USER_ID_" + std::to_string(user->getUserId());
		response["userId"] = user->getUserId();
		response["username"] = user->getUsername();
		response["email"] = user->getEmail();
		response["phoneNumber"] = user->getPhoneNumber();

		if (user->getRole() == User::Role::GUEST) {
			Guest* guest = static_cast<Guest*>(user.get());
			response["role"] = "guest";
			response["bonuses"] = guest->getBonuses();
			response["paymentInfo"] = guest->getPaymentInfo();

		}
		else if (user->getRole() == User::Role::ADMIN) {
			Administrator* admin = static_cast<Administrator*>(user.get());
			response["role"] = "admin";
			response["accessLevel"] = admin->getAccessLevel();
		}

		return crow::response(200, response);
			});

	CROW_ROUTE(m_app, "/api/users/<int>").methods("PUT"_method)
		([this](const crow::request& req, int userId) {

		auto body = crow::json::load(req.body);
		if (!body || !body.has("username") || !body.has("email")) {
			return crow::response(400, "Missing fields");
		}

		std::string username = body["username"].s();
		std::string email = body["email"].s();

		std::string phoneNumber = "";
		if (body.has("phoneNumber") && body["phoneNumber"].t() != crow::json::type::Null) {
			phoneNumber = body["phoneNumber"].s();
		}

		try {
			auto updatedUser = m_userRepo.updateUser(userId, username, email, phoneNumber);

			if (!updatedUser) {
				return crow::response(404, "User not found");
			}

			Guest* guest = static_cast<Guest*>(updatedUser.get());

			crow::json::wvalue response;
			response["userId"] = guest->getUserId();
			response["username"] = guest->getUsername();
			response["email"] = guest->getEmail();
			response["phoneNumber"] = guest->getPhoneNumber();
			response["bonuses"] = guest->getBonuses();
			response["paymentInfo"] = guest->getPaymentInfo();
			response["role"] = "guest";

			return crow::response(200, response);

		}
		catch (const pqxx::unique_violation& e) {
			return crow::response(409, "Username or email already exists");
		}
		catch (const std::exception& e) {
			return crow::response(500, "Internal server error");
		}
			});

	CROW_ROUTE(m_app, "/api/users/<int>/bookings")
		.methods("GET"_method)
		([this](int userId) {

		auto bookings = m_bookingRepo.getAllBookingsByUserId(userId);

		crow::json::wvalue response;
		response["bookings"] = crow::json::wvalue::list();

		int index = 0;
		for (const auto& b : bookings) {
			response["bookings"][index]["booking_id"] = b.booking_id;
			response["bookings"][index]["room_id"] = b.room_id;
			response["bookings"][index]["check_in_date"] = b.check_in_date;
			response["bookings"][index]["check_out_date"] = b.check_out_date;
			response["bookings"][index]["total_cost"] = b.total_cost;
			response["bookings"][index]["status"] = b.status;
			index++;
		}

		return crow::response(200, response);
			});

	CROW_ROUTE(m_app, "/api/users").methods("GET"_method)
		([this] {
		auto users = m_userRepo.getAllUsers();

		crow::json::wvalue response;
		response["users"] = crow::json::wvalue::list();

		int index = 0;
		for (const auto& user : users) {
			response["users"][index]["userId"] = user->getUserId();
			response["users"][index]["username"] = user->getUsername();
			response["users"][index]["email"] = user->getEmail();
			response["users"][index]["phoneNumber"] = user->getPhoneNumber();

			if (user->getRole() == User::Role::GUEST) {
				Guest* guest = static_cast<Guest*>(user.get());
				response["users"][index]["role"] = "guest";

				response["users"][index]["bonuses"] = guest->getBonuses();

			}
			else {
				response["users"][index]["role"] = "admin";
			}
			index++;
		}

		return crow::response(200, response);
			});
}
