#include "Arduino.h"
#include "ArduinoJson.h"
#include "comms.h"
#include "esp_now.h"
#include "config_ui.h"
#include <vector>
#include <cstring>
#include <cassert>
#include "ESPAsyncWebServer.h"

HardwareSerial Serial;
unsigned long currentMillis = 0;
unsigned long millis() { return currentMillis; }
void digitalWrite(int, int) {}

struct SendRecord {
  uint8_t mac[6];
  uint8_t data;
  size_t len;
};
std::vector<SendRecord> sendRecords;

esp_err_t esp_now_send(const uint8_t* mac, const uint8_t* data, size_t len) {
  SendRecord r;
  memcpy(r.mac, mac, 6);
  r.data = data[0];
  r.len = len;
  sendRecords.push_back(r);
  return ESP_OK;
}

bool esp_now_is_peer_exist(const uint8_t*) { return true; }
esp_err_t esp_now_add_peer(const esp_now_peer_info_t*) { return ESP_OK; }
esp_err_t esp_now_init() { return ESP_OK; }
void esp_now_register_recv_cb(esp_now_recv_cb_t) {}

class DummyConfigUI : public ConfigUI {
public:
  void begin(AsyncWebServer&) override {}
  void loadConfig() override {}
  void saveConfig() override {}
  void setBaseMac(const String&) override {}
  String getFriendlyName() const override { return ""; }
  String getBaseMac() const override { return ""; }
  String getMdnsName() const override { return ""; }
  float getToolOnThreshold() const override { return 5.0f; }
  void setToolOnThreshold(float) override {}
  float getCollectorOffDelay() const override { return 0.0f; }
  void setCollectorOffDelay(float) override {}
};

DummyConfigUI dummyConfigUI;
ConfigUI& configUI = dummyConfigUI;
bool mdnsStarted = false;
extern const uint8_t RELAY_PIN = 1;

void updateStationRegistry(const String&, const String&, const String&, const String&, float) {}
void setupRegistryRoutes(AsyncWebServer&) {}

const char* unitTestRegistryJson = "[{\"mac\":\"01:02:03:04:05:08\"}]";

void onDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len);

int main() {
  comms_setup();
  sendRecords.clear();

  uint8_t activeMac[6] = {1,2,3,4,5,6};
  TestMessage msg{"current", 10.0f, ""};
  onDataRecv(activeMac, reinterpret_cast<uint8_t*>(&msg), sizeof(msg));

  currentMillis += 800; // exceed debounce
  comms_loop();

  uint8_t silentMac[6] = {1,2,3,4,5,8};
  bool found = false;
  for (const auto& r : sendRecords) {
    if (std::memcmp(r.mac, silentMac, 6) == 0 && r.data == 0) {
      found = true;
      break;
    }
  }
  assert(found);
  return 0;
}
