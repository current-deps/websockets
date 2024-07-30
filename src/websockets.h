#pragma once

#include <functional>
#include <iostream>
#include <new>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include "current.h"

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

template <typename Fn, int N = 0, typename Callable> Fn *fnptr(Callable &&c) {
  return fnptr_<N>(std::forward<Callable>(c), (Fn *)nullptr);
}

class WebsocketClient {
private:
  ws_cli_conn_t *ws_;

public:
  explicit WebsocketClient(ws_cli_conn_t *ws) { ws_ = ws; }
  WebsocketClient(const WebsocketClient &) = default;
  ~WebsocketClient() = default;

  int State() { return ws_get_state(ws_); };
  std::string Address() { return std::string(ws_getaddress(ws_)); }
  std::string Port() { return std::string(ws_getport(ws_)); }

  void SendText(std::vector<uint8_t> buffer) {
    ws_sendframe_txt(ws_, (char *)(buffer.data()));
  }
  void SendBin(std::vector<uint8_t> buffer) {
    ws_sendframe_bin(ws_, (char *)(buffer.data()), buffer.size());
  }
};

class WebsocketServer {
protected:
  struct ws_server ws_;

public:
  explicit WebsocketServer(
      std::function<void(WebsocketClient &client, std::vector<uint8_t> data,
                         int type)>
          on_data,
      std::function<void(WebsocketClient &client)> on_connected,
      std::function<void(WebsocketClient &client)> on_disconnected,
      int port = 8080, std::string host = std::string("0.0.0.0"),
      int n_threads = 0, int timeout_ms = 1000) {
    ws_.host = host.c_str();
    ws_.port = port;

    /*
     * *If the .thread_loop is != 0, a new thread is created
     * to handle new connections and ws_socket() becomes
     * non-blocking.
     */
    ws_.thread_loop = n_threads;
    ws_.timeout_ms = timeout_ms;

    ws_.evs.onopen =
        fnptr<void(ws_cli_conn_t *)>([on_connected](ws_cli_conn_t *client) {
          auto wrapper = WebsocketClient(client);
          on_connected(wrapper);
        });
    ws_.evs.onclose =
        fnptr<void(ws_cli_conn_t *)>([on_disconnected](ws_cli_conn_t *client) {
          auto wrapper = WebsocketClient(client);
          on_disconnected(wrapper);
        });
    ws_.evs.onmessage =
        fnptr<void(ws_cli_conn_t *, const unsigned char *, uint64_t, int)>(
            [on_data](ws_cli_conn_t *client, const unsigned char *data,
                      uint64_t size, int type) {
              auto buffer_cpp = bytes_to_vector(data, size);
              auto wrapper = WebsocketClient(client);
              on_data(wrapper, buffer_cpp, type);
            });
  }
  WebsocketServer(const WebsocketClient &) = delete;
  ~WebsocketServer() = default;
  static std::vector<uint8_t> bytes_to_vector(const unsigned char *data,
                                              uint64_t size) {
    std::vector<uint8_t> result(0, size);
    for (int i = 0; i <= size; i++) {
      result.push_back(static_cast<uint8_t>(data[i]));
    }
    return std::move(result);
  }

  void start() { ws_socket(&ws_); }
};
