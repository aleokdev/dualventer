#include "bot.hpp"

#include <client_ws.hpp>
#include <cpr/cpr.h>
#include <iostream>
#include <rapidjson/document.h>
#include <utility>
#include <future>
#include <chrono>

namespace dualventer {

Bot::Bot(std::string token) : token(std::move(token)), client(get_gateway_uri()) {
  std::cout << "Gateway URI: " << get_gateway_uri() << std::endl;
  client.on_open = [](std::shared_ptr<Client::Connection> connection) {
    std::cout << "Client: Opened connection" << std::endl;
  };
  client.on_error = [](std::shared_ptr<Client::Connection> /*connection*/, const SimpleWeb::error_code &ec) {
    std::cout << "Client: Error: " << ec << ", error message: " << ec.message() << std::endl;
  };
  client.on_message = [](std::shared_ptr<Client::Connection> connection, std::shared_ptr<Client::Message> message) {
    std::cout << "Client: Message received: \"" << message->string() << "\"" << std::endl;
  };
  std::cout << "Starting client" << std::endl;
  client.start();
}

void Bot::send_message(std::string const & txt) {

}

std::string Bot::get_gateway_uri() {
  auto r = cpr::Get(cpr::Url{api_url+"/gateway"}, cpr::VerifySsl(false));
  std::cout << r.error.message << std::endl;
  std::cout << r.text << std::endl;
  rapidjson::Document doc;
  doc.Parse(r.text.c_str());
  std::string str(doc["url"].GetString());
  str = str.substr(6) + "/?v=6&encoding=json"; // Remove "wss://" and specify API version and encoding to use
  return str;
}

}