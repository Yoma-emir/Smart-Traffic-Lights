#include <Wire.h>
#include <CircularBuffer.hpp>
#include <ESP8266WiFi.h>
#include <espnow.h>
// Change made on purpose 
#define SENSOR_ADDRESS 0x08  // Replace with the correct 7-bit address of your sensor

int boardLED_pin = 2;   // D4

bool car_on_road = true;  // set as true for safety
float sensor_ampel_dist = 50.0; // in meters
float aver_speed_of_cars = 13.8; // in m/s
unsigned int delay_betw_data_captures = 100; // in ms. 
/*Choosing the right value is critical because if while program processes delay, there is a car passing, this can lead to accident!
 The value must be tau, such that x/v< tau, where x is length of smallest car and v is average speed of cars on that street. */
//const unsigned int buffer_size = (sensor_ampel_dist/aver_speed_of_cars)/delay_betw_data_captures*1000;
const unsigned int buffer_size = 36;
CircularBuffer<bool,buffer_size> buffer; 

// Insert your SSID
constexpr char WIFI_SSID[] = "TEST Looking for Job";
constexpr char WIFI_SSID_home_router[] = "...";
/* ESP NOW related variables */
// REPLACE WITH THE MAC Address of your receiver. Board 1.
//uint8_t MAC_of_server_ESP[] = {0xC8, 0xC9, 0xA3, 0x5B, 0x9F, 0xF1};
//uint8_t MAC_of_ESP_leftSide_road_repeater[] = {0xC8, 0xC9, 0xA3, 0x61, 0xAF, 0xAC};
uint8_t MAC_of_new_ESP[] = {0x24, 0xA1, 0x60, 0x2C, 0x37, 0xC5};


// Variable that this ESP gets from server ESP
int received_button_state = 0;
int counter = 0;

// Variable to store if sending data was successful
String success;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
//  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.print("Sent car data to ESP_repeater: ");
    Serial.println(car_on_road);
  }
  else{
    Serial.print("DIDNT Send stuff");
  }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  if (memcmp(mac, MAC_of_new_ESP, 6) == 0) {
    memcpy(&counter, incomingData, sizeof(counter));
    Serial.print("received COUNTER from ESP_repeater: ");
    Serial.println(counter);
    car_on_road = 0; // TEST
    esp_now_send( MAC_of_new_ESP, (uint8_t *) &car_on_road, sizeof(car_on_road) );
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
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(MAC_of_new_ESP, ESP_NOW_ROLE_COMBO, channel, NULL, 0);
//  esp_now_add_peer(MAC_of_server_ESP, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
  
}
  
void loop() {
  
}

void auto_in_zone() {
  int ii = 0;
  for (ii = 0; ii< buffer.size(); ii++){
    if (buffer[ii] == true){
      car_on_road = true;
      break;
    } else {
      car_on_road = false;
    }
  }
}
