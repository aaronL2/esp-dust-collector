#pragma once
#include <cstddef>
#include <cstdint>

typedef int esp_err_t;
constexpr esp_err_t ESP_OK = 0;

using esp_now_recv_cb_t = void (*)(const uint8_t*, const uint8_t*, int);

struct esp_now_peer_info_t {
  uint8_t peer_addr[6];
  int channel;
  bool encrypt;
};

esp_err_t esp_now_init();
esp_err_t esp_now_send(const uint8_t* mac, const uint8_t* data, size_t len);
bool esp_now_is_peer_exist(const uint8_t* mac);
esp_err_t esp_now_add_peer(const esp_now_peer_info_t* peer);
void esp_now_register_recv_cb(esp_now_recv_cb_t cb);
