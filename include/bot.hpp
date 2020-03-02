#ifndef DUALVENTER_BOT_HPP
#define DUALVENTER_BOT_HPP

#include <client_wss.hpp>

#include <future>
#include <string_view>
#include <string>
#include <atomic>

namespace dualventer {

class Bot {
public:
  using Client = SimpleWeb::SocketClient<SimpleWeb::WSS>;

  Bot(std::string token);
  ~Bot();

  void send_message(std::string const&);

  const inline static std::string api_url{"https://discordapp.com/api/v6"};
private:
  std::atomic<bool> connection_closed = false;
  std::atomic<size_t> last_sequence_number = -1;
  std::thread heartbeat;
  Client client;

  const std::string token;

  std::string get_gateway_uri();
};

}

#endif // DUALVENTER_BOT_HPP
