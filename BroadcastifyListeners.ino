

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "sensData.h" //Put sesitive data in different file

//NETWORK VARS
const char* ssid = STASSID;
const char* password = STAPSK;
const char* host = "api.broadcastify.com";
const String url = "/owner/?a=feed&feedId=" + (String)FEEDID + "&type=json&u=" + FEEDUSR + "&p=" + FEEDPSK; //Had to cast FEEDID as String, idk why
const int httpsPort = 443;
const char fingerprint[] PROGMEM = FEEDFINGERPRINT;
// In Chrome, click the lock -> Certificate -> Details -> Thumbprint
//DISPLAY VARS
/*
D0 = 16;
D1 = 5;
D2 = 4;
D3 = 0;
D4 = 2;
D5 = 14;
D6 = 12;
D7 = 13;
D8 = 15;
D9 = 3;
D10 = 1;

font width = 6*SIZE
font height = 8*SIZE
*/
#define TFT_DC 15//D8
#define TFT_CS 13//D7
#define TFT_RST 12//D6
#define TFT_MISO 14//D5
#define TFT_MOSI 2//D4
#define TFT_CLK 0//D3
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

//OTHER VARS
String listeners = "";
int listenersInt = 0;
bool feedStatus = true;
//String descr = "";
String response = "";
const String sTitle = "Oak Park Scanner";
String title = sTitle;
int rebootMax = -1;
const int rollingMaxLength = 96;
int rollingMax[rollingMaxLength];
int updateCount = 0;


bool feedOffline = false;

const int listenersFntSz = 15;
const int descrFntSz = 3;
const int maxFntSz = 2;

int loopNum = 1;
int scrollLoop = 0;
int scrollSpd = 6;//must be a multiple of (descrFntSz*6)

const int colorFlip = 15;
bool colorFliped = false;

unsigned long currentMillisecs = 0;
unsigned long previousMillisecs = 0;
const long interval = 900000;


void setup() {
  Serial.begin(9600);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  updateData();

  tft.begin();
  tft.setRotation(1);
  if(listenersInt >= colorFlip)
  {
    tft.fillScreen(ILI9341_WHITE);
    colorFliped = true;
  }
  else
  {
    tft.fillScreen(ILI9341_BLACK);
  }
  tft.setTextWrap(false); 
}

String getData(String p, String key)
{
  p = p.substring(p.indexOf(key));
  p = p.substring(0, p.indexOf(", \""));
  p = p.substring(key.length() + 3);//+3 for the space : "
  return p;
}

void updateRollingMax(int n)
{
  if(updateCount < rollingMaxLength)
  {
    rollingMax[updateCount-1] = n;
  }
  else
  {
    for(int i = 0; i < rollingMaxLength-1; i++)
    {
      rollingMax[i] = rollingMax[i+1];
    }
    rollingMax[rollingMaxLength-1] = n;
  }
}

int getRollingMax()
{
  int m = -1;
  for(int i = 0; i < rollingMaxLength; i++)
  {
    if(rollingMax[i] > m)
    {
      m = rollingMax[i];
    }
  }
  return m;
}

void updateData() 
{
  WiFiClientSecure client;

  client.setFingerprint(FEEDFINGERPRINT);

  if (!client.connect(host, httpsPort)) {
    return;
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }


  response = client.readString();
  
  listeners = getData(response, "listeners");
  listenersInt = listeners.toInt();
  feedStatus = getData(response, "status").toInt() == 1;
  listeners = " " + listeners + " ";

  if(listenersInt >= rebootMax)
  {
    rebootMax = listenersInt;
  }
  
  
}


//  ================= LOOP =================
  void loop() {

  //Check Time, Update Print Time untill update
  currentMillisecs = millis();
  unsigned long remainingTime = interval - (currentMillisecs - previousMillisecs);
  int minutesRemaing = remainingTime/60000;
  int secondsRemaing = (remainingTime%60000)/1000;
  String minutesRemaingString = String(minutesRemaing);
  String secondsRemaingString = String(secondsRemaing);
  if(minutesRemaingString.length() < 2)
  {
    minutesRemaingString = "0" + minutesRemaingString;
  }
  if(secondsRemaingString.length() < 2)
  {
    secondsRemaingString = "0" + secondsRemaingString;
  }
  if(colorFliped)
  {
    tft.setTextColor(ILI9341_BLACK,ILI9341_WHITE);  
  }
  else
  {
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  }
  tft.setTextSize(maxFntSz);
  tft.setCursor(320-((maxFntSz*6) * 5) - 1, (240-maxFntSz*8)+1);
  tft.print(minutesRemaingString + ":" + secondsRemaingString);
  
  
  if((currentMillisecs - previousMillisecs) >= interval)
  {
    tft.setTextSize(maxFntSz);
    tft.setCursor(320-((maxFntSz*6) * 5) - 1, (240-maxFntSz*8)+1);
    tft.print("     ");
    updateData();
    updateCount++;//Moved updateCount++ and updateRollingMax() to prevent time from increasing too fast when feed offline
    updateRollingMax(listenersInt);
    if(listenersInt >= colorFlip && !colorFliped)
    {
      tft.fillScreen(ILI9341_WHITE);
      colorFliped = true;
    }else if(listenersInt < colorFlip && colorFliped)
    {
      tft.fillScreen(ILI9341_BLACK);
      colorFliped = false;
    }
    previousMillisecs = currentMillisecs;
  }
  if(feedStatus)
  {
    //Print Listeners
    int cursorPosX = (320-(listenersFntSz*6*listeners.length()))/2;
    int cursorPosY = ((240-(listenersFntSz*8))/2);
    tft.setCursor(cursorPosX, cursorPosY);
    tft.setTextSize(listenersFntSz);
    if(colorFliped)
    {
      tft.setTextColor(ILI9341_GREEN,ILI9341_WHITE);  
    }
    else
    {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    }
    tft.println(listeners);
    
    //Print Title
    tft.setCursor((320-(descrFntSz*6*title.length()))/2, 10);
    if(colorFliped)
    {
      tft.setTextColor(ILI9341_BLACK,ILI9341_WHITE);  
    }
    else
    {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  
    }
    
    tft.setTextSize(descrFntSz);
    tft.print(title);
  
    //Print Max
    tft.setCursor(1, (240-maxFntSz*8)+1);
    tft.setTextSize(maxFntSz);
    tft.print("Max " + String(updateCount/96) + " Days: " + String(rebootMax) + " ");
    tft.setCursor(1, (240-maxFntSz*16)-2);
    tft.print("Max " + String(min(updateCount/4,24)) + " Hours: " + String(getRollingMax()) + " ");

    
    
  }
  //FEED OFFLINE
  if(!feedStatus)
  {
    listeners = "";
    listenersInt = 0;
    if(!feedOffline)
    {
      tft.fillScreen(ILI9341_RED);
      feedOffline = true;
      title = "";
    }
    tft.setTextColor(ILI9341_ORANGE,ILI9341_RED);
    tft.setCursor(0, 0);
    tft.setTextSize(7);
    tft.println("FEED");
    tft.println("OFFLINE");
    updateData();
  }
  else
  {
    if(feedOffline)
    {
      title = sTitle;
      feedOffline = false;
      if(colorFliped)
      {
        tft.fillScreen(ILI9341_WHITE);  
      }
      else
      {
        tft.fillScreen(ILI9341_BLACK);  
      }
    }
  }
  
  //Increase LoopNum
  loopNum++;
}
