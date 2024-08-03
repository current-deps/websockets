#pragma once

#include <cstdint>  // uint8_t
#include <functional>
#include <iostream>  // IWYU pragma: keep
#include <new>
#include <string>
#include <utility>
#include <vector>  // IWYU pragma: keep

extern "C" {
#include "include/base64.h"
#include "include/sha1.h"
#include "include/utf8.h"
#include "include/ws.h"
}

template <int, typename Callable, typename Ret, typename... Args>
auto fnptr_(Callable &&c, Ret (*)(Args...)) {
  static std::decay_t<Callable> storage = std::forward<Callable>(c);
  static bool used = false;
  if (used) {
    using type = decltype(storage);
    storage.~type();
    new (&storage) type(std::forward<Callable>(c));
  }
  used = true;

  return [](Args... args) -> Ret {
    auto &c = *std::launder(&storage);
    return Ret(c(std::forward<Args>(args)...));
  };
}

template <typename Fn, int N = 0, typename Callable>
Fn *fnptr(Callable &&c) {
  return fnptr_<N>(std::forward<Callable>(c), (Fn *)nullptr);
}

class WebsocketClient final {
 private:
  ws_cli_conn_t *ws_;

 public:
  explicit WebsocketClient(ws_cli_conn_t *ws) { ws_ = ws; }

  WebsocketClient(const WebsocketClient &) = default;
  ~WebsocketClient() = default;

  // TODO: Reconsider making `WebsocketClient`-s movable. Perhaps move it all into some `std::unique_ptr<Impl>`?
  WebsocketClient(WebsocketClient &&) = delete;
  WebsocketClient &operator=(const WebsocketClient &) = delete;
  WebsocketClient &operator=(WebsocketClient &&) = delete;

  // QUESTION: Why? Can't this `Close()` just be the destructor?
  void Close() { ws_close_client(ws_); }

  int State() const { return ws_get_state(ws_); };
  std::string Address() const { return ws_getaddress(ws_); }
  std::string Port() const { return ws_getport(ws_); }

  void SendText(std::vector<uint8_t> buffer) { ws_sendframe_txt(ws_, reinterpret_cast<const char *>(buffer.data())); }
  void SendBin(std::vector<uint8_t> buffer) {
    ws_sendframe_bin(ws_, reinterpret_cast<const char *>(buffer.data()), buffer.size());
  }
};

class WebsocketServer final {
 protected:
  struct ws_server ws_;

 public:
  WebsocketServer(const WebsocketServer &) = delete;
  ~WebsocketServer() = default;

  // TODO: Reconsider making `WebsocketServer`-s movable.
  WebsocketServer(WebsocketServer &&) = delete;
  WebsocketServer &operator=(const WebsocketServer &) = delete;
  WebsocketServer &operator=(WebsocketServer &&) = delete;

  explicit WebsocketServer(std::function<void(WebsocketClient &client, std::vector<uint8_t> data, int type)> on_data,
                           std::function<void(WebsocketClient &client)> on_connected,
                           std::function<void(WebsocketClient &client)> on_disconnected,
                           int port = 8080,
                           std::string host = "0.0.0.0",
                           int n_threads = 0,
                           int timeout_ms = 1000) {
    ws_.host = host.c_str();
    ws_.port = port;

    // If `.thread_loop` is not zero, a new thread handles each new connection, and `ws_socket()` is non-blocking.
    ws_.thread_loop = n_threads;
    ws_.timeout_ms = timeout_ms;

    ws_.evs.onopen = fnptr<void(ws_cli_conn_t *)>([on_connected](ws_cli_conn_t *client) {
      auto wrapper = WebsocketClient(client);
      on_connected(wrapper);
    });
    ws_.evs.onclose = fnptr<void(ws_cli_conn_t *)>([on_disconnected](ws_cli_conn_t *client) {
      auto wrapper = WebsocketClient(client);
      on_disconnected(wrapper);
    });
    ws_.evs.onmessage = fnptr<void(ws_cli_conn_t *, const unsigned char *, uint64_t, int)>(
        [on_data](ws_cli_conn_t *client, const unsigned char *data, uint64_t size, int type) {
          auto buffer_cpp = bytes_to_vector(data, size);
          auto wrapper = WebsocketClient(client);
          on_data(wrapper, buffer_cpp, type);
        });
  }
  static std::vector<uint8_t> bytes_to_vector(const unsigned char *data, uint64_t size) {
    auto p = reinterpret_cast<uint8_t const *>(data);
    return std::vector<uint8_t>(p, p + size);
  }

  void start() { ws_socket(&ws_); }
};
