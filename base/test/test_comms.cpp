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

void updateStationRegistry(const String&, const String&, const String&, const String&, float, float) {}
void setupRegistryRoutes(AsyncWebServer&) {}

const char* unitTestRegistryJson = nullptr;

void onDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len);

int main() {
  uint8_t mac1[6] = {1,2,3,4,5,6};
  TestMessage msg1{"current", 10.0f, ""};
  onDataRecv(mac1, reinterpret_cast<uint8_t*>(&msg1), sizeof(msg1));

  currentMillis += 800; // exceed debounce
  comms_loop();
  sendRecords.clear();

  uint8_t mac2[6] = {1,2,3,4,5,7};
  TestMessage msg2{"current", 0.0f, ""};
  onDataRecv(mac2, reinterpret_cast<uint8_t*>(&msg2), sizeof(msg2));

  assert(sendRecords.size() == 1);
  assert(std::memcmp(sendRecords[0].mac, mac2, 6) == 0);
  assert(sendRecords[0].data == 0);
  return 0;
}
