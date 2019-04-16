/**
Created 4.14.19 MNMakerMan
Web Server Options
  1. Test led (GPIO 5)
  2. Change Color and Turn On/Off all Lights
  3. Special Functions/Modes
         a. UpDown Multi Color
         b. Random Star Functions
         c. Slow Fade In And Out
         d. Cylon
         e. Color Wipe
         f. Strobe
         g. Sound Reactive (if you select black then special Sound Reactive Mode)
         h. Slow Fade In And Out
**/


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

//Red Test LED will be on GPIO 5
const int led = 5;

//Microphone Settings
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
int pastSample = 0;

//Globals for color values
String defaultColor ="#42dff4";
int redValue = 0;
int greenValue = 0;
int blueValue = 0;


//#Setup Neopixels
#define PIN  0                  //Neopixel Pins
#define NUMPIXELS      15       //Number of Pixels
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//Http Handler Functions
void home();            //not used yet
void handleRoot();              // function prototypes for HTTP handlers
void handleLED();     //Tester LED
void ledStripOn(); //Turn All Neopixels On
void ledStripOff(); //Turn All Neopixels Off
void handleNotFound();

//http Light Show Functions
void starShowHTTP(); //Random Star Function
void upDownShowHTTP(); //upDown Function
void upDownShowSingleColorHTTP(); //Single updown show single color
void slowFadeHTTP(); //bounce ball function
void cylonShowHTTP();
void colorWipeShowHTTP();
void strobeShowHTTP();
void soundReactiveHTTP();

//Light Functions (Going to place in main loop and change Global booleans cause it works)
void upDownShow(); //Up Down MultiColor
void upDownShowSingleColor(); //Up Down Single Color
void starShow(); //Random Star Function
void slowFade(); // bounce ball Function
void cylonShow(); //Cylong Function
void colorWipeShow(); //Color Wipe Functions
void strobeShow(); //StobeShow
void soundReactive();  //Light Reactive Show

//booleans to trigger above light functions
boolean upDownShowOn = 0;
boolean upDownShowSingleOn = 0;
boolean starShowOn = 0;
boolean slowFadeOn = 0;
boolean cylonShowOn = 0;
boolean colorWipeShowOn = 0;
boolean strobeShowOn = 0;
boolean soundReactiveOn = 0;

int slowFadeState = 0;
int slowfadeColor = 0;

//Supoort Functions
void updateLightShow(int delays,int forward,int colorR, int colorG, int colorB);
void setColor(int Pixel, int R2, int G2, int B2);
void extractRGB (String rgb);

void setup(void){
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  pinMode(led, OUTPUT); //Setup Test LED

  wifiMulti.addAP("Wifi SSID", "Wifi Password");   // add Wi-Fi networks you want to connect to

  Serial.println("Connecting ...");
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  //Testing Out Additional Pages
  server.on("/home", home);

  //Functions called when a POST request is made to certain URI
  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/LED", HTTP_POST, handleLED);  // Call the 'handleLED' function when a POST request is made to URI "/LED"
  server.on("/ledStripOn", HTTP_POST, ledStripOn);
  server.on("/ledStripOff", HTTP_POST, ledStripOff);
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  //Special Light Show Functions
  server.on("/upDownShowHTTP", HTTP_POST, upDownShowHTTP); //UpDownShow Function
  server.on("/starShowHTTP", HTTP_POST, starShowHTTP); //Star function
  server.on("/upDownShowSingleColorHTTP", HTTP_POST, upDownShowSingleColorHTTP); //updown single color function
  server.on("/slowFadeHTTP", HTTP_POST, slowFadeHTTP); //bounce ball
  server.on("/cylonShowHTTP", HTTP_POST, cylonShowHTTP); //cylon show
  server.on("/strobeShowHTTP", HTTP_POST, strobeShowHTTP); //cylon show
  server.on("/colorWipeShowHTTP", HTTP_POST, colorWipeShowHTTP); //cylon show
  server.on("/soundReactiveHTTP", HTTP_POST, soundReactiveHTTP); //cylon show
  server.begin();                           // Actually start the server

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.show(); // This sends the updated pixel color to the hardware.

  setColor(7500, 0, 0, 0); //initialize all LEDs to off
}

void loop(void){
  server.handleClient();                    // Listen for HTTP requests from client

  //Run Light Show Functions
  upDownShow();
  starShow();
  upDownShowSingleColor();
  slowFade();
  cylonShow();
  strobeShow();
  colorWipeShow();
  soundReactive();

}

void home(){  //currently unused
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);
}

void handleRoot() {
  //Serial.println(defaultColor);
  String ptr = "<style>"
  "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
  "p {font-size: 50px; margin: 5px 10px; color: #ffffff; }"
  "header {font-size: 80px; text-align: center; color:#ffffff}"
  "body { background-color: #000000}"
  "input[type=submit] {background-color: #008CBA; border: none; font-size: 24px; border-radius: 12px; width: 50%; padding: 15px 32px; border: 2px solid #ffffff; color: white;}"
  "input[type=color] {border:none; width: 50%; height: 5%}"
  "</style>"
  "<center><header><b>Ikea Grono LED Lamp</b></header><br><br>"
  "<br><form action=\"/LED\" method=\"POST\"><input type=\"submit\" value=\"Test LED\"></form>"
  "<p>Color Selection</p>"
  "<form action=\"/ledStripOn\" method=\"POST\">"
  "<input type=\"color\" name=\"Color\" value=" + defaultColor;                  //This is so the color on server is updated
  ptr +=   "><br>"
  "<br><br>"
  "<input type=\"submit\" value=\"Turn On Neopixels\"></form>"
  "<br>"
  "<form action=\"/ledStripOff\" method=\"POST\"><input type=\"submit\" value=\"Turn Off Neopixels\"></form>"
  "<br>"
  "<p>Functions</p>"
  "<form action=\"/upDownShowHTTP\" method=\"POST\"><input type=\"submit\" value=\"Up Down Light Show\"></form>"
  "<br>"
  "<form action=\"/starShowHTTP\" method=\"POST\"><input type=\"submit\" value=\"Sparkle Stars\"></form>"
  "<br>"
  "<form action=\"/upDownShowSingleColorHTTP\" method=\"POST\"><input type=\"submit\" value=\"Up Down Single Color\"></form>"
  "<br>"
  "<form action=\"/slowFadeHTTP\" method=\"POST\"><input type=\"submit\" value=\"Slow Fade\"></form>"
  "<br>"
  "<form action=\"/cylonShowHTTP\" method=\"POST\"><input type=\"submit\" value=\"Cylon\"></form>"
  "<br>"
  "<form action=\"/strobeShowHTTP\" method=\"POST\">""<input type=\"submit\" value=\"Strobe\"></form>"
  "<br>"
  "<form action=\"/colorWipeShowHTTP\" method=\"POST\"><input type=\"submit\" value=\"Color Wipe\"></form>"
  "<br>"
  "<form action=\"/soundReactiveHTTP\" method=\"POST\"><input type=\"submit\" value=\"Sound Reactive\"></form>"
  "<br>"
  "</center>";
  server.send(200, "text/html", ptr);
}

//************Toggle Test LED**********************
void handleLED() {                          // If a POST request is made to URI /LED for testing purposes
  digitalWrite(led,!digitalRead(led));      // Change the state of the LED
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

//************Turns LED Strip On **********************
void ledStripOn(){
  if( ! server.hasArg("Color") || server.arg("Color") == NULL) { // If the POST request doesn't have color then invalid
    server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
    return;
  }
  else {
    setColor(7500, 0, 0, 0);

    server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect

    defaultColor = server.arg("Color");
    Serial.println(defaultColor);
    extractRGB(defaultColor); //extract rgb values from default Color. Goes into Globals redValue, greenValue, blueValue)

    setColor(7500, redValue, greenValue, blueValue);
  }
}
void extractRGB(String rgb){
  int numColor = (int) strtol(&rgb[1], NULL, 16);
  redValue = numColor >> 16;
  greenValue = numColor >> 8 & 0xFF;
  blueValue = numColor & 0xFF;
}

//************Turn off LED Strip (Also reset special function booleans)**********************
void ledStripOff(){

  //Turn Off all Booleans
  upDownShowOn = 0;
  upDownShowSingleOn = 0;
  starShowOn = 0;
  slowFadeOn = 0;
  cylonShowOn = 0;
  colorWipeShowOn = 0;
  strobeShowOn = 0;
  soundReactiveOn = 0;

  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect

  for(int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
      pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

//************Turn on LEDS (If pixel is 7500 then turn on all LEDS)**********************
void setColor(int Pixel, int R2, int G2, int B2) {
  uint32_t color = pixels.Color(R2, G2, B2); // make a color

  if (Pixel==7500){                                                         //7500 means all the LEDs turn on
    for (int i=0; i<=NUMPIXELS;i++){
      pixels.setPixelColor(i, color); //set a single pixel color
    }
  }
  pixels.setPixelColor(Pixel, color); //set a single pixel color
  pixels.show(); //update the colors
}

//************Clear Lights**********************
void clearLights() {
  uint32_t color = pixels.Color(0,0,0); // make a color
  for(int i=0;i<=NUMPIXELS;i++){
    pixels.setPixelColor(i,color);
    pixels.show();
  }
}

//************Turn on Star Show Boolean**********************
void starShowHTTP(){
    server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303);
    starShowOn = 1;
}

//************Turn on Up Down Show Boolean**********************
void upDownShowHTTP() {
    server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303);
    upDownShowOn = 1;
}

//************Turn on Up Down Show Single Color Boolean**********************
void upDownShowSingleColorHTTP() {
    server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303);
    upDownShowSingleOn = 1;
}

//************Turn on Slow Fade Boolean**********************
void slowFadeHTTP() {
    server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303);
    slowFadeOn = 1;
    slowFadeState = 0;
}

//************Turn on Cylong Show Boolean**********************
void cylonShowHTTP() {
    server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303);
    cylonShowOn = 1;
}

//************Turn on Strobe Show Boolean**********************
void strobeShowHTTP() {
    server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303);
    strobeShowOn = 1;
}

//************Turn on Color Wipe Boolean**********************
void colorWipeShowHTTP() {
    server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303);
    colorWipeShowOn = 1;
}

//************Turn on Sound Reactive Boolean**********************
void soundReactiveHTTP() {
    server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
    server.send(303);
    soundReactiveOn = 1;
}

//************Error handle**********************
void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

//Light Show Functions

void starShow() {
  if (starShowOn == 1){
      pixels.setPixelColor(random(NUMPIXELS), pixels.Color(random(0,255),random(0,255),random(0,255)));
      delay(25);
      for(int i=0;i<NUMPIXELS;i++){
        setColor(i, 0, 0, 0);
      }
  }
}

void colorWipeShow() {
  if (colorWipeShowOn == 1){
    for(int i=0; i<NUMPIXELS; i++) {
      setColor(i, redValue, greenValue, blueValue);
      delay(75);
    }
    for(int i=0; i<NUMPIXELS; i++) {
      setColor(i, 0, 0, 0);
      delay(75);
    }
  }
}

void upDownShow() {
  int delays=35;
  if (upDownShowOn == 1){
    updateLightShow(delays,1,random(0,255),random(0,255),random(0,255));
    updateLightShow(delays,0,random(0,255),random(0,255),random(0,255));
  }
}

void soundReactive() {
  if (soundReactiveOn == 1){
    unsigned long startMillis= millis();  // Start of sample window
    unsigned int peakToPeak = 0;   // peak-to-peak level

    unsigned int signalMax = 0;
    unsigned int signalMin = 1024;


    // collect data for 50 mS
    while (millis() - startMillis < sampleWindow)  {
      sample = analogRead(0);
      delay(4);    //Need this delay otherwise esp8266 gets locked up.
      yield();
      if (sample < 1024)  {
           if (sample > signalMax) {
              signalMax = sample;  // save just the max levels
           }
           else if (sample < signalMin){
              signalMin = sample;  // save just the min levels
           }
        }
     }
     peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
     double volts = (peakToPeak * 5.0) / 1024;  // convert to volts
     int pixelsLight = volts/ .33;

     //If new number is lower than the past sample, then starting at the past Sample, turn off leds until you reach pixelsLight
     //else measning the new number is higher than the past sample, fill in leds normally

     if (pixelsLight <= pastSample){
       for(int i=pastSample; i >pixelsLight; i--) {
         setColor(i, 0,0,0);
       }
     }
     else {
       for(int i=0; i < pixelsLight; i++) {
         //Do something special for color #000000 BLACK
         if (defaultColor == "#000000"){
           if (i<3){
             setColor(i, 255,202,51);
           }
           else if (i>=3 && i<5){
             setColor(i, 66,193,1);
           }
           else if (i>=5 && i<7){
             setColor(i, 6, 215, 223);
           }
           else if (i>=7 && i<10){
             setColor(i, 194,6, 223);
           }
           else{
             setColor(i, 223, 6, 6);
           }
         }
         else{
           setColor(i, redValue, greenValue, blueValue);
         }
         delay(10);
       }
     }
     pastSample = pixelsLight;
  }
}

void strobeShow(){
  if (strobeShowOn == 1){
    for(int j = 0; j < 10; j++) {
      setColor(7500, redValue, greenValue, blueValue);
      delay(50);
      setColor(7500, 0, 0, 0);
      delay(50);
    }
  }
}




void upDownShowSingleColor() {
  int delays=35;
  if (upDownShowSingleOn == 1){
    int randomRed = random(0,155);
    int randomBlue = random(0,155);
    int randomGreen = random(0,155);
    updateLightShow(delays,1,randomRed, randomBlue, randomGreen);
    updateLightShow(delays,0,randomRed, randomBlue, randomGreen);
  }
}

void slowFade(){

  if(slowFadeOn== 1){
    if (slowFadeState == 1){
      int picker = random(0,6);
      for(int k = 0; k<256; k++){
        switch(picker){
          case 0: setColor(7500,k,0,0);
          break;
          case 1: setColor(7500,0,k,0);
          break;
          case 2: setColor(7500,0,0,k);
          break;
          case 3: setColor(7500,k,k,0);
          break;
          case 4: setColor(7500,k,0,k);
          break;
          case 5: setColor(7500,0,k,k);
          break;
          case 6: setColor(7500,k,k,k);
          break;
        }
        delay(10);
      }
      slowFadeState = 0;
      slowfadeColor = picker;
    }
    else{
      for(int k = 255; k>=0; k--){
        switch(slowfadeColor){
          case 0: setColor(7500,k,0,0);
          break;
          case 1: setColor(7500,0,k,0);
          break;
          case 2: setColor(7500,0,0,k);
          break;
          case 3: setColor(7500,k,k,0);
          break;
          case 4: setColor(7500,k,0,k);
          break;
          case 5: setColor(7500,0,k,k);
          break;
          case 6: setColor(7500,k,k,k);
          break;
        }
        delay(10);
      }
      slowFadeState = 1;
    }
  }
}

void cylonShow() {


  //Convert the hex string into 3 seperate ints for the pixels.color command
  int EyeSize = 1;
  int SpeedDelay = 30;
  int ReturnDelay = 50;

  if (cylonShowOn == 1){
    for(int i = 0; i < NUMPIXELS-EyeSize-2; i++) {
        //setAll(0,0,0);
        for(int i=0;i<NUMPIXELS;i++){
            pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
            pixels.show(); // This sends the updated pixel color to the hardware.
        }
        setColor(i, redValue, greenValue, blueValue);
    for(int j = 1; j <= EyeSize; j++) {
        setColor(i+j, redValue, greenValue, blueValue);
    }
    setColor(i+EyeSize+1, redValue, greenValue, blueValue);
    pixels.show();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);

  for(int i = NUMPIXELS-EyeSize-2; i > 0; i--) {
  //  setAll(0,0,0);
    for(int i=0;i<NUMPIXELS;i++){
        pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
        pixels.show(); // This sends the updated pixel color to the hardware.
    }
    setColor(i, redValue, greenValue, blueValue);
    for(int j = 1; j <= EyeSize; j++) {
      setColor(i+j, redValue, greenValue, blueValue);
    }
    setColor(i+EyeSize+1, redValue, greenValue, blueValue);
    pixels.show();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);
  }
}

//Support Functions
void updateLightShow(int delays,int forward,int colorR, int colorG, int colorB){
  if (forward ==1){
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(colorR, colorG, colorB)); //turn all to red
        if (i>0) {
          pixels.setPixelColor(i-1, pixels.Color(0, 0, 0));
        }
        delay(delays);
        pixels.show();
      }
  }
  else{
      for (int i = NUMPIXELS; i >0; i--) {
        pixels.setPixelColor(i, pixels.Color(colorR, colorG, colorB)); //turn all to red
        if (i<NUMPIXELS) {
          pixels.setPixelColor(i+1, pixels.Color(0, 0, 0));
        }
        delay(delays);
        pixels.show();
      }
  }
}
