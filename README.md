# moodlightlamp
title: "moodlightlamp"
author: "Maximus"
description: "intensity based ,esp color changing 3d printed modular lamp, uses light sensors to get current light room and adjust neopixel colors accordingly "
created_at: "1/7/24"
https://cad.onshape.com/documents/24e9abbaab56b7240cb457e8/w/20b29b3eb999e24861c2671e/e/4f256c3f2ddf39ff50254116?renderMode=0&uiState=6869f6d7b630910b3c71f076
total hrs speant- 18+
esp ws28b12 based mood light lamp that changes intensity ,color based on light outside , this is a 3d printed designed ,lamp split in 9 pieces a semicircular base that holds the esp32 ,lipobattery intensity and light sensors and controls, wherein the 4 parts form a circle thst interlock and contain the ws28b12 leds
this is a modulkar design entiely designed by me, the schematic and pcb were enirely designed by me too, the code was taken from multiple instructable and aurdio formas i made some adjustmnts to code it was in doff fies ombined them to oned added coder for deep sleep but no ide if itll work , my focud was on design this ttime hope it passes
model
![image](https://github.com/user-attachments/assets/bcf766f4-5bee-4ce8-ae39-e35570022209)
schematic
![image](https://github.com/user-attachments/assets/ea8c061c-abf7-4d50-9d21-5f0ccadb76d6)

pcb
![image](https://github.com/user-attachments/assets/0bbb20d9-b39a-4deb-94a4-2a9e96e4c1fc)


compeletely modular i guess thats the term i coc\nsists of 9 total pats that cn be assembled to get the light

inner diffuser section that snaps together on outer part
![image](https://github.com/user-attachments/assets/13bd1f42-7b2b-4f39-a4de-af1be7adfe09)

top paty after assembly of 4 pieces-
![image](https://github.com/user-attachments/assets/ce5ee1df-acca-4042-a582-9d4638dff2ea)

stand-
![image](https://github.com/user-attachments/assets/1e3857ba-2f29-4ce1-a807-43f56f4739c9)

| Item No. | Name | Supplier | Cost (INR) | Cost (USD) | Comment | Link |
|---|---|---|---|---|---|---|
| 1 | Microcontroller: ESP32-DevKitC-32E V4 | Amazon India | 600 | 7.02 | | https://www.amazon.in/ESP32-DevkitC-32E-V4-Latest-ESP32-Development/dp/B0BSSB25MS |
| 2 | LED Strip: AURECIS Individually Addressable RGBIC (30 LEDs/m, ~0.85m) | Amazon India | 750 | 8.78 | Approx. 0.85m length for 27cm diameter circle, approx. 26 LEDs. | https://www.amazon.in/AURECIS-Individually-Addressable-Non-Waterproof-Combinations/dp/B0FC96PCJQ/ |
| 3 | Battery: 18650 3.7V 3800mAH Li-Ion with Header (Pack of 2) | Robocraze | 250 | 2.93 | Reverted to original selection. Two batteries in parallel (1S2P) for 7600mAh total. | https://robocraze.com/products/18650-3-7v-3800mah-li-ion-rechargeable-battery-pack-of-2 |
| 4 | Battery Charger: TP4056 C Type Module with Protection | Robocraze | 14 | 0.16 | | https://robocraze.com/products/tp4056-battery-charger-c-type-module-with-protection-1 |
| 5 | Boost Converter: MT3608 DC-DC Boost Module (2V-24V) | Robocraze | 35 | 0.41 | | https://robocraze.com/products/mt3608-dc-dc-boost-module-2V-24V?_pos=1&_sid=a627efe0c&_ss=r |
| 6 | Color Sensor: TCS34725 RGB Color Sensor Module | Robocraze | 200 | 2.34 | Reverted to original price. | https://robocraze.com/products/tcs34725-rgb-color-sensor?variant=40192938475673 |
| 7 | Button: Tactile 4 Pin Push Button Switch (12x12x7.3 mm) | Owned | 0 | 0 | Owned. Pack of 5, only 1 needed for soft power control. | |
| 8 | Female Berg Strip (40x1) | Robocraze | 27 | 0.42 | 3 | https://www.robocraze.com/products/40x1-female-berg-strip?_pos=1&_psq=berg&_ss=e&_v=1.0 |
| 9 | Male Berg Strip (40x1, 2.54mm Pitch) | Robocraze | 14 | 0.16 | 2 pcs. | https://www.robocraze.com/products/40x1-pin-2-54mm-pitch-male-berg-strip?_pos=3&_psq=berg&_ss=e&_v=1.0 |
| 10 | Capacitor (1000µF, 25V, Electrolytic) | Import Dukan | 10 | 0.12 | | https://www.importdukan.com/1000uf-25v-electrolytic-capacitor-resistors?search=1000uf+capicator&description=true |
| 11 | Resistor (470 Ohm, 1/4W, Pack of 10) | Robocraze | 9 | 0.11 | | https://www.robocraze.com/products/470-ohm-resistor-pack-of-10?_pos=1&_sid=04790dc15&_ss=r |
| 12 | 18650 1-Cell Holder | Robocraze | 72 | 0.84 | For secure mounting of two 18650 batteries (2 pcs). | https://www.robocraze.com/products/18650-1-cell-holder?_pos=4&_psq=holder&_ss=e&_v=1.0 |
| 13 | 3D Printing Shipping Cost | Printing Legion | 512.64 | 6 | shipping from printing Legion. | (Shipping) |
| 14 | Solder Wire (1 Meter, 10gm) | Robocraze | 15 | 0.18 | | https://www.robocraze.com/products/soldering-wire10gm?_pos=2&_psq=solder&_ss=e&_v=1.0 |
| 15 | Soldering Paste (50gm) | Robocraze | 40 | 0.47 | | https://www.robocraze.com/products/soldering-paste-50-gm?_pos=5&_sid=5845158cf&_ss=r |
| 16 | Veroboard (Stripboard) 6x4 inch | Robocraze | 40 | 0.47 | Reverted to original price. | |
| 17 | M3 Screws (40 pieces of screws) | Import Dukan | 175 | 2.05 | cheaper than amazon its like 60-70screws amazo askes 5-8 dollars its 1 | https://www.importdukan.com/m2.5-x-8.5mm-socket-head-cap-allen-screws-pack-10-screws |
| 18 | M3 x 6mm Brass Heat Set Threaded Round Insert Nuts (25 pcs) | RoboticsDNA | 170.88 | 2 | Price includes delivery. | https://roboticsdna.in/product/m3-x-6mm-brass-heat-set-threaded-round-insert-nut-25-pcs/ |
| 19 | Price Increase Contingency | - | 427.2 | 5 | | |
| 20 | IRF520 MOSFET Driver Module | Robocraze | 42 | 0.49 | For "soft power" control of main circuit via ESP32. Heat sink needed for loads > 1A. | https://robocraze.com/products/keyes-mos-driving-module-for-boards-compatible-with-arduino |
| 21 | Aluminium Heatsink 19mmx19mmx15mm Silver Tone | Robocraze | 12 | 0.14 | For MOSFET. Requires heatsink in enclosed space. | https://robocraze.com/products/aluminium-heatsink-19mmx19mmx15mm-silver-tone |
| 22 | Hook up Wire Packet (1 meter each, 5 colors) | Robocraze | 49 | 0.57 | | |
| **Grand Total (INR)** | | | **3415.72** | | | |
| **Grand Total (USD)** | | | | **40.09** | | |
