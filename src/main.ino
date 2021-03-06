/*----------------------------------------------------------------------------//
//  Max Chichikalov (12 2016)
//  Posted the data (received from UART) to  www.narodmon.ru
//  ESP8266, ver0.1
//----------------------------------------------------------------------------*/
#include "ESP8266WiFi.h"
#include "Ticker.h"

#define  PASSWORD              "ABCDE12345"         //password of WiFi
#define  SSID                  "Sport16"            //ssid WiFi
#define  HOST                  "narodmon.ru"        //adres of HOST
#define  HTTPPORT              8283                 // port
#define  INTERVAL              (5*60)               // 5 minut
#define  LED                   5                    // pin LED

String text;
Ticker flipper;
volatile int tm = 60;
volatile int tm_led = 0;
double tempr_out;
double tempr_in1;
double tempr_in2;
double tempr_sys;
double dht_in1;
int isBeepEnable;
int isHeatEnable;
bool stringComplete = false;

void flip(void);      //callback function of timer (period 1 sec)
int getInt(void);     // convert string to Integer
float getFloat(void); // convert string to float
void strToData(void); // convert the whole string to data
void Send(void);      // send data into www.narodmon.ru


void setup() {
  pinMode(LED, OUTPUT_OPEN_DRAIN); // the LED for indication of receiving data from UART
  digitalWrite(LED, HIGH);         // turn off LED
  Serial.begin(115200);
  text.reserve(200);
  delay(10);
  flipper.attach(1, flip);          // run timer
}


void loop() {
  // text = "-15.69,22.14,20.14,65.14,68.54,0,1#";
 if (tm == 0){
    tm = INTERVAL;
    Send();
  }

  if ((tm == tm_led) && (tm_led != 0)){
    digitalWrite(LED, HIGH);
    tm_led = 0;
  }

  if (stringComplete) {
    strToData();
    text = "";
    stringComplete = false;
    tm_led = tm - 2;
    digitalWrite(LED, LOW);
  }

  while (Serial.available()) {
    char inChar = (char)Serial.read();
    text += inChar;
    if (inChar == '#') {
      stringComplete = true;
    }
  }

  yield();  // or delay(0);
}

/// sending the data into narodmon.ru
void Send() {
  short temp_count = 0;
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
	yield();  // or delay(0);
    delay(500);
    ++temp_count;
    if (temp_count == 20) {
    	ESP.restart();
    }
  }
  WiFiClient client;
  if (!client.connect(HOST, HTTPPORT)) {
	tm = 30; // in 30 sec trying again
	WiFi.disconnect();
    return;
  }

  String temp = "#";
  temp += WiFi.macAddress();
  temp += "#ESP\n#out#";
  temp += String(tempr_out);
  temp += "\n#in#";
  temp += String(tempr_in1);
  temp += "\n#H-in#";
  temp += String(tempr_sys);
  temp += "\n#t-sys#";
  temp += String(dht_in1);
  temp += "\n#Heat#";
  temp += String(isHeatEnable);
  temp += "\n#Allarm#";
  temp += String(isBeepEnable);
  temp += "\n##";
  client.println(temp);
  delay(10);
  while(client.available()){
    String line = client.readStringUntil('\r');
 /* if answer is not equal OK, send DATA in 10 sec */
    if (line != "OK") {
    	tm = 10;
	}
  }
  client.stop();
  WiFi.disconnect();
}

void strToData(){
  if (text != ""){
    tempr_out 		= getFloat();
    tempr_in1 		= getFloat();
    tempr_in2 		= getFloat();
    tempr_sys 		= getFloat();
    dht_in1   		= getFloat();
    isBeepEnable 	= getInt();
    isHeatEnable 	= getInt();
  }
}

float getFloat(){
  String st_temp;
  int index = text.indexOf(',');
  st_temp = text.substring(0, index);
  text.remove(0, (index + 1));
  double f1;
  f1 = st_temp.toFloat();
  return f1;
}

int getInt(){
  String st_temp;
  int index = text.indexOf(',');
  st_temp = text.substring(0, index);
  text.remove(0, (index + 1));
  int int_temp;
  int_temp = st_temp.toInt();
  return int_temp;
}

void flip(){
  tm--;
}
