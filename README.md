# BroadcastifyListenersDisplayUnit

Displays the current number of listeners of a Bradcastify Stream on TFT LCD display.
Uses the feed owner API to get number of listeners.

## Hardware
HiLetgo ESP8266 NodeMCU CP2102 ESP-12E:
https://www.amazon.com/gp/product/B081CSJV2V/

HiLetgo 2.2 Inch ILI9341 SPI TFT LCD Display 240x320 ILI9341 LCD Screen:
https://www.amazon.com/gp/product/B01CZL6QIQ/

## To Use
Add http://arduino.esp8266.com/stable/package_esp8266com_index.json to Preferences > Additional Boards Manager URLs in Arduino IDE.  
  
Pins:  
TFT_DC -> ESP_D8  
TFT_CS -> ESP_D7  
TFT_RST -> ESP_D6  
TFT_MISO -> ESP_D5  
TFT_MOSI -> ESP_D4  
TFT_CLK -> ESP_D3
