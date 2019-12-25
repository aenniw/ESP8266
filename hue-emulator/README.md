# HUE-Bridge

Projects project that aims to add support of Ambient Light based on [Philips HUE](https://developers.meethue.com/develop/hue-api/lights-api/) protocol to ESP8266 based chips and led strips.
This implementation of _Philips HUE_ is quite slow as the SPIFFS is used for storing setting instead of internal RAM and using scenes may hang clients due to long timeouts

Parts: [ESP8266](http://www.ebay.com/itm/D1-Mini-NodeMcu-4M-bytes-Lua-WIFI-Development-Boards-ESP8266-by-WeMos-New-US-/391699373540?hash=item5b331a1de4:g:qOUAAOSwzLlXhRqp),
[WS2812B-RGB-LED](http://www.ebay.com/itm/WS2812B-5050-RGB-LED-Strip-5M-150-300-Leds-144-60LED-M-Individual-Addressable-5V-/371432213255?var=&hash=item567b15cb07:m:mtQ859zLUV_msJ6iSTwfRDg),
[USB-Charger](https://www.ebay.com/itm/White-Black-5V-2A-US-EU-Plug-1-Port-USB-Wall-Charger-Fast-Power-Adapter-Travel/311959304526?hash=item48a239394e:m:moM12m_XHuZ5v03MtcSnv6w:rk:29:pf:0)


#### Used pin-outs

| Wemos D1  | ESP-8266 | Led Strip | Function                      |
|:---------:|:--------:|:---------:|:-----------------------------:|
| RX        | RXD      | DATA      | RXD                           |
| G         | GND      | GND       | Ground                        |
| 5V / 3V3  | - / 3.3V | +V        | Power                         |

*   Connect a capacitor with a capacitance between 100uF and 1000uF from power to ground to smooth out the power supply.
*   Add a 220 or 470 Ohm resistor between the ESP digital output pin and the strip data input pin to reduce noise on that line.
