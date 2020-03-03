#ifndef DUALVENTER_CONTEXT_HPP
#define DUALVENTER_CONTEXT_HPP

#include "snowflake.hpp"

#include <rapidjson/document.h>
#include <sol/sol.hpp>

#include <optional>
#include <rapidjson/document.h>
#include <string>
#include <vector>

namespace dualventer {

// https://discordapp.com/developers/docs/resources/user#user-object
struct User {
  Snowflake id;
  std::string username;
  unsigned int discriminator;
  //std::string avatar_hash;
  //bool is_bot;
  //std::string locale;
  //unsigned int flags;
  //unsigned int nitro_type;
};

struct Role {
  Snowflake id;
  std::string name;
  unsigned int color;
  bool display_separately;
  int position;
  unsigned int permissions;
  bool managed_by_integration;
  bool mentionable;
};

class Bot;
// https://discordapp.com/developers/docs/resources/channel#channel-object
struct Channel {
  enum class Type {
    guild_text,
    dm,
    guild_voice,
    group_dm,
    guild_category,
    guild_news,
    guild_store
  };

  explicit Channel(Bot& bot, Snowflake id);

#define GENERIC_GET_FUNC(t, n) inline t get_##n() { if (loaded) return n; else { load_data(); return n; }}

  Snowflake id;
  GENERIC_GET_FUNC(Type, type);
  GENERIC_GET_FUNC(Snowflake, guild_id);
  GENERIC_GET_FUNC(int, position);
  GENERIC_GET_FUNC(std::string, name);
  GENERIC_GET_FUNC(std::string, topic);
  GENERIC_GET_FUNC(bool, nsfw);
  GENERIC_GET_FUNC(Snowflake, last_message_id);
  // TODO: permission_overwrites
  // TODO: bitrate
  // TODO: user_limit
  // TODO: rate_limit_per_user
  // TODO: recipients
  // TODO: icon
  // TODO: owner_id
  // TODO: application_id
  // TODO: parent_id
  // TODO: last_pin_timestamp
#undef GENERIC_GET_FUNC

  bool loaded = false;

private:
  void load_data();
  Bot* bot;
  Type type;
  Snowflake guild_id = -1;
  // TODO: replace guild_id for guild
  int position;
  std::string name;
  std::string topic;
  bool nsfw;
  Snowflake last_message_id = -1;
};

struct ChannelMention {
  Snowflake id;
  Snowflake guild_id;
  Channel::Type type;
  std::string name;
};

struct Attachment {
  Snowflake id;
  std::string filename;
  std::size_t size;
  std::string url;
  std::string proxy_url;
  int height; // -1 if not image
  int width;  // -1 if not image
};

struct Embed {
  std::string title;
  std::string type;
  std::string description;
  std::string url;
  // TODO: timestamp
  unsigned int color;
  struct EmbedFooter {
    std::string text;
    std::string icon_url;
    std::string proxy_icon_url;
  } footer;
  struct EmbedImage {
    std::string url;
    std::string proxy_url;
    int height;
    int width;
  } image;
  struct EmbedProvider {
    std::string name;
    std::string url;
  } provider;
  struct EmbedAuthor {
    std::string name;
    std::string url;
    std::string icon_url;
    std::string proxy_icon_url;
  } author;
  struct EmbedField {
    std::string name;
    std::string value;
    bool pos_inline = false;
  };
  std::vector<EmbedField> fields;
};

class Message {
public:
  explicit Message(Bot& bot, rapidjson::Document &ws_msg);

  Bot* bot;
  Snowflake id;
  Channel channel;
  // TODO: guild (guild_id)
  User author;
  std::string content;
  // bool tts;
  // bool mentions_everyone;
  // TODO: member
  // TODO: timestamp
  // TODO: edited_timestamp
  // [[nodiscard]] std::vector<User> get_mentions();
  // [[nodiscard]] std::vector<Role> get_role_mentions();
  // [[nodiscard]] std::vector<ChannelMention> get_channel_mentions();
  // [[nodiscard]] std::vector<Attachment> get_attachments();
};

class Context {
public:
  Bot *bot;
  Message message;
  inline Channel& get_channel() { return message.channel; }

  void send(sol::table);
};

} // namespace dualventer

#endif // DUALVENTER_CONTEXT_HPP
