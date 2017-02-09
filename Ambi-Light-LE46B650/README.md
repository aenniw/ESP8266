# Ambi-Light-LE46B650

Projects is part of [Ambi-Light](https://github.com/aenniw/SamyGo/tree/master/Ambi-Light) project that aims to add
support of Ambient Light to [Samsung LE46B650](http://www.samsung.com/cz/consumer/tv-av/tv/hd/LE46B650T2WXXH).

Parts:
*   [ESP8266](http://www.ebay.com/itm/D1-Mini-NodeMcu-4M-bytes-Lua-WIFI-Development-Boards-ESP8266-by-WeMos-New-US-/391699373540?hash=item5b331a1de4:g:qOUAAOSwzLlXhRqp) or other solutions based on this processor,
*   [Individual-Addressable Led-strip](http://www.ebay.com/itm/WS2812B-5050-RGB-LED-Strip-5M-150-300-Leds-144-60LED-M-Individual-Addressable-5V-/371432213255?var=&hash=item567b15cb07:m:mtQ859zLUV_msJ6iSTwfRDg),
*   [PSU](http://www.ebay.com/itm/5V-2A-AC-2-5mm-DC-USB-Charger-Power-Supply-Adapter-EU-Plug-For-Android-Tablet-/112004132044?hash=item1a13f76ccc:g:1WIAAOSw8vZXMDen)

Note: 
*   PSU used for build needs to be adjusted based on used led-strip length

#### TV and ESP setup

    TV =======> Wifi AP =======> ESP8266

#### Used pin-outs

| Wemos D1  | ESP-8266 | Led Strip | Function                      |
|:---------:|:--------:|:---------:|:-----------------------------:|
| RX        | RXD      | DATA      | RXD                           |
| G         | GND      | GND       | Ground                        |
| 5V / 3V3  | - / 3.3V | +V        | Power                         |
