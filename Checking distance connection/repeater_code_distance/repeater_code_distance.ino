#include <ESP8266WiFi.h>
#include <espnow.h>

// Insert your SSID
constexpr char WIFI_SSID[] = "TEST Looking for Job";
constexpr char WIFI_SSID_home_router[] = "...";

// Board 1
uint8_t MAC_of_server_ESP[] = {0xC8, 0xC9, 0xA3, 0x5B, 0x9F, 0xF1};
// Board 3 (sensor)
uint8_t MAC_of_ESP_leftSide_road[] = {0xC8, 0xC9, 0xA3, 0x5D, 0xA6, 0xFC};


// Variable that this ESP gets from server ESP
int received_button_state = 0;
int counter_test = 0;
bool car_on_road_received = true; // set as true for safety 

// Variable to store if sending data was successful
String success;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    if (memcmp(mac_addr, MAC_of_server_ESP, 6) == 0) {
      Serial.print("Sent car on road data to ESP_button: ");
      Serial.println(car_on_road_received); 
    }
    else if (memcmp(mac_addr, MAC_of_ESP_leftSide_road, 6) == 0) {
      Serial.print("Sent COUNTER to ESP_sensorI2C: ");
      Serial.println(counter_test); 
    }
  }
  else {
    Serial.println("Delivery fail");
  }
}
  
// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  // Check the MAC address of the sender
  if (memcmp(mac, MAC_of_server_ESP, 6) == 0) {
    // Data received from the 1st ESP (MAC_of_server_ESP)
    memcpy(&counter_test, incomingData, sizeof(counter_test));  
    Serial.print("received COUNTER from ESP_button: ");
    Serial.println(counter_test); 
    esp_now_send( MAC_of_ESP_leftSide_road, (uint8_t *) &counter_test, sizeof(counter_test) );
    received_button_state = 0; // so that button state is returned to normal state
  } 
  else if (memcmp(mac, MAC_of_ESP_leftSide_road, 6) == 0) {
    // Data received from the 2nd ESP (MAC_of_ESP_leftSide_road)
    memcpy(&car_on_road_received, incomingData, sizeof(car_on_road_received));

    Serial.print("received car on road state from ESP_sensor: ");
    Serial.println(car_on_road_received);
    esp_now_send( MAC_of_server_ESP, (uint8_t *) &car_on_road_received, sizeof(car_on_road_received) );
  } 
  else {
    // Data received from an unrecognized MAC address
  }
}

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
    for (uint8_t i=0; i<n; i++) {
      if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);     // start serial for output
  // Set device as a Wi-Fi Station and set channel
  WiFi.mode(WIFI_STA);

  int32_t channel = getWiFiChannel(WIFI_SSID);
//  int32_t channel = getWiFiChannel(WIFI_SSID_home_router);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  wifi_promiscuous_enable(1);
  wifi_set_channel(channel);
  wifi_promiscuous_enable(0);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after
  
  WiFi.disconnect();
  
  // Init ESP-NOW
  if (esp_now_init() != 0) {
//    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(MAC_of_server_ESP, ESP_NOW_ROLE_COMBO, channel, NULL, 0);
  esp_now_add_peer(MAC_of_ESP_leftSide_road, ESP_NOW_ROLE_COMBO, channel, NULL, 0);

  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
}
