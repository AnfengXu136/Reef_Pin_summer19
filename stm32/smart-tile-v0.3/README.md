# smart-tile-v0.3

## Changes/Updates from v0.2.1

* Implemented 3 states: wet/standby/charging
* Changed wakeup/standby setting suited to wet/dry system: turn on when PA1 high and turn off when PA1 low
* Added charging state
* Changed the charging display
* Added turn on/off for LCD2
* Added water enable pin
* Disabled interrupt when in water. When in water, we do not want to use the wet/dry circutry because of the corrosion. Please disable the enable pin for wet/dry (something yet to be done). We have not developped the logistics to turn off the MCU when in water, but I believe you can either use depth information or turn on wet/dry circuit occationaly to check if it is on land.
