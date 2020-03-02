#ifndef DUALVENTER_BOT_HPP
#define DUALVENTER_BOT_HPP

#include <client_ws.hpp>

#include <future>
#include <string_view>
#include <string>

namespace dualventer {

class Bot {
public:
  using Client = SimpleWeb::SocketClient<SimpleWeb::WS>;

  Bot(std::string token);

  void send_message(std::string const&);

  const inline static std::string api_url{"https://discordapp.com/api/v6"};
private:
  bool connection_closed = false;
  std::thread heartbeat;
  Client client;

  const std::string token;

  std::string get_gateway_uri();
};

}

#endif // DUALVENTER_BOT_HPP
