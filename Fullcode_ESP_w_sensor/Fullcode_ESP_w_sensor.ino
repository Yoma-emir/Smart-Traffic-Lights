/*
  
*/
#include <SoftwareSerial.h>
#include <CircularBuffer.hpp>
#include <ESP8266WiFi.h>
#include <espnow.h>


SoftwareSerial MySerial(5,4); //Serial1 connected to TOF Serial：5(D1):RX, 4 (D2):TX

constexpr char WIFI_SSID[] = "Button Looking for Job";

unsigned char TOF_data[32] = {0};   //store 2 TOF frames
unsigned char TOF_length = 16;
unsigned char TOF_header[3] {0x57,0x00,0xFF};
unsigned long TOF_system_time = 0;
unsigned long TOF_distance = 0;
unsigned char TOF_status = 0;
unsigned int TOF_signal = 0;
unsigned char TOF_check = 0;

bool car_on_road = true;  // set as true for safety
float sensor_ampel_dist = 50.0; // in meters
float aver_speed_of_cars = 13.8; // in m/s
unsigned int delay_betw_data_captures = 100; // in ms. 
/*Choosing the right value is critical because if while program processes delay, there is a car passing, this can lead to accident!
 The value must be tau, such that x/v< tau, where x is length of smallest car and v is average speed of cars on that street. */
//const unsigned int buffer_size = (sensor_ampel_dist/aver_speed_of_cars)/delay_betw_data_captures*1000;
const unsigned int buffer_size = 9;
CircularBuffer<bool,buffer_size> buffer; 
// Change depending on the size of the road.
long min_dist_car_on_road_true = 1000L;
long max_dist_car_on_road_true = 8000L;


/* ESP NOW related variables */
// REPLACE WITH THE MAC Address of your receiver. Board- previously Soft AP.
uint8_t MAC_of_repeater_right_side[] = {0x5C, 0xCF, 0x7F, 0xD0, 0x45, 0xB7};
// bc of battery -> 5C:CF:7F:D0:45:B7
// Variable that this ESP gets from server ESP
int received_button_state;

// Variable to store if sending data was successful
String success;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
//  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
//    Serial.println("Delivery success");
//      digitalWrite(14, HIGH); // sets the digital pin 13 on
//      delay(1000);            // waits for a second
//      digitalWrite(14, LOW);
  }
  else{
//      digitalWrite(12, HIGH); // sets the digital pin 13 on
//      delay(1000);            // waits for a second
//      digitalWrite(12, LOW);
//    Serial.println("Delivery fail");
  }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&received_button_state, incomingData, sizeof(received_button_state));
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
  // initialize both serial ports:
  MySerial.begin(115200);
  Serial.begin(115200);
  // Set device as a Wi-Fi Station
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
//    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(MAC_of_repeater_right_side, ESP_NOW_ROLE_COMBO, channel, NULL, 0);
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);

  pinMode(14, OUTPUT); // D5
  pinMode(12, OUTPUT); // D6
}

bool verifyCheckSum(unsigned char data[], unsigned char len){
  TOF_check = 0;

  for(int k=0;k<len-1;k++)
  {
      TOF_check += data[k];
  }

  if(TOF_check == data[len-1])
  {
//      Serial.println("TOF data is ok!");
      return true;    
  }else{
//      Serial.println("TOF data is error!");
      return false;  
  }
}

void loop() {
  delay(delay_betw_data_captures); // time for buffer to collect data
  if (MySerial.available()>=32) {
     for(int i=0;i<32;i++)
     {
       TOF_data[i] = MySerial.read();
     }
  
    for(int j=0;j<16;j++)
    {
      if( (TOF_data[j]==TOF_header[0] && TOF_data[j+1]==TOF_header[1] && TOF_data[j+2]==TOF_header[2]) && (verifyCheckSum(&TOF_data[j],TOF_length)))
      {
        if(((TOF_data[j+12]) | (TOF_data[j+13]<<8) )==0)
        {
//           Serial.println("Out of range!");
         }else{
//           Serial.print("TOF id is: ");
//           Serial.println(TOF_data[j+3],DEC);
//    
//           TOF_system_time = TOF_data[j+4] | TOF_data[j+5]<<8 | TOF_data[j+6]<<16 | TOF_data[j+7]<<24;
//           Serial.print("TOF system time is: ");
//           Serial.print(TOF_system_time,DEC);
//           Serial.println("ms");
    
           TOF_distance = (TOF_data[j+8]) | (TOF_data[j+9]<<8) | (TOF_data[j+10]<<16);
           Serial.print("TOF distance is: ");
           Serial.print(TOF_distance,DEC);
           Serial.println("mm");
           if (TOF_distance > min_dist_car_on_road_true && TOF_distance < max_dist_car_on_road_true) { // width of a lane is various
              buffer.unshift(true);
           }
           else {
              buffer.unshift(false);
           } 
        }
        break;
      }
    }
  }
  if (received_button_state == HIGH) {
    auto_in_zone(); // processing circular buffer
    // Send message via ESP-NOW
    // @attention 2. If peer_addr is NULL, send data to all of the peers that are added to the peer list
    esp_now_send( MAC_of_repeater_right_side, (uint8_t *) &car_on_road, sizeof(car_on_road) );
    received_button_state = 0; // so that button state is returned to normal state
  }
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
