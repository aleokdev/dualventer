#include "bot.hpp"
#include "context.hpp"

#include <chrono>
#include <cpr/cpr.h>
#include <exception>
#include <future>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <utility>

namespace dualventer {

Bot::Bot(std::string t, fs::path modules_dir)
    : token(std::move(t)),
      client(get_gateway_uri()) {
  Bot::setup_usertypes(lua_state);
  lua_state["bot"] = this;
  lua_state.open_libraries(sol::lib::base);

  load_commands(modules_dir);

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

    case 0: {
      if (doc["t"] == "MESSAGE_CREATE" && !doc["d"].HasMember("webhook_id")) {
        if (doc["d"]["author"].HasMember("bot"))
          if (doc["d"]["author"]["bot"].GetBool())
            return;
        std::string msg_str = doc["d"]["content"].GetString();
        if (msg_str.substr(0, prefix.size()) == prefix) {
          std::cout << msg_str.substr(prefix.size()) << std::endl;
          auto cmd_iter = commands.find(msg_str.substr(prefix.size()));
          if (cmd_iter != commands.end()) {
            std::cout << "Executing command" << std::endl;
            Message mmsg(*this, doc);
            cmd_iter->second["callback"](Context{this, mmsg});
          }
        }
      }
    } break;
    }
  };
}

void Bot::send_message(Snowflake const &channel_id,
                       std::string const &content_txt) {
  rapidjson::StringBuffer buf;
  rapidjson::Writer<rapidjson::StringBuffer> sw(buf);
  sw.StartObject();
  sw.Key("content");
  sw.String(content_txt.c_str());
  sw.Key("tts");
  sw.Bool(false);
  sw.EndObject();
  auto r = cpr::Post(
      cpr::Url{api_url + "/channels/" + std::to_string(channel_id.id) +
               "/messages"},
      cpr::Header{{"Content-Type", "application/json"},
                  {"User-Agent", "DiscordBot "
                                 "(https://github.com/alexdevteam/dualventer, "
                                 "0.0.0)"}, // TODO: replace with version macro
                  {"Authorization", "Bot " + token}},
      cpr::Body{buf.GetString()}, cpr::VerifySsl(false));
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

void Bot::load_module(fs::path const &path) {
  lua_state.do_file(path);
  std::cout << "Loaded module " << path.string() << std::endl;
}

void Bot::load_commands(fs::path dir) {
  for (auto &module_path : fs::directory_iterator(dir))
    load_module(module_path);
}

void Bot::setup_usertypes(sol::state &state) {
  /* clang-format off */
  state.new_usertype<Bot>("Bot",
      "add_command", &Bot::add_command,
      "prefix", &Bot::prefix,
      "commands", sol::readonly(&Bot::commands));

  state.new_usertype<Message>("Message",
      "id", sol::readonly(&Message::id),
      "channel", sol::readonly(&Message::channel),
      "author", sol::readonly(&Message::author),
      "content", sol::readonly(&Message::content));

  state.new_usertype<Snowflake>("Snowflake",
      sol::meta_function::to_string, &Snowflake::to_string);

  state.new_enum<Channel::Type>("ChannelType", {
    {"guild_text", Channel::Type::guild_text},
    {"dm", Channel::Type::dm},
    {"guild_voice", Channel::Type::guild_voice},
    {"group_dm", Channel::Type::group_dm},
    {"guild_category", Channel::Type::guild_category},
    {"guild_news", Channel::Type::guild_news},
    {"guild_store", Channel::Type::guild_store}});

  state.new_usertype<Channel>("Channel",
      "id", sol::readonly(&Channel::id),
      "type", sol::readonly_property(&Channel::get_type),
      "position", sol::readonly_property(&Channel::get_position),
      "name", sol::readonly_property(&Channel::get_name),
      "topic", sol::readonly_property(&Channel::get_topic),
      "nsfw", sol::readonly_property(&Channel::get_nsfw),
      "last_message_id", sol::readonly_property(&Channel::get_last_message_id));

  state.new_usertype<Context>("Context",
      "bot", sol::readonly(&Context::bot),
      "message", sol::readonly(&Context::message),
      "channel", sol::readonly_property(&Context::get_channel),
      "send", &Context::send);
  /* clang-format on */
}

void Bot::add_command(sol::table table) {
  if (table["name"].get_type() != sol::type::string)
    throw std::runtime_error(
        "Command hasn't defined a name, or its type is invalid.");

  commands[table["name"]] = table;
  std::cout << "Added command named " << table["name"].get<std::string>()
            << std::endl;
}

std::string Bot::get_token() { return token; }

void Bot::run() {
  std::cout << "Starting client" << std::endl;
  client.start();
}

} // namespace dualventer