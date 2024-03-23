#include <ESP8266WiFi.h>
// Set your access point network credentials
const char* ssid = "TEST Looking for Job";
const char* password = "123456789";
void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  WiFi.softAP(ssid, password);
  // Test for Github
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());
}

void loop() {
  // put your main code here, to run repeatedly:

}
