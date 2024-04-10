// Import required libraries
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include "ESPAsyncTCP.h"
#include <espnow.h>
#include <SoftwareSerial.h>


SoftwareSerial MySerial(5,4); //Serial1 connected to ESP_button Serial：5(D1):RX, 4 (D2):TX

// Set your access point network credentials
const char* password = "123456789";
const char* ssid_for_browser = "Button Looking for Job";

//const char* ssid_home_router = "...";
//const char* password_home_router = "11111111";

// constants won't change. They're used here to set pin numbers:
const int buttonPin = 14;     // the number of the pushbutton pin. D5 on node12E
const int ledPin = 12;
bool led_state = false;

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
long startmillis_time_saved_special_TL;
long startmillis_time_saved_TL;
long time_saved;

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
     sodipodi:docname="ready_TL_pedestrian_TLsimul.svg"
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
       inkscape:cx="374.21028"
       inkscape:cy="484.07879"
       inkscape:window-width="1440"
       inkscape:window-height="900"
       inkscape:window-x="0"
       inkscape:window-y="0"
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
      <rect
         style="display:inline;fill:#999999;fill-opacity:1;fill-rule:nonzero;stroke:#ffffff;stroke-width:0.935073;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="rect41-78"
         width="12.28637"
         height="1.886323"
         x="-69.152443"
         y="149.07565"
         transform="matrix(0.74234283,-0.67002025,0.69013139,0.72368407,0,0)" />
      <g
         inkscape:groupmode="layer"
         id="layer4-4"
         inkscape:label="Zebra"
         transform="matrix(0.93273597,-0.06755996,0.06307257,0.99909687,39.520332,-29.494057)">
        <rect
           style="display:inline;fill:#999999;fill-opacity:1;fill-rule:nonzero;stroke:#ffffff;stroke-width:0.935073;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
           id="rect41-2"
           width="12.28637"
           height="1.886323"
           x="-68.317383"
           y="181.08809"
           transform="matrix(0.74234283,-0.67002025,0.69013139,0.72368407,0,0)" />
        <rect
           style="display:inline;fill:#999999;fill-opacity:1;fill-rule:nonzero;stroke:#ffffff;stroke-width:0.935073;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
           id="rect41-1-3"
           width="12.28637"
           height="1.886323"
           x="-68.645607"
           y="174.25076"
           transform="matrix(0.74234283,-0.67002025,0.69013139,0.72368407,0,0)" />
        <rect
           style="display:inline;fill:#999999;fill-opacity:1;fill-rule:nonzero;stroke:#ffffff;stroke-width:0.935073;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
           id="rect41-7-3"
           width="12.28637"
           height="1.886323"
           x="-68.552864"
           y="167.54546"
           transform="matrix(0.74234283,-0.67002025,0.69013139,0.72368407,0,0)" />
        <rect
           style="display:inline;fill:#999999;fill-opacity:1;fill-rule:nonzero;stroke:#ffffff;stroke-width:0.935073;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
           id="rect41-4-7"
           width="12.28637"
           height="1.886323"
           x="-68.856865"
           y="161.51787"
           transform="matrix(0.74234283,-0.67002025,0.69013139,0.72368407,0,0)" />
        <rect
           style="display:inline;fill:#999999;fill-opacity:1;fill-rule:nonzero;stroke:#ffffff;stroke-width:0.935073;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
           id="rect41-0-0"
           width="12.28637"
           height="1.886323"
           x="-69.150154"
           y="154.8239"
           transform="matrix(0.74234283,-0.67002025,0.69013139,0.72368407,0,0)" />
        <rect
           style="display:inline;fill:#999999;fill-opacity:1;fill-rule:nonzero;stroke:#ffffff;stroke-width:0.935073;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
           id="rect41-78-0"
           width="12.28637"
           height="1.886323"
           x="-69.152443"
           y="149.07565"
           transform="matrix(0.74234283,-0.67002025,0.69013139,0.72368407,0,0)" />
      </g>
    </g>
    <g
       inkscape:label="Layer 1"
       inkscape:groupmode="layer"
       id="layer1">
      <path
         style="fill:#000000;fill-opacity:1;stroke-width:0.21752;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path8"
         d="m 38.717819,221.10568 c 0.126952,-0.19701 0.241823,-0.4039 0.380856,-0.59105 2.500742,-3.36642 5.34659,-6.45527 8.1906,-9.47202 4.459995,-4.73088 6.18346,-6.34547 10.911838,-11.01763 9.853782,-9.60214 20.224139,-18.54087 30.569166,-27.50252 2.334839,-1.97072 4.635506,-3.98777 6.989172,-5.93204 0,0 -1.009563,-0.69021 -1.009563,-0.69021 v 0 c -2.290146,1.9969 -4.540323,4.04637 -6.831459,6.04202 -10.217908,9.0962 -20.520867,18.08616 -30.393957,27.62749 -4.749786,4.63434 -6.511616,6.28365 -11.027403,10.9453 -2.90172,2.99544 -5.810055,6.04855 -8.453296,9.32193 -0.146208,0.18107 -0.268697,0.38317 -0.403047,0.57475 z" />
      <path
         style="fill:#000000;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path9"
         d="m 95.504943,165.26215 c 2.40269,1.11511 4.498077,2.81332 6.611737,4.38689 5.05741,3.90652 10.00708,7.94982 15.01763,11.91592 3.6351,2.8297 7.32558,5.58556 10.89724,8.49548 1.4244,1.17332 2.86255,2.33401 4.12316,3.68578 0.20727,0.2242 0.40644,0.45557 0.60376,0.68847 0,0 0.90852,0.38489 0.90852,0.38489 v 0 c -0.20174,-0.23594 -0.40525,-0.47039 -0.61587,-0.69852 -1.28132,-1.36756 -2.73893,-2.54319 -4.18867,-3.7264 -3.59674,-2.91944 -7.30957,-5.68767 -10.97021,-8.52604 -5.00453,-3.95203 -9.94613,-7.98324 -14.97675,-11.90209 -2.59849,-1.9475 -5.215809,-3.96218 -8.190186,-5.31008 z" />
      <path
         style="fill:#000000;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path10"
         d="m 123.02974,149.0321 c 4.48903,1.20342 8.12698,4.369 11.71156,7.16364 7.14498,5.95665 14.12725,12.10535 21.18912,18.16013 3.4265,2.92928 6.81478,5.90353 10.10653,8.98381 0.22344,0.22305 0.44688,0.44609 0.67031,0.66914 0,0 0.87154,0.36071 0.87154,0.36071 v 0 c -0.22853,-0.22626 -0.45705,-0.45251 -0.68558,-0.67876 -3.34446,-3.09702 -6.77901,-6.0942 -10.25145,-9.04671 -7.08085,-6.04368 -14.07803,-12.18509 -21.20755,-18.17111 -3.96555,-3.11516 -8.23317,-6.6828 -13.2021,-8.02035 z" />
      <path
         style="fill:#000000;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path11"
         d="m 131.27577,192.67905 c 1.57446,1.04258 3.02691,2.25079 4.46591,3.46924 0.94338,0.82178 1.80929,1.72669 2.69333,2.61069 0,0 0.88177,0.36505 0.88177,0.36505 v 0 c -0.89207,-0.8857 -1.76456,-1.79331 -2.7081,-2.62496 -1.42748,-1.21026 -2.86299,-2.41813 -4.4224,-3.45792 z" />
      <path
         style="fill:#000000;fill-opacity:1;stroke-width:0.224875;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path12"
         d="m 5.9323247,196.70155 c 0.1206488,-0.20156 0.2253308,-0.41382 0.3619451,-0.60466 2.4049287,-3.35948 5.2950172,-6.45492 8.1023902,-9.45097 4.372326,-4.66617 6.944861,-7.14301 11.532269,-11.70908 9.131931,-8.88082 18.438481,-17.59746 28.317192,-25.6265 2.072952,-1.64625 4.205015,-3.21585 6.35146,-4.76213 1.154283,-0.74045 2.302251,-1.49107 3.478486,-2.19619 0.787678,-0.48624 1.562075,-0.99179 2.357661,-1.46497 0,0 -1.109376,-0.72679 -1.109376,-0.72679 v 0 c -0.787634,0.48191 -1.558414,0.98982 -2.345523,1.47243 -1.181246,0.70671 -2.340602,1.4504 -3.495844,2.19909 -2.114688,1.55815 -4.168013,3.20319 -6.202149,4.8665 -9.717203,8.18638 -18.940263,16.94846 -28.074918,25.78916 -4.939807,4.87108 -6.898414,6.74287 -11.643064,11.64519 -2.884821,2.98069 -5.848807,6.00233 -8.4032996,9.2953 -0.1445984,0.18641 -0.2571086,0.39615 -0.385663,0.59422 z" />
      <path
         style="fill:#000000;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path13"
         d="m 121.59542,147.4559 c 3.70958,-3.50664 7.02831,-7.42084 10.41943,-11.23103 7.4166,-8.38691 15.1107,-16.51808 22.98162,-24.478 2.23245,-2.30064 4.52967,-4.53636 6.81571,-6.78315 0.96959,-0.95294 1.92296,-1.91206 2.93526,-2.81937 0.1603,-0.14417 0.32061,-0.28833 0.48091,-0.43249 0,0 -0.81796,-0.50583 -0.81796,-0.50583 v 0 c -0.15555,0.14705 -0.3111,0.29409 -0.46665,0.44114 -0.98458,0.92773 -1.91243,1.90423 -2.85976,2.87002 -2.23747,2.28108 -4.4976,4.54015 -6.70099,6.85455 -7.8077,8.02078 -15.48245,16.16775 -22.97167,24.48763 -3.43961,3.77881 -6.80704,7.68156 -10.64584,11.06513 z" />
      <path
         style="fill:#000000;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path14"
         d="m 92.946232,123.41042 c 0.644295,-0.52936 0.517017,-0.40591 1.308613,-1.19861 1.315779,-1.31761 2.596751,-2.66966 3.911431,-3.98836 3.251754,-3.26172 4.168844,-4.1235 7.665914,-7.52327 9.45319,-8.92298 18.82648,-17.927282 28.04489,-27.092681 2.69966,-2.759961 5.44885,-5.471226 8.13472,-8.244613 0,0 -0.76603,-0.466542 -0.76603,-0.466542 v 0 c -2.61196,2.822549 -5.30346,5.569934 -7.96383,8.346633 -9.12189,9.256347 -18.4649,18.286493 -27.99749,27.120033 -3.44292,3.26772 -4.55985,4.31016 -7.798373,7.44344 -1.344835,1.30113 -2.661092,2.63191 -4.020452,3.91785 -0.802198,0.75887 -0.690697,0.65457 -1.349126,1.14926 z" />
      <path
         style="fill:#000000;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path15"
         d="m 92.083878,122.44196 c -2.702269,-2.02251 -5.406895,-4.03936 -8.157315,-5.99631 -4.978715,-3.48057 -10.178889,-6.6243 -15.230885,-9.99472 -3.273359,-2.20122 -6.468282,-4.5097 -9.454767,-7.088248 -0.912836,-0.787543 -1.784622,-1.616538 -2.59779,-2.506075 0,0 -0.888833,-0.372689 -0.888833,-0.372689 v 0 c 0.827024,0.899485 1.710894,1.739598 2.639142,2.535023 3.00922,2.592359 6.226315,4.914189 9.523423,7.127239 5.053719,3.36856 10.256861,6.50857 15.233501,9.99298 2.70791,1.93147 5.368991,3.92323 8.017426,5.93517 z" />
      <path
         style="fill:#4d4d4d;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path16"
         d="m 64.210882,138.82035 c -0.188336,-0.0343 -0.383874,-0.041 -0.56501,-0.10292 -1.905947,-0.6518 -3.633208,-1.7579 -5.29159,-2.87443 -3.251517,-2.18913 -3.281892,-2.27279 -6.53298,-4.68839 -6.032791,-4.76687 -11.856773,-9.79904 -17.397627,-15.12964 -2.083557,-2.03493 -4.041547,-4.19358 -5.905942,-6.42965 0,0 -0.868698,-0.36859 -0.868698,-0.36859 v 0 c 1.909453,2.24809 3.900032,4.42853 6.015434,6.48559 5.570879,5.33311 11.417866,10.37283 17.459973,15.16544 2.328847,1.73801 4.061391,3.09 6.473015,4.6714 1.980541,1.29872 4.496017,2.97105 6.819255,3.7491 0.189219,0.0634 0.39344,0.0669 0.590161,0.10041 z" />
      <path
         style="fill:#4d4d4d;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path17"
         d="m 36.287866,191.32825 c 1.922338,-1.82188 3.835116,-3.65246 5.725141,-5.50813 1.292103,-1.33808 2.675771,-2.58059 4.018066,-3.86618 0,0 -0.81815,-0.50284 -0.81815,-0.50284 v 0 c -1.316924,1.30479 -2.679409,2.56372 -3.96875,3.89691 -1.895438,1.8609 -3.809921,3.70149 -5.796815,5.46517 z" />
      <path
         style="fill:#4d4d4d;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path17-1"
         d="m 21.241308,207.53049 c 1.922338,-1.82188 3.835116,-3.65246 5.725141,-5.50813 1.292103,-1.33808 2.675771,-2.58059 4.018066,-3.86618 0,0 -0.81815,-0.50284 -0.81815,-0.50284 v 0 c -1.316924,1.30479 -2.679409,2.56372 -3.96875,3.89691 -1.895438,1.8609 -3.809921,3.70149 -5.796815,5.46517 z" />
      <path
         style="fill:#4d4d4d;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path18"
         d="m 53.037676,174.82363 c 1.258998,-1.54776 2.728293,-2.89781 4.202271,-4.23518 1.096579,-0.95784 2.197973,-1.91021 3.363579,-2.78347 0,0 -0.815951,-0.5152 -0.815951,-0.5152 v 0 c -1.150176,0.89163 -2.244505,1.85031 -3.339923,2.80793 -1.477571,1.34484 -2.945127,2.70324 -4.250708,4.21955 z" />
      <path
         style="fill:#4d4d4d;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path21"
         d="m 125.78392,114.86682 c 1.25056,-1.27536 2.5333,-2.5185 3.79269,-3.78552 1.36307,-1.30952 2.61475,-2.72389 3.88151,-4.12411 0.13393,-0.14435 0.26785,-0.2887 0.40178,-0.43305 0,0 -0.82503,-0.50045 -0.82503,-0.50045 v 0 c -0.13143,0.14624 -0.26286,0.29248 -0.39429,0.43872 -1.2498,1.41221 -2.50496,2.81871 -3.86352,4.12932 -1.26104,1.27134 -2.54078,2.52228 -3.83608,3.75853 z" />
      <path
         style="fill:#4d4d4d;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path23"
         d="m 140.48679,99.561932 c 0.92452,-1.144963 1.96168,-2.198727 3.01318,-3.226779 0.77073,-0.837427 1.74268,-1.447207 2.56351,-2.226384 0.3784,-0.493326 0.6691,-1.049086 0.9865,-1.583084 0.1616,-0.27913 0.0859,-0.146902 0.22753,-0.396457 0,0 -0.85642,-0.492718 -0.85642,-0.492718 v 0 c -0.13695,0.25041 -0.0634,0.117904 -0.22117,0.397166 -0.31088,0.52754 -0.59767,1.075815 -0.97472,1.559351 -0.80475,0.790152 -1.76283,1.413706 -2.52848,2.248726 -1.05597,1.035364 -2.09847,2.092113 -3.06124,3.214968 z" />
      <path
         style="fill:#4d4d4d;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path24"
         d="m 73.8932,128.74841 c 0.158552,0.0881 0.295328,0.28363 0.475655,0.2642 0.164632,-0.0177 -0.277715,-0.18041 -0.416608,-0.27056 -0.688591,-0.44696 -1.341821,-0.92339 -1.983761,-1.43517 -0.978154,-0.88825 -2.07014,-1.63086 -3.128597,-2.41647 -0.827926,-0.61663 -1.592498,-1.30643 -2.323468,-2.03279 -0.06506,-0.0629 -0.130125,-0.12572 -0.195186,-0.18858 0,0 -0.90974,-0.37442 -0.90974,-0.37442 v 0 c 0.0666,0.063 0.133196,0.12594 0.199795,0.18891 0.742394,0.72413 1.498827,1.43277 2.33468,2.05183 1.057101,0.78691 2.155505,1.52122 3.130122,2.41336 1.125886,0.90487 2.310796,1.72712 3.581921,2.41846 z" />
      <path
         style="fill:#4d4d4d;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path24-8"
         d="m 119.84384,164.06867 c 0.15855,0.0881 0.29533,0.28363 0.47565,0.2642 0.16464,-0.0177 -0.27771,-0.18041 -0.4166,-0.27056 -0.68859,-0.44696 -1.34182,-0.92339 -1.98376,-1.43517 -0.97816,-0.88825 -2.07014,-1.63086 -3.1286,-2.41647 -0.82793,-0.61663 -1.5925,-1.30643 -2.32347,-2.03279 -0.0651,-0.0629 -0.13012,-0.12572 -0.19519,-0.18858 0,0 -0.90974,-0.37442 -0.90974,-0.37442 v 0 c 0.0666,0.063 0.1332,0.12594 0.1998,0.18891 0.74239,0.72413 1.49883,1.43277 2.33468,2.05183 1.0571,0.78691 2.1555,1.52122 3.13012,2.41336 1.12589,0.90487 2.3108,1.72712 3.58192,2.41846 z" />
      <path
         style="fill:#4d4d4d;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path25"
         d="m 56.925188,115.18428 c -0.1149,-0.0571 -0.450702,-0.2435 -0.344702,-0.17123 0.131202,0.0895 0.439314,0.3809 0.420892,0.22317 -0.02111,-0.18078 -0.290582,-0.21931 -0.433594,-0.3319 -0.483452,-0.3806 -0.922904,-0.7932 -1.363144,-1.22231 -0.76617,-0.7478 -1.468234,-1.55969 -2.24934,-2.29192 -0.354954,-0.21322 -0.561861,-0.6264 -0.915931,-0.83783 -0.322869,-0.19281 -0.652929,-0.37331 -0.979393,-0.55996 -0.135519,-0.0326 -0.255249,-0.10509 -0.377415,-0.16909 0,0 0.774092,0.61994 0.774092,0.61994 v 0 c 0.12373,0.0608 0.248642,0.11838 0.377121,0.16859 -1.325012,-0.79051 -0.831236,-0.52554 -0.586637,-0.33309 0.194945,0.15338 0.347742,0.34498 0.531897,0.51072 0.08901,0.0801 0.183158,0.15431 0.274736,0.23146 0.794136,0.72321 1.492427,1.54473 2.26174,2.29392 1.007131,0.99338 2.061678,1.91321 3.380042,2.48574 z" />
      <path
         style="fill:#4d4d4d;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path26"
         d="m 153.2837,191.59967 c -1.51711,-0.8641 -2.12857,-1.26769 -3.79231,-2.40642 -1.50759,-0.96986 -2.8811,-2.11539 -4.18695,-3.33814 -0.53435,-0.50181 -1.06942,-0.99304 -1.65859,-1.42832 -0.10405,-0.079 -0.20207,-0.16525 -0.29851,-0.25322 0,0 -0.92006,-0.37735 -0.92006,-0.37735 v 0 c 0.0969,0.0924 0.19603,0.18257 0.30142,0.26546 0.58433,0.45128 1.16068,0.90567 1.68748,1.42892 1.31178,1.2277 2.68731,2.38393 4.2008,3.35919 1.23298,0.8477 2.48077,1.63005 3.76615,2.39752 0.11714,0.0699 -0.21651,-0.18901 -0.35131,-0.21004 -0.1282,-0.02 0.2199,0.13779 0.32985,0.20669 z" />
      <path
         style="fill:#4d4d4d;fill-opacity:1;stroke-width:0.165;stroke-dasharray:none;paint-order:stroke fill markers"
         id="path27"
         d="m 134.89753,177.12783 c -0.89791,-0.99977 -1.89018,-1.90936 -2.91521,-2.77626 -0.82983,-0.68959 -1.67002,-1.36962 -2.56045,-1.98002 -0.40101,-0.25997 -0.76832,-0.56682 -1.15351,-0.84928 0,0 -0.91915,-0.36388 -0.91915,-0.36388 v 0 c 0.39154,0.28154 0.76144,0.59097 1.16349,0.85678 0.90326,0.6014 1.74493,1.29038 2.5831,1.97808 1.02037,0.8541 2.0157,1.74921 2.88837,2.75625 z" />
      <rect
         style="display:inline;fill:#666666;fill-opacity:1;fill-rule:nonzero;stroke:#574200;stroke-width:0.938129;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="rect28"
         width="2.5418098"
         height="21.373508"
         x="-107.04801"
         y="113.3679"
         transform="matrix(-0.90397708,-0.4275809,0,1,0,0)" />
      <rect
         style="display:inline;fill:#666666;fill-opacity:1;fill-rule:nonzero;stroke:#574200;stroke-width:0.938129;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="rect28-5"
         width="2.5418098"
         height="21.373508"
         x="-121.32133"
         y="112.09499"
         transform="matrix(-0.90397709,-0.4275809,0,1,0,0)" />
      <use
         xlink:href="#walking"
         width="250.4117"
         height="516.91193"
         style="display:inline;fill:#00ff00;fill-opacity:1;fill-rule:nonzero;stroke:none;stroke-width:1.403;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         id="use1"
         transform="matrix(-0.02698684,-0.01205792,0,0.02316762,92.623442,176.31807)" />
      <path
         style="fill:#00ff00;fill-opacity:0.21411765;fill-rule:nonzero;stroke:#ffff8f;stroke-width:1.403;stroke-dasharray:none;stroke-opacity:0.08941177;paint-order:stroke fill markers"
         d="M 45.397538,215.94504 27.483914,201.7123 62.084201,166.86662 76.562333,180.1178 50.059986,204.657 Z"
         id="path46" />
      <rect
         style="fill:#1a1a1a;fill-opacity:0.9929412;fill-rule:nonzero;stroke:#ffff8f;stroke-width:1.403;stroke-dasharray:none;stroke-opacity:0.0894118;paint-order:stroke fill markers"
         id="rect47"
         width="6.8709784"
         height="3.4354892"
         x="-136.07784"
         y="174.6804"
         transform="rotate(-49.463182)" />
      <rect
         style="fill:#1a1a1a;fill-opacity:0.992941;fill-rule:nonzero;stroke:#ffff8f;stroke-width:1.403;stroke-dasharray:none;stroke-opacity:0.0894118;paint-order:stroke fill markers"
         id="rect47-4"
         width="6.8709784"
         height="3.4354892"
         x="36.502392"
         y="146.32945"
         transform="rotate(-43.637223)" />
      <rect
         style="display:inline;fill:#808080;fill-opacity:0.995294;fill-rule:nonzero;stroke:#3c0000;stroke-width:0.902999;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         id="rect122"
         width="1.2525645"
         height="12.16777"
         x="56.00753"
         y="131.51929" />
      <rect
         style="display:inline;fill:#808080;fill-opacity:0.995294;fill-rule:nonzero;stroke:#3c0000;stroke-width:0.902999;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         id="rect122-0"
         width="1.2525645"
         height="12.16777"
         x="62.203163"
         y="128.9827" />
      <path
         style="fill:#00ff00;fill-opacity:0.21647058;fill-rule:nonzero;stroke:#ffff8f;stroke-width:1.403;stroke-dasharray:none;stroke-opacity:0.0894118;paint-order:stroke fill markers"
         d="M 133.00251,80.488606 145.7629,91.285858 117.05203,122.45065 105.76399,111.6534 130.54859,87.604977 Z"
         id="path48" />
      <text
         xml:space="preserve"
         transform="scale(0.26458333)"
         id="text131"
         style="white-space:pre;shape-inside:url(#rect132);shape-padding:4.02887;display:inline;fill:#ff0000;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:2.6948;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"><tspan
           x="241.73242"
           y="304.04953"
           id="tspan4"><tspan
             style="fill:#00ff00"
             id="tspan3">Time saved:</tspan></tspan></text>
      <text
         xml:space="preserve"
         transform="matrix(0.26458333,0,0,0.26458333,-0.26649324,2.131946)"
         id="time_saved_value"
         style="white-space:pre;shape-inside:url(#rect133);display:inline;fill:#00ff00;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:2.6948;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         inkscape:label="time_saved_text"><tspan
           x="332.38281"
           y="295.99093"
           id="tspan5">0 sec</tspan></text>
      <text
         xml:space="preserve"
         transform="matrix(0.26458333,0,0,0.26458333,38.64152,18.12154)"
         id="text133"
         style="font-size:14.6667px;white-space:pre;shape-inside:url(#rect134);display:inline;fill:#000000;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"><tspan
           x="79.570312"
           y="363.89259"
           id="tspan6">Proposed traffic light</tspan></text>
      <text
         xml:space="preserve"
         transform="matrix(0.26458333,0,0,0.26458333,92.402814,74.017595)"
         id="text133-2"
         style="font-size:14.6667px;white-space:pre;shape-inside:url(#rect134-8);display:inline;fill:#000000;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"><tspan
           x="79.570312"
           y="363.89259"
           id="tspan7">Proposed traffic light</tspan></text>
      <text
         xml:space="preserve"
         transform="matrix(0.26458333,0,0,0.26458333,-4.2424127,23.783222)"
         id="text133-3"
         style="font-size:14.6667px;white-space:pre;shape-inside:url(#rect134-2);display:inline;fill:#000000;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"><tspan
           x="79.570312"
           y="363.89259"
           id="tspan8">Actual traffic light</tspan></text>
      <text
         xml:space="preserve"
         transform="matrix(0.26458333,0,0,0.26458333,81.594167,44.126426)"
         id="text133-3-2"
         style="font-size:14.6667px;white-space:pre;shape-inside:url(#rect134-2-6);display:inline;fill:#000000;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"><tspan
           x="79.570312"
           y="363.89259"
           id="tspan9">Actual traffic light</tspan></text>
    </g>
    <g
       inkscape:groupmode="layer"
       id="layer3"
       inkscape:label="Traffic_lights">
      <g
         sodipodi:type="inkscape:box3d"
         id="g29"
         style="display:inline;fill:#666666;fill-opacity:1;fill-rule:nonzero;stroke:#030000;stroke-width:1.403;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         inkscape:perspectiveID="#perspective29"
         inkscape:corner0="0.03389611 : -0.0073111163 : 0 : 1"
         inkscape:corner7="-0.0090713903 : -0.027233575 : 0.015343881 : 1"
         inkscape:label="TL_head">
        <path
           sodipodi:type="inkscape:box3dside"
           id="path35"
           style="fill:#666666;fill-rule:evenodd;stroke:#030000;stroke-width:1.15491;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:1"
           inkscape:box3dsidetype="11"
           d="m 94.589447,139.07298 6.122603,-0.2034 -0.39134,18.47705 -6.10658,-0.55325 z"
           points="100.71205,138.86958 100.32071,157.34663 94.21413,156.79338 94.589447,139.07298 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path30"
           style="fill:#666666;fill-rule:evenodd;stroke:#030000;stroke-width:1.15491;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:1"
           inkscape:box3dsidetype="6"
           d="m 92.35251,140.67308 -0.380887,17.98338 2.242507,-1.86308 0.375317,-17.7204 z"
           points="91.971623,158.65646 94.21413,156.79338 94.589447,139.07298 92.35251,140.67308 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path34"
           style="fill:#666666;fill-rule:evenodd;stroke:#030000;stroke-width:1.15491;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:1"
           inkscape:box3dsidetype="13"
           d="m 91.971623,158.65646 6.103896,0.64261 2.245191,-1.95244 -6.10658,-0.55325 z"
           points="98.075519,159.29907 100.32071,157.34663 94.21413,156.79338 91.971623,158.65646 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path31"
           style="fill:#666666;fill-rule:evenodd;stroke:#030000;stroke-width:1.15491;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:1"
           inkscape:box3dsidetype="5"
           d="m 92.35251,140.67308 6.120411,-0.13717 2.239129,-1.66633 -6.122603,0.2034 z"
           points="98.472921,140.53591 100.71205,138.86958 94.589447,139.07298 92.35251,140.67308 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path33"
           style="fill:#666666;fill-rule:evenodd;stroke:#030000;stroke-width:1.15491;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:1"
           inkscape:box3dsidetype="14"
           d="m 98.472921,140.53591 -0.397402,18.76316 2.245191,-1.95244 0.39134,-18.47705 z"
           points="98.075519,159.29907 100.32071,157.34663 100.71205,138.86958 98.472921,140.53591 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path32"
           style="fill:#666666;fill-rule:evenodd;stroke:#030000;stroke-width:1.15491;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:1"
           inkscape:box3dsidetype="3"
           d="m 92.35251,140.67308 6.120411,-0.13717 -0.397402,18.76316 -6.103896,-0.64261 z"
           points="98.472921,140.53591 98.075519,159.29907 91.971623,158.65646 92.35251,140.67308 " />
      </g>
      <g
         sodipodi:type="inkscape:box3d"
         id="g29-0"
         style="display:inline;fill:#666666;fill-opacity:1;fill-rule:nonzero;stroke:#030000;stroke-width:1.003;stroke-dasharray:none;stroke-opacity:1;paint-order:stroke fill markers"
         inkscape:perspectiveID="#perspective29-6"
         inkscape:corner0="0.03389611 : -0.0073111163 : 0 : 1"
         inkscape:corner7="-0.0090713903 : -0.027233575 : 0.015343881 : 1"
         inkscape:label="TL_head_simul">
        <path
           sodipodi:type="inkscape:box3dside"
           id="path35-4"
           style="fill:#666666;fill-rule:evenodd;stroke:#030000;stroke-width:1.003;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:1"
           inkscape:box3dsidetype="11"
           d="m 107.56703,144.88593 5.53606,-0.19733 -0.35385,17.9258 -5.52157,-0.53675 z"
           points="113.10309,144.6886 112.74924,162.6144 107.22767,162.07765 107.56703,144.88593 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path30-1"
           style="fill:#666666;fill-rule:evenodd;stroke:#030000;stroke-width:1.003;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:1"
           inkscape:box3dsidetype="6"
           d="m 105.5444,146.43829 -0.3444,17.44687 2.02767,-1.80751 0.33936,-17.19172 z"
           points="105.2,163.88516 107.22767,162.07765 107.56703,144.88593 105.5444,146.43829 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path34-0"
           style="fill:#666666;fill-rule:evenodd;stroke:#030000;stroke-width:1.003;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:1"
           inkscape:box3dsidetype="13"
           d="m 105.2,163.88516 5.51914,0.62343 2.0301,-1.89419 -5.52157,-0.53675 z"
           points="110.71914,164.50859 112.74924,162.6144 107.22767,162.07765 105.2,163.88516 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path31-8"
           style="fill:#666666;fill-rule:evenodd;stroke:#030000;stroke-width:1.003;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:1"
           inkscape:box3dsidetype="5"
           d="m 105.5444,146.43829 5.53407,-0.13307 2.02462,-1.61662 -5.53606,0.19733 z"
           points="111.07847,146.30522 113.10309,144.6886 107.56703,144.88593 105.5444,146.43829 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path33-1"
           style="fill:#666666;fill-rule:evenodd;stroke:#030000;stroke-width:1.003;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:1"
           inkscape:box3dsidetype="14"
           d="m 111.07847,146.30522 -0.35933,18.20337 2.0301,-1.89419 0.35385,-17.9258 z"
           points="110.71914,164.50859 112.74924,162.6144 113.10309,144.6886 111.07847,146.30522 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path32-4"
           style="fill:#666666;fill-rule:evenodd;stroke:#030000;stroke-width:1.003;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:1"
           inkscape:box3dsidetype="3"
           d="m 105.5444,146.43829 5.53407,-0.13307 -0.35933,18.20337 -5.51914,-0.62343 z"
           points="111.07847,146.30522 110.71914,164.50859 105.2,163.88516 105.5444,146.43829 " />
      </g>
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
      <g
         sodipodi:type="inkscape:box3d"
         id="g106"
         style="fill:#666666;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0.713;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         inkscape:perspectiveID="#perspective46"
         inkscape:corner0="-2.255579 : 0.38057108 : 0 : 1"
         inkscape:corner7="-2.3547969 : 0.29233782 : 0.040411611 : 1"
         inkscape:transform-center-x="-0.23996863"
         inkscape:transform-center-y="-1.7838327e-05"
         inkscape:label="TL_pedestrian">
        <path
           sodipodi:type="inkscape:box3dside"
           id="path108"
           style="fill:#666666;fill-rule:evenodd;stroke:#000000;stroke-width:0.889484;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="3"
           d="m 53.661048,129.93251 3.833404,-1.09321 -0.221672,-8.68246 -3.505096,0.0987 z"
           points="57.494452,128.8393 57.27278,120.15684 53.767684,120.25558 53.661048,129.93251 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path109"
           style="fill:#666666;fill-rule:evenodd;stroke:#000000;stroke-width:0.889484;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="14"
           d="m 57.494452,128.8393 -0.221672,-8.68246 1.30936,1.07865 0.354983,9.16012 z"
           points="57.27278,120.15684 58.58214,121.23549 58.937123,130.39561 57.494452,128.8393 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path107"
           style="fill:#666666;fill-rule:evenodd;stroke:#000000;stroke-width:0.889484;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="5"
           d="m 53.661048,129.93251 3.833404,-1.09321 1.442671,1.55631 -3.847513,1.33116 z"
           points="57.494452,128.8393 58.937123,130.39561 55.08961,131.72677 53.661048,129.93251 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path111"
           style="fill:#666666;fill-rule:evenodd;stroke:#000000;stroke-width:0.889484;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="11"
           d="m 55.08961,131.72677 3.847513,-1.33116 -0.354983,-9.16012 -3.517811,0.21904 z"
           points="58.937123,130.39561 58.58214,121.23549 55.064329,121.45453 55.08961,131.72677 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path106"
           style="fill:#666666;fill-rule:evenodd;stroke:#000000;stroke-width:0.889484;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="6"
           d="m 53.661048,129.93251 0.106636,-9.67693 1.296645,1.19895 0.02528,10.27224 z"
           points="53.767684,120.25558 55.064329,121.45453 55.08961,131.72677 53.661048,129.93251 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path110"
           style="fill:#666666;fill-rule:evenodd;stroke:#000000;stroke-width:0.889484;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="13"
           d="m 53.767684,120.25558 3.505096,-0.0987 1.30936,1.07865 -3.517811,0.21904 z"
           points="57.27278,120.15684 58.58214,121.23549 55.064329,121.45453 53.767684,120.25558 " />
      </g>
      <g
         sodipodi:type="inkscape:box3d"
         id="g106-2"
         style="fill:#666666;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0.713;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         inkscape:perspectiveID="#perspective46-1"
         inkscape:corner0="-2.255579 : 0.38057108 : 0 : 1"
         inkscape:corner7="-2.3547969 : 0.29233782 : 0.040411611 : 1"
         inkscape:transform-center-x="-0.83288684"
         inkscape:transform-center-y="-7.7326349e-05"
         inkscape:label="TL_pedestrian">
        <path
           sodipodi:type="inkscape:box3dside"
           id="path108-2"
           style="fill:#666666;fill-rule:evenodd;stroke:#000000;stroke-width:0.836958;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="3"
           d="m 60.343502,128.16418 3.520321,-1.03399 -0.203568,-8.21215 -3.218826,0.0934 z"
           points="63.863823,127.13019 63.660255,118.91804 60.441429,119.01144 60.343502,128.16418 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path109-2"
           style="fill:#666666;fill-rule:evenodd;stroke:#000000;stroke-width:0.836958;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="14"
           d="m 63.863823,127.13019 -0.203568,-8.21215 1.202422,1.02022 0.32599,8.66394 z"
           points="63.660255,118.91804 64.862677,119.93826 65.188667,128.6022 63.863823,127.13019 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path107-4"
           style="fill:#666666;fill-rule:evenodd;stroke:#000000;stroke-width:0.836958;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="5"
           d="m 60.343502,128.16418 3.520321,-1.03399 1.324844,1.47201 -3.533277,1.25905 z"
           points="63.863823,127.13019 65.188667,128.6022 61.65539,129.86125 60.343502,128.16418 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path111-9"
           style="fill:#666666;fill-rule:evenodd;stroke:#000000;stroke-width:0.836958;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="11"
           d="m 61.65539,129.86125 3.533277,-1.25905 -0.32599,-8.66394 -3.230504,0.20717 z"
           points="65.188667,128.6022 64.862677,119.93826 61.632173,120.14543 61.65539,129.86125 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path106-7"
           style="fill:#666666;fill-rule:evenodd;stroke:#000000;stroke-width:0.836958;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="6"
           d="m 60.343502,128.16418 0.09793,-9.15274 1.190744,1.13399 0.02322,9.71582 z"
           points="60.441429,119.01144 61.632173,120.14543 61.65539,129.86125 60.343502,128.16418 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path110-7"
           style="fill:#666666;fill-rule:evenodd;stroke:#000000;stroke-width:0.836958;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="13"
           d="m 60.441429,119.01144 3.218826,-0.0934 1.202422,1.02022 -3.230504,0.20717 z"
           points="63.660255,118.91804 64.862677,119.93826 61.632173,120.14543 60.441429,119.01144 " />
      </g>
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
      <g
         sodipodi:type="inkscape:box3d"
         id="g123"
         style="fill:#ffff00;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0.1;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         inkscape:perspectiveID="#perspective29"
         inkscape:corner0="0.0080270118 : -0.038074534 : 0 : 1"
         inkscape:corner7="0.00041049244 : -0.041104106 : 0.0051643692 : 1">
        <path
           sodipodi:type="inkscape:box3dside"
           id="path129"
           style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:0.1;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="11"
           d="m 96.135926,168.37125 1.097773,0.18583 -0.05955,2.81172 -1.097325,-0.20697 z"
           points="97.233699,168.55708 97.174147,171.3688 96.076822,171.16183 96.135926,168.37125 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path124"
           style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:0.1;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="6"
           d="m 95.37163,169.07372 -0.05941,2.80489 0.764599,-0.71678 0.0591,-2.79058 z"
           points="95.312223,171.87861 96.076822,171.16183 96.135926,168.37125 95.37163,169.07372 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path128"
           style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:0.1;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="13"
           d="m 95.312223,171.87861 1.097169,0.21349 0.764755,-0.7233 -1.097325,-0.20697 z"
           points="96.409392,172.0921 97.174147,171.3688 96.076822,171.16183 95.312223,171.87861 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path125"
           style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:0.1;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="5"
           d="m 95.37163,169.07372 1.097621,0.19214 0.764448,-0.70878 -1.097773,-0.18583 z"
           points="96.469251,169.26586 97.233699,168.55708 96.135926,168.37125 95.37163,169.07372 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path127"
           style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:0.1;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="14"
           d="m 96.469251,169.26586 -0.05986,2.82624 0.764755,-0.7233 0.05955,-2.81172 z"
           points="96.409392,172.0921 97.174147,171.3688 97.233699,168.55708 96.469251,169.26586 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path126"
           style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:0.1;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="3"
           d="m 95.37163,169.07372 1.097621,0.19214 -0.05986,2.82624 -1.097169,-0.21349 z"
           points="96.469251,169.26586 96.409392,172.0921 95.312223,171.87861 95.37163,169.07372 " />
      </g>
      <g
         sodipodi:type="inkscape:box3d"
         id="g123-4"
         style="fill:#ffff00;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0.05;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         inkscape:perspectiveID="#perspective29-4"
         inkscape:corner0="0.0080270118 : -0.038074534 : 0 : 1"
         inkscape:corner7="0.00041049244 : -0.041104106 : 0.0051643692 : 1">
        <path
           sodipodi:type="inkscape:box3dside"
           id="path129-4"
           style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:0.0577385;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="11"
           d="m 109.60136,170.97153 0.96208,0.18295 -0.0522,2.76798 -0.96168,-0.20375 z"
           points="110.56344,171.15448 110.51124,173.92246 109.54956,173.71871 109.60136,170.97153 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path124-7"
           style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:0.0577385;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="6"
           d="m 108.93154,171.66308 -0.0521,2.76125 0.67008,-0.70562 0.0518,-2.74718 z"
           points="108.87948,174.42433 109.54956,173.71871 109.60136,170.97153 108.93154,171.66308 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path128-9"
           style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:0.0577385;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="13"
           d="m 108.87948,174.42433 0.96154,0.21018 0.67022,-0.71205 -0.96168,-0.20375 z"
           points="109.84102,174.63451 110.51124,173.92246 109.54956,173.71871 108.87948,174.42433 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path125-5"
           style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:0.0577385;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="5"
           d="m 108.93154,171.66308 0.96194,0.18916 0.66996,-0.69776 -0.96208,-0.18295 z"
           points="109.89348,171.85224 110.56344,171.15448 109.60136,170.97153 108.93154,171.66308 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path127-5"
           style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:0.0577385;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="14"
           d="m 109.89348,171.85224 -0.0525,2.78227 0.67022,-0.71205 0.0522,-2.76798 z"
           points="109.84102,174.63451 110.51124,173.92246 110.56344,171.15448 109.89348,171.85224 " />
        <path
           sodipodi:type="inkscape:box3dside"
           id="path126-6"
           style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:0.0577385;stroke-linejoin:round;stroke-dasharray:none;stroke-opacity:0.955294"
           inkscape:box3dsidetype="3"
           d="m 108.93154,171.66308 0.96194,0.18916 -0.0525,2.78227 -0.96154,-0.21018 z"
           points="109.89348,171.85224 109.84102,174.63451 108.87948,174.42433 108.93154,171.66308 " />
      </g>
      <rect
         style="fill:#ff0000;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0.0792864;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         id="rect129"
         width="0.7987898"
         height="0.52646333"
         x="-29.925079"
         y="192.7254"
         transform="rotate(-38.4588)"
         inkscape:label="red_on_press_button" />
      <rect
         style="fill:#ff0000;fill-opacity:0.995294;fill-rule:nonzero;stroke:#000000;stroke-width:0.0792864;stroke-dasharray:none;stroke-opacity:0.955294;paint-order:stroke fill markers"
         id="rect129-5"
         width="0.7987898"
         height="0.52646333"
         x="-21.125273"
         y="203.07275"
         transform="rotate(-38.4588)"
         inkscape:label="red_on_press_button_simul" />
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
        "time_saved_value": document.getElementById("time_saved_value")
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

//    source.addEventListener('time_saved_feature', function (e) {
//        console.log(e.data);
//        svgElements["time_saved_value"].innerHTML = e.data;
//    }, false);

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
  // initialize both serial ports:
  MySerial.begin(115200);
  Serial.begin(115200);
//   Start the soft AP with a different SSID and specify network settings
  IPAddress softAP_IP(192, 168, 1, 203);
  IPAddress softAP_gateway(192, 168, 1, 1);
  IPAddress softAP_subnet(255, 255, 255, 0);
  WiFi.softAPConfig(softAP_IP, softAP_gateway, softAP_subnet);
  WiFi.softAP(ssid_for_browser, password);
  Serial.print("Soft AP IP Address: ");
  Serial.println(WiFi.softAPIP());
// This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  
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
  // Getting data of car_on_road_received
  if (MySerial.available() > 0){
    car_on_road_received = MySerial.read(); // WILL IT RETURN CORRECTLY?
    Serial.print("Car on road RX: ");
    Serial.println(car_on_road_received);
  }
  
  // turns on LED
  if (ldrVal > 350) {
     button_state = digitalRead(buttonPin);
//     Serial.print("             BUTTON STATE value: ");
//     Serial.println(button_state); 
     if (button_state == 1) {
        digitalWrite(ledPin, HIGH);
        led_state = true;
     }
//     delay(100); // so that this ESP sends only at button trigger, not On.
  }
  else {
     digitalWrite(ledPin, LOW);
     led_state = false;
  } 
  
  currentmillis_TL = millis();
  if ( (currentmillis_TL - startmillis_TL) > TL_reading_interval ) {
    ldrVal = analogRead(ldrPin);    // Read the analog value of the LDR
    if (counter >= 10 && led_state== true && car_on_road_received == true) { 
      button_state = 1;
      // Send message via ESP-NOW
      // @attention 2. If peer_addr is NULL, send data to all of the peers that are added to the peer list
//      esp_now_send(MAC_of_new_ESP, (uint8_t *) &button_state, sizeof(button_state));
      MySerial.write(button_state);
      button_state = 0; //reset
    }
    Serial.print("             LDR value: ");
    Serial.println(ldrVal);         // Show the value in the serial monitor
    if (ldrVal > 350) {
      Serial.println("GREEN_real");
      Serial.println("GREEN_simul");
    
      startmillis_for_amber = millis();
      currentmillis_for_amber = millis();
      if (send_event_green) {
        events.send("g", "real_TL", millis());
        events.send("g", "simul_TL", millis());
        send_event_green = false; // to avoid sending redundant signals to web
        time_saved = startmillis_time_saved_special_TL- startmillis_time_saved_TL;
//        events.send( String(time_saved).c_str(), "time_saved_feature", millis() );
      }
      Serial.print("Time saved: ");
      Serial.println(time_saved);
      send_event_red = true; // reset
      
      counter++;
//      Serial.print("Car on road? : ");
//      Serial.println(car_on_road_received);
      Serial.print("Counter: ");
      Serial.println(counter);

    }
    else if ( (currentmillis_for_amber - startmillis_for_amber) > amber_phase_duration && ldrVal < 350){
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
        startmillis_time_saved_TL = millis(); // capture time when yellow phase started
      }
      send_event_green = true;  // reset
    }
    // Special case when there are no cars on the road.
    if (counter >= 15 && car_on_road_received == false && counter_for_amber_simul <= amber_count_duration ){
      Serial.println("AMBER_simul_special");
      if (send_event_amber_special) {
        events.send("a", "simul_TL", millis());
        send_event_amber_special = false;
        startmillis_time_saved_special_TL = millis(); // capture time when yellow phase started
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
