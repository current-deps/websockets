#include "websockets.h"

int main() {
  auto server = WebsocketServer(
      [](WebsocketClient &client, std::vector<uint8_t> data, int type) {
        std::cout << "WebSocket Message: " << std::string((char *)(data.data())) << std::endl;
        std::string answer = "Hello from server!";
        std::vector<uint8_t> to_send(answer.begin(), answer.end());
        client.SendText(to_send);
      },
      [](WebsocketClient &client) {
        std::cout << "Client " << client.Address() << ":" << client.Port() << " connected" << std::endl;
      },
      [](WebsocketClient &client) {
        std::cout << "Client " << client.Address() << ":" << client.Port() << " disconnected" << std::endl;
      },
      8080);
  std::cout << "Started WebSocket server on port 8080." << std::endl;
  server.start();
  return 0;
}
