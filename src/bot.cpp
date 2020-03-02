#include "bot.hpp"

#include <chrono>
#include <cpr/cpr.h>
#include <future>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <utility>

namespace dualventer {

Bot::Bot(std::string t) : token(std::move(t)), client(get_gateway_uri()) {
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
      // Start heartbeat
      std::size_t heartbeat_interval =
          doc["d"]["heartbeat_interval"].GetUint64();
      heartbeat = std::thread(&Bot::heartbeat_func, this, heartbeat_interval,
                              connection);
      heartbeat.detach();
      std::cout << "Started heartbeat." << std::endl;

      // Identify
      rapidjson::StringBuffer buf;
      rapidjson::Writer<rapidjson::StringBuffer> w(buf);
      /* clang-format off */
      w.StartObject();
      w.Key("op"); w.Int(2);
      w.Key("d"); w.StartObject();
      {
      w.Key("token"); w.String(token.c_str());
      w.Key("properties");
      w.StartObject(); {
        w.Key("$os");       w.String("Linux");
        w.Key("$browser");  w.String("Dualventer");
        w.Key("$device");   w.String("Dualventer");
      } w.EndObject();
      w.Key("presence");
      w.StartObject(); {
        w.Key("since");    w.Int(0);
        w.Key("game");     w.StartObject();
        {
          w.String("name"); w.String(">>help");
          w.Key("type"); w.Int(1);
          w.Key("url"); w.String("https://www.youtube.com/channel/UCleUBml3YGQf1InmZQ-yJXw");
        } w.EndObject();
        w.Key("status");   w.String("dnd");
        w.Key("afk");      w.Bool(false);
      } w.EndObject();
      } w.EndObject();
      w.EndObject();
      /* clang-format on */
      auto send_stream = std::make_shared<Client::SendStream>();
      *send_stream << buf.GetString();
      connection->send(send_stream);
      std::cout << "Sent identification info." << std::endl;
    } break;

    case (0): {
      if (doc["t"] == "MESSAGE_CREATE" && !doc["d"].HasMember("webhook_id")) {
        if (doc["d"]["author"].HasMember("bot"))
          if (doc["d"]["author"]["bot"].GetBool())
            return;
        send_message(std::stoull(doc["d"]["channel_id"].GetString()), "test!");
      }
    } break;
    }
  };
  std::cout << "Starting client" << std::endl;
  client.start();
}

void Bot::send_message(Snowflake const &channel_id, std::string const &content_txt) {
  rapidjson::StringBuffer buf;
  rapidjson::Writer<rapidjson::StringBuffer> sw(buf);
  sw.StartObject();
  sw.Key("content"); sw.String(content_txt.c_str());
  sw.Key("tts"); sw.Bool(false);
  sw.EndObject();
  auto r = cpr::Post(
      cpr::Url{api_url + "/channels/"+std::to_string(channel_id.id)+"/messages"},
      cpr::Header{{"Content-Type", "application/json"},
                  {"User-Agent",
                      "DiscordBot "
                      "(https://github.com/alexdevteam/dualventer, "
                      "0.0.0)"}, // TODO: replace with version macro
                  {"Authorization", "Bot " + token}},
      cpr::Body{buf.GetString()},
      cpr::VerifySsl(false));
  std::cout << "Status code: " << r.status_code
            << ", MESSAGE_CREATE response: " << r.text << std::endl;
}

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
  client.stop();
}

void Bot::heartbeat_func(std::size_t interval,
                         std::shared_ptr<Client::Connection> connection) {
  for (;;) {
    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
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
}

} // namespace dualventer