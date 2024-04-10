#include <ESP8266WiFi.h>
#include <espnow.h>
#include <SoftwareSerial.h>


SoftwareSerial MySerial(5,4); //Serial1 connected to ESP_button Serial：5(D1):RX, 4 (D2):TX

constexpr char WIFI_SSID[] = "Button Looking for Job";

uint8_t MAC_of_repeater_left_side[] = {0x5C, 0xCF, 0x7F, 0xD0, 0x45, 0xB7};
uint8_t MAC_of_repeater_right_side[] = {0x24, 0xA1, 0x60, 0x2C, 0x37, 0xC5};

int received_button_state = 0;
bool car_on_left_road_received= true; // set as true for safety 
bool car_on_right_road_received= true; // set as true for safety 
//bool car_on_road_received = true; // to be sent to ESP_with_button in case no cars on both sides

// Variable to store if sending data was successful
String success;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  if (sendStatus == 0){
    if (memcmp(mac_addr, MAC_of_repeater_left_side, 6) == 0) {
      Serial.print("Sent button state to ESP_left_repeater: ");
      Serial.println(received_button_state); 
    }
    else if (memcmp(mac_addr, MAC_of_repeater_right_side, 6) == 0) {
      Serial.print("Sent button state to ESP_right_repeater: ");
      Serial.println(received_button_state); 
    }
  }
  else {
    Serial.println("Delivery fail");
    car_on_left_road_received = true; // reset as true for safety
    car_on_right_road_received = true; // reset as true for safety 
  }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  if (memcmp(mac, MAC_of_repeater_left_side, 6) == 0) {
      memcpy(&car_on_left_road_received, incomingData, sizeof(car_on_left_road_received));
      Serial.print("received car data from ESP_left_repeater: ");
      Serial.println(car_on_left_road_received);
  }
  else if (memcmp(mac, MAC_of_repeater_right_side, 6) == 0) {
      memcpy(&car_on_right_road_received, incomingData, sizeof(car_on_right_road_received));
      Serial.print("received car data from ESP_right_repeater: ");
      Serial.println(car_on_right_road_received);
  }
}

// To connect all devices in ESP NOW to the same channel
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
  // initialize both serial ports:
  MySerial.begin(115200);
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  wifi_promiscuous_enable(1);
  wifi_set_channel(channel);
  wifi_promiscuous_enable(0);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after
  
  WiFi.disconnect();
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(MAC_of_repeater_left_side, ESP_NOW_ROLE_COMBO, channel, NULL, 0);
  esp_now_add_peer(MAC_of_repeater_right_side, ESP_NOW_ROLE_COMBO, channel, NULL, 0);

  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);

}

void loop() {
  // put your main code here, to run repeatedly:
  
  if (MySerial.available() > 0){
    received_button_state = MySerial.read();
    Serial.print("check rx_button_state: ");
    Serial.println(received_button_state);
  }
  if (received_button_state > 0) {
    // NULL means send it to all peers
    esp_now_send( NULL, (uint8_t *) &received_button_state, sizeof(received_button_state) );
    received_button_state = 0; // reset
  }
  // sending false if no cars on both sides of a road
//  if (car_on_left_road_received == false && car_on_right_road_received == false) {
    if (car_on_right_road_received == false) {
    // passing the value to ESP_with_button
      MySerial.write(false);
      // reset 
      car_on_left_road_received = true;
      car_on_right_road_received = true;
  }
}
