// Import required libraries
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include "ESPAsyncTCP.h"
#include <espnow.h>

constexpr char WIFI_SSID[] = "TEST Looking for Job";

uint8_t MAC_of_new_ESP[] = {0x24, 0xA1, 0x60, 0x2C, 0x37, 0xC5};

// constants won't change. They're used here to set pin numbers:
const int buttonPin = 14;     // the number of the pushbutton pin. D5 on node12E
const int ledPin = 12;
bool led_state = false;

// Define variables to be sent to other ESPs
int button_state = 0; 
int counter = 0;
bool car_on_road_received = true; // set as true for safety 


// Variable to store if sending data was successful
String success;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    if (memcmp(mac_addr, MAC_of_new_ESP, 6) == 0) {
      Serial.print("Sent COUNTER to ESP_repeater: ");
      Serial.println(counter); 
    }
//    else if (memcmp(mac_addr, MAC_of_ESP_leftSide_road, 6) == 0) {
//      Serial.print("Sent button state to ESP_sensorI2C: ");
//      Serial.println(button_state); 
//    }
  }
  else {
    Serial.println("Delivery fail");
    car_on_road_received = true; // reset as true for safety 
  }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  if (memcmp(mac, MAC_of_new_ESP, 6) == 0) {
      memcpy(&car_on_road_received, incomingData, sizeof(car_on_road_received));
      Serial.print("received car data from ESP_repeater: ");
      Serial.println(car_on_road_received);
  }
//  memcpy(&array_sensor_readings, incomingData, sizeof(array_sensor_readings));
//  Serial.print("Bytes received: "); // It shows 4 bytes - correct for int data type. But I want to receive sensor data only when button pressed
//  Serial.println(len);
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
  WiFi.mode(WIFI_STA);
  int32_t channel = getWiFiChannel(WIFI_SSID);

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
//  esp_now_add_peer(MAC_of_ESP_leftSide_road, ESP_NOW_ROLE_COMBO, channel, NULL, 0);
  esp_now_add_peer(MAC_of_new_ESP, ESP_NOW_ROLE_COMBO, channel, NULL, 0);
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // setup the Output LED for initial state

}

void loop() {
  // put your main code here, to run repeatedly:
//  button_state = digitalRead(buttonPin);
  button_state = 1;
//     Serial.print("             BUTTON STATE value: ");
//     Serial.println(button_state); 
  if (button_state == 1) {
     counter++;
     digitalWrite(ledPin, HIGH);
     esp_now_send(MAC_of_new_ESP, (uint8_t *) &counter, sizeof(counter));
     delay(1000);
  }
}
