#pragma once

#include <functional>
#include <iostream>  // IWYU pragma: keep
#include <string>
#include <vector>  // IWYU pragma: keep

extern "C" {
#include "include/base64.h"
#include "include/sha1.h"
#include "include/utf8.h"
#include "include/ws.h"
}

static std::string default_host = "0.0.0.0";

class WebsocketClient final {
 private:
  ws_connection *ws_;

 public:
  explicit WebsocketClient(ws_connection *ws) { ws_ = ws; }

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

  void SendText(std::string_view buffer) { ws_sendframe_txt(ws_, reinterpret_cast<const char *>(buffer.data())); }
  void SendBin(std::string_view buffer) {
    ws_sendframe_bin(ws_, reinterpret_cast<const char *>(buffer.data()), buffer.size());
  }
};

class WebsocketServer final {
 protected:
  struct ws_server ws_;

  using on_connected_t = std::function<void(WebsocketClient &client)>;
  using on_disconnected_t = std::function<void(WebsocketClient &client)>;
  using on_data_t = std::function<void(WebsocketClient &client, std::string_view data, int type)>;

  const on_connected_t on_connected_;
  const on_disconnected_t on_disconnected_;
  const on_data_t on_data_;

 public:
  WebsocketServer(const WebsocketServer &) = delete;
  ~WebsocketServer() = default;

  // TODO: Reconsider making `WebsocketServer`-s movable.
  WebsocketServer(WebsocketServer &&) = delete;
  WebsocketServer &operator=(const WebsocketServer &) = delete;
  WebsocketServer &operator=(WebsocketServer &&) = delete;

  explicit WebsocketServer(on_data_t on_data,
                           on_connected_t on_connected,
                           on_disconnected_t on_disconnected,
                           int port = 8080,
                           const std::string &host = default_host,
                           int n_threads = 0,
                           int timeout_ms = 1000)
      : on_data_(std::move(on_data)),
        on_connected_(std::move(on_connected)),
        on_disconnected_(std::move(on_disconnected)) {
    ws_.host = host.c_str();
    ws_.port = port;
    ws_.context = this;

    // If `.thread_loop` is not zero, a new thread handles each new connection, and `ws_socket()` is non-blocking.
    ws_.thread_loop = n_threads;
    ws_.timeout_ms = timeout_ms;

    // NOTE(dkorolev): Non-capturing lambdas can be cast into C-style callbacks directly.
    ws_.evs.onopen = StaticOnOpen;
    ws_.evs.onclose = StaticOnClose;
    ws_.evs.onmessage = StaticOnMessage;
  }

  static void StaticOnOpen(ws_connection *c) {
    auto ptr = reinterpret_cast<WebsocketServer *>(ws_get_server_context(c));
    WebsocketClient wsc(c);
    ptr->on_connected_(wsc);
  }
  static void StaticOnMessage(ws_connection *c, const unsigned char *data, uint64_t size, int type) {
    auto ptr = reinterpret_cast<WebsocketServer *>(ws_get_server_context(c));
    WebsocketClient wsc(c);
    auto buffer_cpp = std::string_view(reinterpret_cast<const char*>(data), size);
    ptr->on_data_(wsc, buffer_cpp, type);
  }

  static void StaticOnClose(ws_connection *c) {
    auto ptr = reinterpret_cast<WebsocketServer *>(ws_get_server_context(c));
    WebsocketClient wsc(c);
    ptr->on_disconnected_(wsc);
  }

  void start() { ws_socket(&ws_); }
};
