# Decorative-Lights

Parts:
*   [ESP8266](https://www.ebay.com/itm/WEMOS-D1-Mini-Lite-WIFI-V1-0-0-Internet-Development-Board-1MB-Flash-ESP8285/263176521762?epid=873884728&hash=item3d468b0822:g:LI0AAOSwOsBZpp5M) or other solutions based on this processor,
*   [Individual-Addressable Led-strip](http://www.ebay.com/itm/WS2812B-5050-RGB-LED-Strip-5M-150-300-Leds-144-60LED-M-Individual-Addressable-5V-/371432213255?var=&hash=item567b15cb07:m:mtQ859zLUV_msJ6iSTwfRDg),
*   [PSU](https://www.ebay.com/itm/EU-Plug-USB-Charger-Travel-Wall-Charger-Adapter-Phone-Charger-for-iPhone-Samsung/132216603822?hash=item1ec8b944ae:m:mPF6wkB17adq9mAsrNkq5BQ)
*   [Cable](https://www.ebay.com/itm/Braided-Fabric-Micro-USB-Data-Sync-Charger-Cable-Cord-For-Cell-Phone-H3U5/172624074429?epid=1779939750&hash=item28313256bd:m:mpb8rO8jRKycpwmyiX-V8eQ)

Optional parts:
*   ESP-Case [Base.stl](./models/wemos-base.stl) [Base-button.stl](./models/wemos-reset-button.stl) [Cap.stl](./models/wemos-cap.stl) [Cap-button.stl](./models/wemos-cap-button.stl)
*   Light-Lantern [Base.stl](./models/lantern-bottom.stl) [Top.stl](./models/lantern-top.stl)

Note: 
*   PSU used for build needs to be adjusted based on used led-strip length

#### Usage

- on stock settings AP decorative-lights without any passwd will be created, from here you can configure behaviour of lights
- stock credentials are admin:admin, without these credentials only part of light settings is accesible
- after power-up while blue led is still off pressing reset button will cause WIFI config reset
- short press of button changes light mode \[color/all-rainbow/single-ranbow/all-custom/single/custom/off]
- long press of button adjust brightness of lights

#### Used pin-outs

| Wemos D1  | Led Strip | Function                      |
|:---------:|:---------:|:-----------------------------:|
| D8        | -         | Button                        |
| 3V3       | -         | Button                        |
| D4        | DATA      | Data                          |
| G         | GND       | Ground                        |
| 5V        | +V        | Power                         |
