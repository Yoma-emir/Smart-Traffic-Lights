// Import required libraries
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include "ESPAsyncTCP.h"
#include <espnow.h>

// Set your access point network credentials
const char* ssid = "TEST Looking for Job";
const char* password = "123456789";
const char* ssid_for_browser = "Button_2 Looking for Job";
constexpr char WIFI_SSID[] = "TEST Looking for Job";

//const char* ssid_home_router = "...";
//const char* password_home_router = "11111111";

// REPLACE WITH THE MAC Address of your receiver . Board 3 (with sensor on left side road)
//uint8_t MAC_of_ESP_leftSide_road[] = {0xC8, 0xC9, 0xA3, 0x5D, 0xA6, 0xFC};
//uint8_t MAC_of_ESP_leftSide_road_repeater[] = {0xC8, 0xC9, 0xA3, 0x61, 0xAF, 0xAC};
uint8_t MAC_of_new_ESP[] = {0x24, 0xA1, 0x60, 0x2C, 0x37, 0xC5};

// constants won't change. They're used here to set pin numbers:
const int buttonPin = 14;     // the number of the pushbutton pin. D5 on node12E
const int ledPin = 12;
bool led_state = false;

// Variables for Debouncing of the button
//int counter_button = 0;       // how many times we have seen new value
//int reading_button;           // the current value read from the input pin
//int current_state_button = LOW;    // the debounced input value
//
//// the following variable is a long because the time, measured in milliseconds,
//// will quickly become a bigger number than can be stored in an int.
//long time_button = 0;         // the last time the output pin was sampled
//int debounce_count = 10; // number of millis/samples to consider before declaring a debounced input


// Define variables to be sent to other ESPs
int button_state = 0; 

int counter;  // to count up 15 sec of green cycle after which button request car_on_road data
// Define variables to store incoming readings. Must match the receiver structure
bool car_on_road_received = true; // set as true for safety 

int ldrPin = A0;              // LDR pin
int ldrVal = 0;               // Value of LDR
unsigned long startmillis_for_amber;
unsigned long currentmillis_for_amber;
const unsigned long amber_phase_duration = 5000;
int counter_for_amber_simul;
const unsigned long amber_count_duration = 5;
unsigned long startmillis_TL;
unsigned long currentmillis_TL;
const unsigned long TL_reading_interval = 1000;
const int red_phase_duration = 30;
int red_phase_counter;
bool follow_amber_simul_spec;
// variables to send events only once at the beginning of each phase.
bool send_event_green = true;
bool send_event_amber = true;
bool send_event_red = true;
bool send_event_amber_special = true;
bool send_event_red_special = true;
bool block_overlay_of_redspecial_by_ambersimul = false;

// Variable to store if sending data was successful
String success;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    if (memcmp(mac_addr, MAC_of_new_ESP, 6) == 0) {
      Serial.print("Sent button state to ESP_repeater: ");
      Serial.println(button_state); 
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

//void remove_effect_of_button_debounce () {
//  // If we have gone on to the next millisecond
//  if(millis() != time)
//  {
//    reading = digitalRead(inPin);
//
//    if(reading == current_state && counter > 0)
//    {
//      counter--;
//    }
//    if(reading != current_state)
//    {
//       counter++; 
//    }
//    // If the Input has shown the same value for long enough let's switch it
//    if(counter >= debounce_count)
//    {
//      counter = 0;
//      current_state = reading;
//      digitalWrite(outPin, current_state);
//    }
//    time = millis();
//  }
//}

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

AsyncWebServer server(80);
AsyncEventSource events("/events");

// HTML content which includes the SVG file (simplified for clarity)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<body>
  <!-- Your SVG content goes here -->
  <?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!-- Created with Inkscape (http://www.inkscape.org/) -->

  <svg
     width="210mm"
     height="297mm"
     viewBox="0 0 210 297"
     version="1.1"
     id="svg1"
     inkscape:version="1.3.2 (091e20e, 2023-11-25)"
     sodipodi:docname="trial_onlyTL_to_check_EventsDisconnected.svg"
     xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
     xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
     xmlns:xlink="http://www.w3.org/1999/xlink"
     xmlns="http://www.w3.org/2000/svg"
     xmlns:svg="http://www.w3.org/2000/svg">
    <sodipodi:namedview
       id="namedview1"
       pagecolor="#ffffff"
       bordercolor="#000000"
       borderopacity="0.25"
       inkscape:showpageshadow="2"
       inkscape:pageopacity="0.0"
       inkscape:pagecheckerboard="0"
       inkscape:deskcolor="#d1d1d1"
       inkscape:document-units="mm"
       inkscape:zoom="1.0649093"
       inkscape:cx="369.98456"
       inkscape:cy="536.1959"
       inkscape:window-width="1440"
       inkscape:window-height="783"
       inkscape:window-x="0"
       inkscape:window-y="25"
       inkscape:window-maximized="0"
       inkscape:current-layer="layer3" />
    <defs
       id="defs1">
      <inkscape:path-effect
         effect="powerstroke"
         id="path-effect135"
         is_visible="true"
         lpeversion="1.3"
         scale_width="1"
         interpolator_type="CubicBezierJohan"
         interpolator_beta="0.2"
         start_linecap_type="zerowidth"
         end_linecap_type="zerowidth"
         offset_points="0,4.9921382"
         linejoin_type="extrp_arc"
         miter_limit="4"
         not_jump="false"
         sort_points="true"
         message="&lt;b&gt;Ctrl + click&lt;/b&gt; on existing node and move it" />
      <rect
         x="79.570266"
         y="350.51206"
         width="166.19107"
         height="20.144371"
         id="rect134" />
      <rect
         x="332.38213"
         y="285.04285"
         width="51.368147"
         height="27.194901"
         id="rect133" />
      <rect
         x="237.70358"
         y="289.07173"
         width="85.613578"
         height="22.158807"
         id="rect132" />
      <rect
         x="256.84073"
         y="310.22332"
         width="172.23437"
         height="48.346491"
         id="rect131" />
      <rect
         x="255.83351"
         y="300.15113"
         width="179.2849"
         height="49.35371"
         id="rect130" />
      <inkscape:perspective
         sodipodi:type="inkscape:persp3d"
         inkscape:vp_x="91.301445 : 177.80178 : 1"
         inkscape:vp_y="54.825188 : 272.7104 : 1"
         inkscape:vp_z="24.157826 : 204.12323 : 1"
         inkscape:persp3d-origin="138.1189 : 151.07489 : 1"
         id="perspective46" />
      <inkscape:perspective
         sodipodi:type="inkscape:persp3d"
         inkscape:vp_x="-48.798144 : 153.16353 : 1"
         inkscape:vp_y="19.766518 : 933.26604 : 0"
         inkscape:vp_z="245.318 : 265.74455 : 1"
         inkscape:persp3d-origin="97.281481 : 163.25737 : 1"
         id="perspective29" />
      <symbol
         id="walking"
         viewBox="0 0 250.4117 516.91193">
        <title
           id="title57">Walking</title>
        <path
           d="m 160.20234,0.01008599 c -1.33856,0.02806 -2.69009,0.113188 -4.04883,0.238281 C 116.02114,3.051553 100.7586,57.402605 133.38984,80.740563 164.74408,104.76789 211.55927,76.018333 204.0832,37.045242 200.53319,16.525666 184.28098,1.726533 164.17109,0.11555499 c -1.30458,-0.104509 -2.63018,-0.133526 -3.96875,-0.105469 z M 128.10664,95.332353 a 50.010353,50.010353 0 0 0 -2.02539,0.0332 50.010353,50.010353 0 0 0 -17.96875,4.12695 21.972204,21.972204 0 0 0 -7.68164,3.789057 l -69.568361,52.08008 a 21.972204,21.972204 0 0 0 -7.91016,11.39062 l -21.9707,74.73438 a 21.972204,21.972204 0 1 0 42.15625,12.39258 l 19.9082,-67.72266 7.83594,-5.86523 -11.30664,63.00781 a 30.397128,30.397128 0 0 0 -1.61719,10.78906 l 2.19336,117.72266 a 30.397128,30.397128 0 0 0 0.0684,1.25586 l -46.37699,100.14454 a 30.536054,30.536054 0 1 0 55.41406,25.66211 L 119.7082,389.92415 a 30.536054,30.536054 0 0 0 1.13672,-23.75586 l -0.99023,-53.13477 41.86132,57.76563 a 30.397128,30.397128 0 0 0 2.28516,2.82617 l 13.70312,113.5586 a 27.835951,27.835951 0 1 0 55.26563,-6.66993 L 217.61445,353.28548 a 27.835951,27.835951 0 0 0 -1.16601,-5.52344 30.397128,30.397128 0 0 0 -5.50977,-12.63281 l -53.08789,-73.26172 8.16211,-45.49219 a 21.972204,21.972204 0 0 0 1.45898,1.41797 l 46.38282,40.01563 a 21.972204,21.972204 0 1 0 28.70312,-33.26954 l -42.54492,-36.70312 -23.875,-55.66406 A 50.010353,50.010353 0 0 0 128.10664,95.332353 Z"
           id="path58" />
      </symbol>
      <symbol
         id="ToiletsMen">
        <title
           id="title78">Toilets - Men</title>
        <g
           style="stroke:none"
           id="g78">
          <circle
             cx="36"
             cy="9"
             r="5"
             id="circle78" />
          <path
             d="m 24,22 v 15.5 a 2.25,2.25 0 0 0 4.5,0 V 23 h 1 v 39.5 a 3,3 0 0 0 6,0 V 40 h 1 v 22.5 a 3,3 0 0 0 6,0 V 23 h 1 v 14.5 a 2.25,2.25 0 0 0 4.5,0 V 22 A 7,7 0 0 0 41,15 H 31 a 7,7 0 0 0 -7,7 z"
             id="path78" />
        </g>
      </symbol>
      <inkscape:perspective
         sodipodi:type="inkscape:persp3d"
         inkscape:vp_x="-22.083994 : 147.4927 : 1"
         inkscape:vp_y="17.872877 : 905.42263 : 0"
         inkscape:vp_z="243.8557 : 256.71492 : 1"
         inkscape:persp3d-origin="110.00117 : 157.28539 : 1"
         id="perspective29-6" />
      <inkscape:perspective
         sodipodi:type="inkscape:persp3d"
         inkscape:vp_x="-17.417145 : 150.18154 : 1"
         inkscape:vp_y="17.323144 : 918.74765 : 0"
         inkscape:vp_z="240.34279 : 261.01122 : 1"
         inkscape:persp3d-origin="110.60531 : 160.11838 : 1"
         id="perspective29-4" />
      <inkscape:perspective
         sodipodi:type="inkscape:persp3d"
         inkscape:vp_x="94.909714 : 178.98864 : 1"
         inkscape:vp_y="61.412566 : 268.75622 : 1"
         inkscape:vp_z="33.249885 : 203.88431 : 1"
         inkscape:persp3d-origin="137.90347 : 153.70949 : 1"
         id="perspective46-1" />
      <rect
         x="79.570267"
         y="350.51205"
         width="144.03226"
         height="19.137152"
         id="rect134-2" />
      <rect
         x="79.570267"
         y="350.51205"
         width="166.19107"
         height="20.144371"
         id="rect134-8" />
      <rect
         x="79.570267"
         y="350.51205"
         width="144.03226"
         height="19.137152"
         id="rect134-2-6" />
      <inkscape:path-effect
         effect="fill_between_many"
         method="bsplinespiro"
         autoreverse="false"
         close="false"
         join="false"
         linkedpaths="#path135,0,1"
         id="path-effect136" />
    </defs>
    <g
       inkscape:groupmode="layer"
       id="layer4"
       inkscape:label="Zebra">
      <rect
         style="display:inline;fill:#999999;fill-opacity:1;fill-rule:nonzero;stroke:#ffffff;stroke-width:0.935073;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="rect41"
         width="12.28637"
         height="1.886323"
         x="-68.317383"
         y="181.08809"
         transform="matrix(0.74234283,-0.67002025,0.69013139,0.72368407,0,0)" />
      <rect
         style="display:inline;fill:#999999;fill-opacity:1;fill-rule:nonzero;stroke:#ffffff;stroke-width:0.935073;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="rect41-1"
         width="12.28637"
         height="1.886323"
         x="-68.645607"
         y="174.25076"
         transform="matrix(0.74234283,-0.67002025,0.69013139,0.72368407,0,0)" />
      <rect
         style="display:inline;fill:#999999;fill-opacity:1;fill-rule:nonzero;stroke:#ffffff;stroke-width:0.935073;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="rect41-7"
         width="12.28637"
         height="1.886323"
         x="-68.552864"
         y="167.54546"
         transform="matrix(0.74234283,-0.67002025,0.69013139,0.72368407,0,0)" />
      <rect
         style="display:inline;fill:#999999;fill-opacity:1;fill-rule:nonzero;stroke:#ffffff;stroke-width:0.935073;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="rect41-4"
         width="12.28637"
         height="1.886323"
         x="-68.856865"
         y="161.51787"
         transform="matrix(0.74234283,-0.67002025,0.69013139,0.72368407,0,0)" />
      <rect
         style="display:inline;fill:#999999;fill-opacity:1;fill-rule:nonzero;stroke:#ffffff;stroke-width:0.935073;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="rect41-0"
         width="12.28637"
         height="1.886323"
         x="-69.150154"
         y="154.8239"
         transform="matrix(0.74234283,-0.67002025,0.69013139,0.72368407,0,0)" />
    </g>
    <g
       inkscape:label="Layer 1"
       inkscape:groupmode="layer"
       id="layer1">
      <use
         xlink:href="#walking"
         width="250.4117"
         height="516.91193"
         style="display:inline;fill:#00ff00;fill-opacity:1;fill-rule:nonzero;stroke:none;stroke-width:1.403;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="use1"
         transform="matrix(-0.02698684,-0.01205792,0,0.02316762,92.623442,176.31807)" />
      <text
         xml:space="preserve"
         transform="scale(0.26458333)"
         id="text131"
         style="white-space:pre;shape-inside:url(#rect132);shape-padding:4.02887;display:inline;fill:#ff0000;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:2.6948;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"><tspan
           x="241.73242"
           y="304.04953"
           id="tspan2"><tspan
             style="fill:#00ff00"
             id="tspan1">Time saved:</tspan></tspan></text>
      <text
         xml:space="preserve"
         transform="matrix(0.26458333,0,0,0.26458333,-0.26649324,2.131946)"
         id="text132"
         style="white-space:pre;shape-inside:url(#rect133);display:inline;fill:#00ff00;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:2.6948;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         inkscape:label="time_saved_text"><tspan
           x="332.38281"
           y="295.99093"
           id="tspan3">0 sec</tspan></text>
    </g>
    <g
       inkscape:groupmode="layer"
       id="layer3"
       inkscape:label="Traffic_lights">
      <ellipse
         style="fill-opacity:1;fill-rule:nonzero;stroke:#031d19;stroke-width:0.762747;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers;display:inline"
         id="circle_red"
         cx="-97.104622"
         cy="125.33957"
         rx="2.2238986"
         ry="2.4740887"
         transform="matrix(-0.9817944,-0.18994676,0,1,0,0)"
         inkscape:label="circle_red" />
      <ellipse
         style="display:inline;fill-opacity:1;fill-rule:nonzero;stroke:#031d19;stroke-width:0.762747;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="circle_red_simul"
         cx="-110.38535"
         cy="128.70204"
         rx="2.2238986"
         ry="2.4740887"
         transform="matrix(-0.98179439,-0.18994676,0,1,0,0)"
         inkscape:label="circle_red_simul" />
      <ellipse
         style="fill-opacity:1;fill-rule:nonzero;stroke:#031d19;stroke-width:0.762747;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers;display:inline"
         id="circle_amber"
         cx="-96.974098"
         cy="131.62682"
         rx="2.2238986"
         ry="2.4740887"
         transform="matrix(-0.9817944,-0.18994676,0,1,0,0)"
         inkscape:label="circle_amber" />
      <ellipse
         style="display:inline;fill-opacity:1;fill-rule:nonzero;stroke:#031d19;stroke-width:0.762747;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="circle_amber_simul"
         cx="-110.34641"
         cy="134.54071"
         rx="2.2238986"
         ry="2.4740887"
         transform="matrix(-0.98179439,-0.18994676,0,1,0,0)"
         inkscape:label="circle_amber_simul" />
      <ellipse
         style="fill-opacity:1;fill-rule:nonzero;stroke:#031d19;stroke-width:0.762747;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="circle_green"
         cx="-96.888443"
         cy="137.4662"
         rx="2.2238986"
         ry="2.4740887"
         transform="matrix(-0.9817944,-0.18994676,0,1,0,0)"
         inkscape:label="circle_green" />
      <ellipse
         style="fill-opacity:1;fill-rule:nonzero;stroke:#031d19;stroke-width:0.762747;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="circle_green_simul"
         cx="-110.21696"
         cy="140.27783"
         rx="2.2238986"
         ry="2.4740887"
         transform="matrix(-0.98179439,-0.18994676,0,1,0,0)"
         inkscape:label="circle_green_simul" />
      <ellipse
         style="fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0.814734;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         id="path122"
         cx="57.277195"
         cy="128.64935"
         rx="1.1384383"
         ry="1.3435811"
         inkscape:label="circle_green_pedestr" />
      <ellipse
         style="fill:#1a1a1a;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0.814734;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         id="path122-4"
         cx="63.545879"
         cy="126.91283"
         rx="1.1384383"
         ry="1.3435811"
         inkscape:label="circle_green_pedestr" />
      <ellipse
         style="fill:#1a1a1a;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0.738612;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         id="path123"
         cx="57.163628"
         cy="124.19068"
         rx="1.20451"
         ry="1.3013005"
         inkscape:label="circle_red_pedestr" />
      <ellipse
         style="fill:#1a1a1a;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0.738612;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         id="path123-9"
         cx="63.499241"
         cy="122.79148"
         rx="1.20451"
         ry="1.3013005"
         inkscape:label="circle_red_pedestr" />
      <use
         xlink:href="#ToiletsMen"
         style="stroke:#000000"
         id="use1_red"
         transform="matrix(0.08892048,0,0,0.03288827,54.078709,123.06937)" />
      <use
         xlink:href="#ToiletsMen"
         style="stroke:#000000"
         id="use1_red_simul"
         transform="matrix(0.08892048,0,0,0.03288827,60.421588,121.61588)"
         inkscape:label="use1_red_simul" />
      <use
         xlink:href="#walking"
         width="250.4117"
         height="516.91193"
         style="stroke:none"
         id="use1_green"
         transform="matrix(0.0093475,0,0,0.00482014,56.151545,127.42919)" />
      <use
         xlink:href="#walking"
         width="250.4117"
         height="516.91193"
         style="stroke:none"
         id="use1_green_simul"
         transform="matrix(0.0093475,0,0,0.00482014,62.366482,125.71112)"
         inkscape:label="use1_green_simul" />
    </g>
  </svg>
  <!-- You will insert the SVG code directly here -->
  
  <script>
       // Initial values
    let initial_a = -0.02698684;
    let initial_b = -0.01205792;
    let initial_c = 0;
    let initial_d = 0.02316762;
    let initialTranslateX = 92;
    let initialTranslateY = 176;
    let initialTranslateX_reset = 92;  // to reset when green switches on
    let initialTranslateY_reset = 176;
    let moveTranslateX = -2.3;
    let moveTranslateY = -2;
    
    if (!!window.EventSource) {
    // Cache frequently accessed DOM elements
    var svgElements = {
        "circle_green": document.getElementById("circle_green"),
        "circle_amber": document.getElementById("circle_amber"),
        "circle_red": document.getElementById("circle_red"),
        "circle_green_simul": document.getElementById("circle_green_simul"),
        "circle_amber_simul": document.getElementById("circle_amber_simul"),
        "circle_red_simul": document.getElementById("circle_red_simul"),
        "use1_red": document.getElementById("use1_red"),
        "use1_green": document.getElementById("use1_green"),
        "use1_red_simul": document.getElementById("use1_red_simul"),
        "use1_green_simul": document.getElementById("use1_green_simul"),
        "use1": document.getElementById("use1")
    };

    var source = new EventSource('/events');

    // Event listeners
    source.addEventListener('open', function (e) {
        console.log("Events Connected");
    }, false);

    source.addEventListener('error', function (e) {
        if (e.target.readyState !== EventSource.OPEN) {
            console.log("Events Disconnected");
        }
    }, false);

    source.addEventListener('message', function (e) {
        console.log("message", e.data);
    }, false);

    source.addEventListener('real_TL', function (e) {
        handleTrafficLightEvent(e.data);
    }, false);

    source.addEventListener('simul_TL', function (e) {
        handleSimulatedTrafficLightEvent(e.data);
    }, false);

    // Function to handle traffic light events
    function handleTrafficLightEvent(data) {
        switch (data) {
            case "g":
                setTrafficLightColors("green");
                resetPedestrianTrafficLight();
                resetPedestrianPosition();
                break;
            case "a":
                setTrafficLightColors("amber");
                break;
            case "r":
                setTrafficLightColors("red");
                setPedestrianTrafficLight("green");
                movePedestrian();
                break;
            default:
                break;
        }
    }

    // Function to handle simulated traffic light events
    function handleSimulatedTrafficLightEvent(data) {
        switch (data) {
            case "g":
                setSimulatedTrafficLightColors("green");
                resetSimulatedPedestrianTrafficLight();
                break;
            case "a":
                setSimulatedTrafficLightColors("amber");
                break;
            case "r":
                setSimulatedTrafficLightColors("red");
                setSimulatedPedestrianTrafficLight("green");
                break;
            default:
                break;
        }
    }

    // Function to set traffic light colors
    function setTrafficLightColors(color) {
        svgElements["circle_green"].setAttribute("fill", color === "green" ? "green" : "black");
        svgElements["circle_amber"].setAttribute("fill", color === "amber" ? "orange" : "black");
        svgElements["circle_red"].setAttribute("fill", color === "red" ? "red" : "black");
    }
    
    function setPedestrianTrafficLight(color) {
        svgElements["use1_green"].setAttribute("fill", color === "green" ? "green" : "black");
        svgElements["use1_red"].setAttribute("fill", color === "red" ? "red" : "black");
    }
    
    // Function to reset pedestrian traffic light
    function resetPedestrianTrafficLight() {
        svgElements["use1_green"].setAttribute("fill", "black");
        svgElements["use1_red"].setAttribute("fill", "red");
    }

    // Function to reset pedestrian position
    function resetPedestrianPosition() {
        svgElements["use1"].setAttribute("transform", "matrix(" + initial_a + "," + initial_b + "," + initial_c + "," + initial_d + "," + initialTranslateX_reset + "," + initialTranslateY_reset + ")");
        initialTranslateX = initialTranslateX_reset;
        initialTranslateY = initialTranslateY_reset;
    }

    // Function to move pedestrian
    function movePedestrian() {
        var c = svgElements["use1"];
        initialTranslateX += moveTranslateX;
        initialTranslateY += moveTranslateY;
        c.setAttribute("transform", "matrix(" + initial_a + "," + initial_b + "," + initial_c + "," + initial_d + "," + initialTranslateX + "," + initialTranslateY + ")");
    }

    // Function to set simulated traffic light colors
    function setSimulatedTrafficLightColors(color) {
        svgElements["circle_green_simul"].setAttribute("fill", color === "green" ? "green" : "black");
        svgElements["circle_amber_simul"].setAttribute("fill", color === "amber" ? "orange" : "black");
        svgElements["circle_red_simul"].setAttribute("fill", color === "red" ? "red" : "black");
    }

    // Function to reset simulated pedestrian traffic light
    function resetSimulatedPedestrianTrafficLight() {
        svgElements["use1_green_simul"].setAttribute("fill", "black");
        svgElements["use1_red_simul"].setAttribute("fill", "red");
    }

    // Function to set simulated pedestrian traffic light
    function setSimulatedPedestrianTrafficLight(color) {
        svgElements["use1_green_simul"].setAttribute("fill", color === "green" ? "green" : "black");
        svgElements["use1_red_simul"].setAttribute("fill", color === "red" ? "red" : "black");
    }
  }


  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
//  WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_STA);
//  WiFi.begin(ssid, password);
//  WiFi.begin(ssid_home_router, password_home_router);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(1000);
//    Serial.println("Setting as a Wi-Fi Station..");
//  }

//  int32_t channel = getWiFiChannel(ssid_home_router);
  int32_t channel = getWiFiChannel(ssid);

  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel");
  Serial.println(WiFi.channel());
  
  // Start the soft AP with a different SSID and specify network settings
//  IPAddress softAP_IP(192, 168, 1, 203);
//  IPAddress softAP_gateway(192, 168, 1, 1);
//  IPAddress softAP_subnet(255, 255, 255, 0);
//  WiFi.softAPConfig(softAP_IP, softAP_gateway, softAP_subnet);
//  WiFi.softAP(ssid_for_browser, password, channel);
//  Serial.print("Soft AP IP Address: ");
//  Serial.println(WiFi.softAPIP());
//// This is the mac address of the Slave in AP Mode
//  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  
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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  
  events.onConnect([](AsyncEventSourceClient *client){
  if(client->lastId()){
    Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
  }
  
  // send event with message "hello!", id current millis
  // and set reconnect delay to 1 second
  client->send("hello!", NULL, millis(), 1000);
  // Reset colors when event source disconnects.
  send_event_green = true;  
  send_event_amber = true;
  send_event_red = true;
  send_event_amber_special = true;
  send_event_red_special = true;
  });
  server.addHandler(&events);
  server.begin();

  startmillis_for_amber = millis();       // Initialise so that code doesn't jump to Red if condition
  currentmillis_for_amber = millis();

  startmillis_TL = millis();
  counter = 0;
  counter_for_amber_simul = 0;
  red_phase_counter = red_phase_duration;
  follow_amber_simul_spec = false;
}


void loop() { 
  // turns on LED
  if (ldrVal > 100) {
     button_state = digitalRead(buttonPin);
//     Serial.print("             BUTTON STATE value: ");
//     Serial.println(button_state); 
     if (button_state == 1) {
        digitalWrite(ledPin, HIGH);
        led_state = true;
     }
//      button_state = 0; // DOESN'T IT COME FROM digitalRead(buttonPin);?? CHECK
//     delay(100); // so that this ESP sends only at button trigger, not On.
  }
  else {
     digitalWrite(ledPin, LOW);
     led_state = false;
  } 
  
  currentmillis_TL = millis();
  if ( (currentmillis_TL - startmillis_TL) > TL_reading_interval ) {
    ldrVal = analogRead(ldrPin);    // Read the analog value of the LDR
    if (counter >= 15 && led_state== true && car_on_road_received == true) { 
      button_state = 1;
      // Send message via ESP-NOW
      // @attention 2. If peer_addr is NULL, send data to all of the peers that are added to the peer list
      esp_now_send(MAC_of_new_ESP, (uint8_t *) &button_state, sizeof(button_state));
//      esp_now_send(MAC_of_new_ESP, (uint8_t *) &button_state, sizeof(button_state));
//      esp_now_send(MAC_of_ESP_leftSide_road, (uint8_t *) &button_state, sizeof(button_state));


      // Print incoming readings
      button_state = 0; //reset
    }
    Serial.print("             LDR value: ");
    Serial.println(ldrVal);         // Show the value in the serial monitor
    if (ldrVal > 100) {
      Serial.println("GREEN_real");
      Serial.println("GREEN_simul");
    
      startmillis_for_amber = millis();
      currentmillis_for_amber = millis();
      if (send_event_green) {
        events.send("g", "real_TL", millis());
        events.send("g", "simul_TL", millis());
        send_event_green = false; // to avoid sending redundant signals to web
      }
      send_event_red = true; // reset
      
      counter++;
      Serial.print("Car on road? : ");
      Serial.println(car_on_road_received);
      Serial.print("Counter: ");
      Serial.println(counter);

    }
    else if ( (currentmillis_for_amber - startmillis_for_amber) > amber_phase_duration && ldrVal < 100){
      Serial.println("RED_real");
      Serial.println("RED_simul");
      if (send_event_red) {
        events.send("r", "real_TL", millis());
        events.send("r", "simul_TL", millis());
        send_event_red = false; // to avoid sending redundant signals to web
        block_overlay_of_redspecial_by_ambersimul = false;  //reset
      }
      counter = 0; // resetting button counter
      send_event_amber = true;  // reset
    }
    else {
      Serial.println("AMBER_real");
      Serial.println("AMBER_simul");
      currentmillis_for_amber = millis();
      if (send_event_amber) {
        events.send("a", "real_TL", millis());
        if (!block_overlay_of_redspecial_by_ambersimul) {
          events.send("a", "simul_TL", millis());
        }
        send_event_amber = false; // to avoid sending redundant signals to web
      }
      send_event_green = true;  // reset
    }
    // Special case when there are no cars on the road.
    if (counter >= 15 && car_on_road_received == false && counter_for_amber_simul <= amber_count_duration ){
      Serial.println("AMBER_simul_special");
      if (send_event_amber_special) {
        events.send("a", "simul_TL", millis());
        send_event_amber_special = false;
      }
      send_event_red_special = true;  // reset
      counter_for_amber_simul++;
      red_phase_counter = red_phase_duration; // resetting red count
      follow_amber_simul_spec = true;
    }
    // this is not just "else" because it would overlay green_simul with red_simul
    else if (follow_amber_simul_spec && red_phase_counter>0 ){ 
      Serial.println("RED_simul_special");
      if (send_event_red_special) {
        events.send("r", "simul_TL", millis());
        send_event_red_special = false;
        block_overlay_of_redspecial_by_ambersimul = true; // amber_simul must not overlay red_spec phase
      }
      send_event_amber_special = true;  // reset
      counter = 0; // resetting button counter
      car_on_road_received = true; // resetting car_on_road value
      counter_for_amber_simul = 0; // resetting amber count
      red_phase_counter--;
    }

    startmillis_TL = currentmillis_TL;
  } 
  
}
