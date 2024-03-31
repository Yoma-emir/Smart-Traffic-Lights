/*
  Code for ESP that is a server and sends buttonpress data to other ESPs and receives radar data from them.
*/

#include <ESP8266WiFi.h>
#include <espnow.h>
#include "esp_wifi_types.h"

// REPLACE WITH THE MAC Address of your receiver . Board 3 (with sensor on left side road)
uint8_t MAC_of_ESP_leftSide_road[] = {0xC8, 0xC9, 0xA3, 0x5D, 0xA6, 0xFC};

// constants won't change. They're used here to set pin numbers:
const int buttonPin = 14;     // the number of the pushbutton pin. D5 on node12E
//const int ledPin =  5;      // the number of the LED pin. D1 on node12E

// Define variables to be sent to other ESPs
int button_state = 0; 

// Define variables to store incoming readings. Must match the receiver structure
bool car_on_road_received = true; // set as true for safety 
int rssi; // received signal strength
// Test variable
//bool array_sensor_readings[36];

// Variable to store if sending data was successful
String success;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
    car_on_road_received = true; // reset as true for safety 
  }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&car_on_road_received, incomingData, sizeof(car_on_road_received));

//  memcpy(&array_sensor_readings, incomingData, sizeof(array_sensor_readings));
  Serial.print("Bytes received: "); // It shows 4 bytes - correct for int data type. But I want to receive sensor data only when button pressed
  Serial.println(len);
}

// Structures for calculating packets, RSSI, etc.
typedef struct {
  unsigned frame_ctrl: 16;
  unsigned duration_id: 16;
  uint8_t addr1[6]; /* receiver address */
  uint8_t addr2[6]; /* sender address */
  uint8_t addr3[6]; /* filtering address */
  unsigned sequence_ctrl: 16;
  uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

//La callback que hace la magia
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
  // All espnow traffic uses action frames which are a subtype of the mgmnt frames so filter out everything else.
  if (type != WIFI_PKT_MGMT)
    return;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  rssi = ppkt->rx_ctrl.rssi;
  
}

void printIncomingReadings(){
  // Display Readings in Serial Monitor
  Serial.println("INCOMING READINGS");
  Serial.print("Car on road? : ");
  Serial.println(car_on_road_received);
  
  Serial.print("RSSI: "); Serial.println(rssi);
//  int ii = 0;
//  for (ii = 0; ii< 36; ii++){
//    Serial.print(ii);
//    Serial.print(": ");
//    Serial.println(array_sensor_readings[ii]);
//  }
//  Serial.println("END of READINGS");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
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
  esp_now_add_peer(MAC_of_ESP_leftSide_road, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);

  //CB for RSSI
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);


  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  // initialize the LED pin as an output:
//  pinMode(ledPin, OUTPUT);
}
 
void loop() {
  button_state = digitalRead(buttonPin);
  if (button_state == 1) {
    // Send message via ESP-NOW
    // @attention 2. If peer_addr is NULL, send data to all of the peers that are added to the peer list
    esp_now_send(MAC_of_ESP_leftSide_road, (uint8_t *) &button_state, sizeof(button_state));

    // Print incoming readings
    printIncomingReadings();
    delay(100); // so that this ESP sends only at button trigger, not On.
    button_state = 0;
  }
//  delay(1000); // Wasn't working because I was pressing the button when the program was working on delay function.
// FOR NEXT TIME, when delivery Fail, reset the last received distance value.
}
