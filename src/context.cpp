#include "context.hpp"
#include "bot.hpp"

#include <cpr/api.h>
#include <rapidjson/schema.h>
#include <rapidjson/writer.h>

namespace dualventer {

Message::Message(Bot &b, rapidjson::Document &ws_msg)
    : bot(&b), id(std::stoull(ws_msg["d"]["id"].GetString())),
      channel(b, std::stoull(ws_msg["d"]["channel_id"].GetString())),
      author{std::stoull(ws_msg["d"]["author"]["id"].GetString())} {
  author.username = ws_msg["d"]["author"]["username"].GetString();
  author.discriminator =
      std::stoi(ws_msg["d"]["author"]["discriminator"].GetString());
}

Channel::Channel(Bot &b, Snowflake id) : bot(&b), id(id) {}

void Channel::load_data() {
  auto r = cpr::Get(
      cpr::Url{Bot::api_url + "/channels/" + id.to_string()},
      cpr::Header{{"Content-Type", "application/json"},
                  {"User-Agent", "DiscordBot "
                                 "(https://github.com/alexdevteam/dualventer, "
                                 "0.0.0)"}, // TODO: replace with version macro
                  {"Authorization", "Bot " + bot->get_token()}},
      cpr::VerifySsl(false));
  rapidjson::Document ws_msg;
  ws_msg.Parse(r.text.c_str());
  type = (Type)ws_msg["type"].GetInt();
#define GET_OPTIONAL_ITEM(name, typefunc)                                      \
  if (ws_msg.HasMember(#name) &&                                               \
      ws_msg[#name].GetType() != rapidjson::kNullType)                         \
    name = ws_msg[#name].typefunc();
#define GET_OPTIONAL_ID(name)                                                  \
  if (ws_msg.HasMember(#name))                                                 \
    name = std::stoull(ws_msg[#name].GetString());
  GET_OPTIONAL_ITEM(position, GetInt)
  else position = -1;

  GET_OPTIONAL_ID(guild_id)
  else guild_id = -1;
  GET_OPTIONAL_ID(last_message_id)
  else last_message_id = -1;

  GET_OPTIONAL_ITEM(name, GetString)
  GET_OPTIONAL_ITEM(topic, GetString)

  GET_OPTIONAL_ITEM(nsfw, GetBool)
  else nsfw = false;

#undef GET_OPTIONAL_ITEM
#undef GET_OPTIONAL_ID
}

void Context::send(sol::table table) {
  rapidjson::StringBuffer buf;
  rapidjson::Writer<rapidjson::StringBuffer> sw(buf);
  sw.StartObject();
  for (auto &[key, val] : table) {
    switch (key.get_type()) {
    case sol::type::string:
      if (key.as<std::string>() == "embed") {
        if (!val.is<sol::table>())
          throw std::runtime_error(
              "Expected table for embed argument given to Context::send");
        sw.Key("embed");
        sw.StartObject();
        for (auto &[embed_key, embed_val] : val.as<sol::table>()) {
          if (!embed_key.is<std::string>())
            throw std::runtime_error(
                "Expected string keys in embed argument inside Context::send");
          if (embed_key.as<std::string>() == "title") {
            sw.Key("title");
            sw.String(embed_val.as<std::string>().c_str());
          } else if (embed_key.as<std::string>() == "description") {
            sw.Key("description");
            sw.String(embed_val.as<std::string>().c_str());
          }
        }
        sw.EndObject();
      } else
        throw std::runtime_error("Invalid argument given to Context::send: " +
                                 key.as<std::string>());
      break;

    case sol::type::number: {
      if (key.as<int>() == 1) {
        sw.Key("content");
        std::string val_str(val.as<std::string>());
        sw.String(val_str.c_str());
      } else
        throw std::runtime_error(
            "Too many positional arguments given to Context::send!");
    } break;

    default:
      throw std::runtime_error("Invalid type given to Context::send!");
    }
  }
  sw.Key("tts");
  sw.Bool(false);
  sw.EndObject();
  auto r = cpr::Post(
      cpr::Url{Bot::api_url + "/channels/" + message.channel.id.to_string() +
               "/messages"},
      cpr::Header{{"Content-Type", "application/json"},
                  {"User-Agent", "DiscordBot "
                                 "(https://github.com/alexdevteam/dualventer, "
                                 "0.0.0)"}, // TODO: replace with version macro
                  {"Authorization", "Bot " + bot->get_token()}},
      cpr::Body{buf.GetString()}, cpr::VerifySsl(false));
  std::cout << "Status code: " << r.status_code
            << ", MESSAGE_CREATE response: " << r.text << std::endl;
}
} // namespace dualventer