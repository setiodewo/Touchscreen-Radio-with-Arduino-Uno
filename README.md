# Touchscreen Radio with Arduino Arduino

I created a Touchscreen radio with this components:
1. Arduino Uno or Leonardo
2. Color TFT 2.4" Touchscreen Shield
3. Si4703 FM Radio Module
4. DS3231 or DS1307 RTC Module
5. PAM8403 Class D Amplifier Module

Si4703 wiring:
    Si4703   Arduino Uno
    3.3V --- 3.3V
    GND  --- GND
    SDIO --- A4
    SCLK --- A5
    RST  --- D0

Please change SI4703.cpp --> resetPin to 0 (originally 2)

The detail can be read on my blog: https://dewo.wordpress.com/2018/05/01/touchscreen-radio-clock-with-arduino-uno-source-code/

The demo video can be watch at youtube:
https://www.youtube.com/watch?v=M0x0-A1TMFE

Unfortunately the RAM of UNO is not enough for this project to add alarm functionalities. Perhaps I will use Arduino Mega in the future.
