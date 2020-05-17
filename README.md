# ECU documentation for Bombyx mori

![GitHub last commit](https://img.shields.io/github/last-commit/GLWine/ECU-Bombyx?logo=github&style=social)
![GitHub release (latest SemVer including pre-releases)](https://img.shields.io/github/v/release/GLWine/ECU-Bombyx?include_prereleases&style=social)
![GitHub All Releases](https://img.shields.io/github/downloads/GLWine/ECU-Bombyx/total?style=social)

Thesis project on the effects of climate and gases in companies that produce silk. Use of an Arduino system called ECU-Bombyx (Environmental control unit - Bombyx Mori).

Purpose: to measure changes in temperature, humidity, eCO2 and total volatile organic compounds (TVOC) to correlate any effects on the yarn obtained.
The project consists of an Arduino, a humidity and temperature sensor, a clock module, an SD card shield and an SGP30 sensor for air quality.
Every 15 minutes The ECU-BM performs a reading that it saves in the SD.

<table cellspacing=”0″ cellpadding=”0″ width=”560″ border="0">
<tbody>
<tr>
<td valign=”top” width=”250”><img border="0" src="https://github.com/GLWine/ECU-Bombyx/blob/master/Prototipo%203D/Alto.png"></td>
<td valign=”top” width=”250”><img border="0" src="https://github.com/GLWine/ECU-Bombyx/blob/master/Prototipo%203D/ECU-B.png"></td>
</tr></tbody></table>

# Installation guide

## Phase 1: *wiring*
Before installing and starting make sure of the presence of these modules:
- DHT 22 sensor (temperature and humidity);
- Micro-SD module;
- RTC DS1302 module (clock);
- SGP30 air quality sensor;
- Arduino module one turn. 3;
Strictly carry out the wiring as indicated in the ECU-Bombyx image.

## Phase 2: *software installation*
Step 0: install the necessary libraries.

Step 1: install the ECU-Bombyx-Setter file from the Arduino IDE. In the "Serial monitor" view, make sure all modules respond.

--- Skip if the previous one is successful ---

Step 2: carefully check the wiring and that the modules are actually working.

Step 3: install the ECU-Bombyx-Base-Line file from the Arduino IDE. This is to find the baseline for the SGP30 sensor. Based on the [manufacturer's information](https://github.com/adafruit/DHT-sensor-library/pull/138 "Baseline Set & Get"), I created the code. This passage is of fundamental importance. The procedure lasts 12, it must be done for each SGP30 you have.

Step 4: install the ECU-Bombyx-Main file from the Arduino IDE. In the "Serial monitor" view, make sure that all modules respond and complete operations.

Final notes:
1. will start reading every 15 minutes writing in the micro-SD. The save format is .cvs with the company name.
2. If the DHT 22 module does not mount a 4K7 Ohm resistor, it is essential to solder one between DAT and VDD.
3. (optional) Add a 100nF capacitor between VDD and GND for filtering the waves.
3. There is a voluntary error in the ECU-BOMBYX-Main code. The reason is to remind those who use the code to set the variable that gives the name the file with the readings.

# Libraries

I made some changes to it based on this libbrerie of DHT [Pull requests #138](https://github.com/adafruit/DHT-sensor-library/pull/138 "Fixed signed/unsigned and unused parameter warnings")

The library to be imported with the correction is in the zip, it is sufficient to decompress it in the folder ..\Documents\Arduino\libraries\.

## Libraries List

Here are the lepers used for the code:

- Adafruit SGP30 Air Quality Sensor Breakout
  - Adafruit SGP30 Sensor by Adafruit V.1.2.0
- DS1302: Trickle-Charge Timekeeping Chip
  - RTC by Makuna V.2.3.4
- DHT22 temperature-humidity sensor + extras
  - DHT sensor librery by Adafruit V.1.3.9 **_modified_**

I recommend installing all the dependencies that arduino IDE offers

# License   [![GitHub license](https://img.shields.io/github/license/GLWine/ECU-Bombyx)](https://github.com/GLWine/ECU-Bombyx/blob/master/LICENSE.md)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
