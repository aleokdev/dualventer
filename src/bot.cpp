#include "bot.hpp"

#include <chrono>
#include <cpr/cpr.h>
#include <future>
#include <iostream>
#include <rapidjson/document.h>
#include <utility>

namespace dualventer {

Bot::Bot(std::string token)
    : token(std::move(token)), client(get_gateway_uri()) {
  std::cout << "Gateway URI: " << get_gateway_uri() << std::endl;
  client.on_open = [](std::shared_ptr<Client::Connection> connection) {
    std::cout << "Client: Opened connection" << std::endl;
  };
  client.on_error = [](std::shared_ptr<Client::Connection> /*connection*/,
                       const SimpleWeb::error_code &ec) {
    std::cout << "Client: Error: " << ec << ", error message: " << ec.message()
              << std::endl;
  };
  client.on_message = [this](std::shared_ptr<Client::Connection> connection,
                             std::shared_ptr<Client::Message> message) {
    std::string str_message = message->string();
    std::cout << "Client: Message received: \"" << str_message << "\""
              << std::endl;
    rapidjson::Document doc;
    doc.Parse(str_message.c_str());
    if (doc["s"].GetType() == rapidjson::Type::kNullType)
      last_sequence_number = -1;
    else
      last_sequence_number = doc["s"].GetUint64();
    switch (doc["op"].GetUint()) {
    // TODO: Reconnect if server doesn't respond to heartbeats with opcode 11
    case 10: // Hello (handshake)
    {
      std::size_t heartbeat_interval =
          doc["d"]["heartbeat_interval"].GetUint64();
      heartbeat = std::thread([this, heartbeat_interval, connection]() {
        for (;;) {
          std::this_thread::sleep_for(
              std::chrono::milliseconds(heartbeat_interval));
          if (connection_closed)
            return;
          std::cout << "Sending heartbeat." << std::endl;
          std::string hb_str(R"({"op":1,"d":)");
          if (last_sequence_number == (std::size_t)-1)
            hb_str += "null}";
          else
            hb_str += std::to_string(last_sequence_number) + "}";
          auto send_stream = std::make_shared<Client::SendStream>();
          *send_stream << hb_str;
          connection->send(send_stream);
        }
      });
      heartbeat.detach();
    } break;
    }
  };
  std::cout << "Starting client" << std::endl;
  client.start();
}

void Bot::send_message(std::string const &txt) {}

std::string Bot::get_gateway_uri() {
  auto r = cpr::Get(cpr::Url{api_url + "/gateway"}, cpr::VerifySsl(false));
  std::cout << r.error.message << std::endl;
  std::cout << r.text << std::endl;
  rapidjson::Document doc;
  doc.Parse(r.text.c_str());
  std::string str(doc["url"].GetString());
  str = str.substr(6) + "/?v=6&encoding=json"; // Remove wss:// and specify API
                                               // version and encoding to use
  return str;
}

Bot::~Bot() {
  connection_closed = false;
  std::cout << "Closing connection." << std::endl;
}

} // namespace dualventer