#ifndef DUALVENTER_BOT_HPP
#define DUALVENTER_BOT_HPP

#include <client_wss.hpp>

#include "command.hpp"
#include "snowflake.hpp"

#include <atomic>
#include <future>
#include <string>
#include <string_view>
#include <unordered_map>
#include <filesystem>

using fs = std::filesystem;

namespace dualventer {

class Bot {
public:
  using Client = SimpleWeb::SocketClient<SimpleWeb::WSS>;

  Bot(std::string token);
  ~Bot();

  void load_module(fs::path const&);

  void send_message(Snowflake const& channel_id, std::string const&);

  const inline static std::string api_url{"https://discordapp.com/api/v6"};

private:
  std::atomic<bool> connection_closed = false;
  std::atomic<size_t> last_sequence_number = -1;
  std::thread heartbeat;
  Client client;

  const std::string token;

  std::string get_gateway_uri();

  std::unordered_map<std::string, Command> commands;

  void heartbeat_func(std::size_t interval, std::shared_ptr<Client::Connection> connection);
};

}

#endif // DUALVENTER_BOT_HPP
