// Emanuel Setio Dewo, 08/04/2018
// Design UI for Radio
 
// SI4703 wiring:
// 3.3V --- 3.3V
// GND  --- GND
// SDIO --- A4
// SCLK --- A5
// RST  --- D0
//          Please change SI4703.cpp --> resetPin to 0 (originally 2)
 
#include <Arduino.h>
#include <Wire.h>
#include <radio.h>
#include <SI4703.h>
#include <EEPROM.h>
#include "RTClib.h"
 
#include <Adafruit_GFX.h>
/* #include <Fonts/Org_01.h> */
#include <TftSpfd5408.h>
#include <TouchScreen.h>
 
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
 
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
 
#define TS_MINX 120
#define TS_MAXX 1160
 
#define TS_MINY 322
#define TS_MAXY 910
 
 
#define YP A3
#define XM A2
#define YM 9
#define XP 8
 
#define MINPRESSURE 10
#define MAXPRESSURE 1000
 
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0x2222
 
#define BG      0xEEE
#define GRS     0xEEEF
 
#define MAX_VOLUME   14
#define MIN_FREQ     8750
#define MAX_FREQ     10800
#define eeprom_vol   0
#define eeprom_freq1 1
#define eeprom_freq2 2
 
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 364);
TftSpfd5408 tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
 
int radio_freq;
boolean radio_on = true;
boolean radio_ubah = false;
int radio_vol = 3;
int display_mode = 0;
long _millis_jam;
long _millis_radio;
int edit_hour, edit_minute, edit_day, edit_month, edit_year, edit_freq;
 
SI4703 radio;
RTC_DS3231 jam;
 
#define center_y   140
#define center_x   160
#define radius     50
#define lebar      100
#define tinggi     70
#define offset     10
#define max_width  320
#define max_height 240
 
char DOW[][7] = {"Minggu","Senin ","Selasa","Rabu  ","Kamis ","Jumat ","Sabtu "};
 
void setup() {
  _millis_jam = millis();
  _millis_radio = millis();
  radio.init();
  radio_read_mem();
  //radio.setBandFrequency(FIX_BAND, radio_freq);
  //radio.setVolume(radio_vol);
  radio.setMono(false);
  radio.setMute(radio_on);
 
  Reset_Display();
  Radio_UI();
}
 
void loop() {
  TSPoint p = ts.getPoint();
 
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
   
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) { TSPoint c = p; c.x = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0); c.y = map(p.x, TS_MINX, TS_MAXX, 0, tft.width()); //Touch_Coordinate(c); if (display_mode == 0) { // MODE RADIO if (touchin(c, 125, 100, 95, 95)) btn_on(true); if (touchin(c, 0, 100, lebar, 40)) btn_freq_down(true); if (touchin(c, 215, 100, lebar, 40)) btn_freq_up(true); if (touchin(c, 0, center_y, lebar, 40)) btn_vol_down(true); if (touchin(c, 215, center_y, lebar, 40)) btn_vol_up(true); if (touchin(c, 0, 190, 150, 50)) Display_Edit_Date(); if (touchin(c, 0, 0, 30, tinggi)) Display_Set_Freq(); } else if (display_mode == -1) { if (touchin(c, 0, 120, 320, 120)) { display_mode = 0; radio_on = true; Radio_UI(); } } else if (display_mode == 1) touching_mode_1(c); else if (display_mode == 2) touching_mode_2(c); else if (display_mode == 3) touching_mode_3(c); } // Jika mode radio nyala if (display_mode == 0) { if (millis() - _millis_jam > 30000) {
      Display_Jam();
      _millis_jam = millis();
    }
  }
 
  // Jika mode mati
  if (display_mode == -1) {
    if (millis() - _millis_jam > 30000) {
      Display_Off_Jam();
      _millis_jam = millis();
    }
  }
 
  // Jika ada perubahan radio
  if (millis() - _millis_radio > 10000) {
    if (radio_ubah) radio_write_mem();
  }
}
 
void touching_mode_1(TSPoint c) {
  if (touchin(c, 0, 200, 160, 50)) {
    display_mode = 0;
    Radio_UI();
  }
  // Day +1
  if (touchin(c, 260, 50, 60, 50)) {
    edit_day = edit_day +1;
    if (edit_day > 31) edit_day = 31;
    Display_Edit_Date_Update();
    delay(100);
  }
  // Day -1 320-120
  if (touchin(c, 200, 50, 60, 50)) {
    edit_day = edit_day -1;
    if (edit_day < 1) edit_day = 1; Display_Edit_Date_Update(); delay(100); } // Month +1 if (touchin(c, 260, 100, 60, 50)) { edit_month = edit_month +1; if (edit_month > 12) edit_month = 12;
    Display_Edit_Date_Update();
    delay(100);
  }
  // Month -1
  if (touchin(c, 200, 100, 60, 50)) {
    edit_month = edit_month -1;
    if (edit_month < 1) edit_month = 1; Display_Edit_Date_Update(); delay(100); } // Year +1 if (touchin(c, 260, 150, 60, 50)) { edit_year = edit_year +1; if (edit_year > 2050) edit_year = 2050;
    Display_Edit_Date_Update();
    delay(100);
  }
  // Year -1
  if (touchin(c, 200, 150, 60, 50)) {
    edit_year = edit_year -1;
    if (edit_year < 2010) edit_year = 2010; Display_Edit_Date_Update(); delay(100); } // Save if (touchin(c, 160, 200, 160, 40)) { jam.adjust(DateTime(edit_year, edit_month, edit_day, edit_hour, edit_minute, 0)); display_mode = 2; Display_Edit_Clock(); } } void touching_mode_2(TSPoint c) { if (touchin(c, 0, 200, 160, 50)) { display_mode = 0; Radio_UI(); } // Hour +1 if (touchin(c, 260, 50, 60, 50)) { edit_hour = edit_hour +1; if (edit_hour > 23) edit_hour = 0;
    Display_Edit_Clock_Update();
    delay(100);
  }
  // Hour -1
  if (touchin(c, 200, 50, 60, 50)) {
    edit_hour = edit_hour -1;
    if (edit_hour < 0) edit_hour = 23; Display_Edit_Clock_Update(); delay(100); } // Minute +1 if (touchin(c, 260, 100, 60, 50)) { edit_minute = edit_minute +1; if (edit_minute > 59) edit_minute = 0;
    Display_Edit_Clock_Update();
    delay(100);
  }
  // Minute -1
  if (touchin(c, 200, 100, 60, 50)) {
    edit_minute = edit_minute -1;
    if (edit_minute < 0) edit_month = 59;
    Display_Edit_Clock_Update();
    delay(100);
  }
  // Save
  if (touchin(c, 160, 200, 160, 40)) {
    jam.adjust(DateTime(edit_year, edit_month, edit_day, edit_hour, edit_minute, 0));
    display_mode = 0;
    Radio_UI();
  }
}
 
void touching_mode_3(TSPoint c) {
  if (touchin(c, 0, 200, 160, 50)) {
    display_mode = 0;
    Radio_UI();
  }
  if (touchin(c, 160, 200, 160, 49)) {
    radio_freq = edit_freq;
    radio.setBandFrequency(RADIO_BAND_FM, radio_freq);
    display_mode = 0;
    Radio_UI();
    _millis_radio = millis();
    radio_ubah = true;
  }
  if (touchin(c, 0, 50, 60, 140)) {
    edit_freq = edit_freq -10;
    if (edit_freq < MIN_FREQ) edit_freq = MAX_FREQ; Display_Set_Freq_Update(); } if (touchin(c, 260, 50, 60, 140)) { edit_freq = edit_freq +10; if (edit_freq > MAX_FREQ) edit_freq = MIN_FREQ;
    Display_Set_Freq_Update();
  }
}
 
boolean touchin(TSPoint p, int x, int y, int w, int h) {
  return (p.x > x && p.x < x+w && p.y > y && p.y < y+h);
}
 
void Reset_Display() {
  tft.reset();
  tft.begin(0x9341);
  tft.setRotation(1);
  //tft.setFont(&Org_01);
}
 
void Radio_UI() {
  Display_Base();
  Display_Freq();
  Display_Volume();
  btn_on(false);
  btn_freq_up(false);
  btn_freq_down(false);
  btn_vol_up(false);
  btn_vol_down(false);
}
 
void btn_on(boolean touched) {
  tft.fillCircle(center_x, center_y, radius-12, BLACK);
  tft.drawCircle(center_x, center_y, radius, WHITE);
 
  if (touched) {
    radio_on = !radio_on;
  }
  radio.setMute(!radio_on);
   
  if (radio_on) {
    tft.setCursor(144, 128);
    tft.setTextSize(3);
    tft.setTextColor(GREEN);
    tft.print("ON");
    for (int i=1; i<8; i++) { tft.drawCircle(center_x, center_y, (radius-4) - i, GREEN); } } else { /* tft.setCursor(center_x - 22, center_y - 12); tft.setTextSize(3); tft.setTextColor(WHITE); tft.print("Off"); for (int i=8; i>0; i--) {
      tft.drawCircle(center_x, center_y, radius-4 -i, BLACK);
    }
    */
    display_mode = -1;
    Display_Off();
  }
 
  if (touched) delay(100);
}
 
void btn_vol_up(boolean touched) {
  if (touched == false) {
    tft.setCursor(237, 159);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print("Vol +");
  }
  else {
    tft.fillCircle(310, 165, 6, GREEN);
    radio_vol++;
    if (radio_vol > MAX_VOLUME) radio_vol = MAX_VOLUME;
    radio.setVolume(radio_vol);
    Display_Volume();
    delay(100);
    radio_ubah = true;
    _millis_radio = millis();
  }
  tft.fillCircle(310, 165, 6, BLACK);
  tft.drawCircle(310, 165, 6, GRS);
}
 
void btn_vol_down(boolean touched) {
  if (touched == false) {
    tft.setCursor(25, 159);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print("- Vol");
  }
  else {
    tft.fillCircle(offset, 165, 6, GREEN);
    radio_vol--;
    if (radio_vol < 0) radio_vol = 0; radio.setVolume(radio_vol); Display_Volume(); delay(100); radio_ubah = true; _millis_radio = millis(); } tft.fillCircle(offset, 165, 6, BLACK); tft.drawCircle(offset, 165, 6, GRS); } void btn_freq_up(boolean touched) { if (touched == false) { tft.setCursor(226, 109); tft.setTextSize(2); tft.setTextColor(WHITE); tft.print("Seek >");
  }
  else {
    Display_Seek();
    tft.fillCircle(310, 115, 6, GREEN);
    radio.seekUp();
    Display_Freq();
    delay(50);
    radio_ubah = true;
    _millis_radio = millis();
  }
  tft.fillCircle(310, 115, 6, BLACK);
  tft.drawCircle(310, 115, 6, GRS);
}
 
void btn_freq_down(boolean touched) {
  if (touched == false) {
    tft.setCursor(25, 109);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print("< Seek");
  }
  else {
    Display_Seek();
    tft.fillCircle(offset, 115, 5, GREEN);
    radio.seekDown();
    Display_Freq();
    delay(50);
    radio_ubah = true;
    _millis_radio = millis();
  }
  tft.fillCircle(offset, 115, 6, BLACK);
  tft.drawCircle(offset, 115, 6, GRS);
}
 
void Display_Base() {
  tft.fillScreen(BLACK);
  tft.drawLine(0, 140, 320, 140, WHITE);
 
  // KIRI
  tft.drawLine(0, center_y-50, lebar, center_y-50, WHITE);
  tft.drawLine(lebar, center_y-50, center_x, center_y, WHITE);
 
  tft.drawLine(0, center_y+50, lebar, center_y+50, WHITE);
  tft.drawLine(lebar, center_y+50, center_x, center_y, WHITE);
 
  // KANAN
  tft.drawLine(320, center_y-50, 320-lebar, center_y-50, WHITE);
  tft.drawLine(320-lebar, center_y-50, center_x, center_y, WHITE);
 
  tft.drawLine(320, center_y+50, 320-lebar, center_y+50, WHITE);
  tft.drawLine(320-lebar, center_y+50, center_x, center_y, WHITE);
   
  tft.fillCircle(center_x, center_y, radius, BLACK);
  tft.drawCircle(center_x, center_y, radius, WHITE);
  tft.drawLine(center_x, 20, center_x, 60, GRS); // vertical-middle
 
  // FREQUENCY
  tft.drawRect(0, 0, offset*2, tinggi, WHITE);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.setCursor(offset*2 +6, 1);
  tft.print(MAX_FREQ / 100);
  tft.setCursor(offset*2 +6, (tinggi-8)/2);
  tft.print(((MAX_FREQ-MIN_FREQ)/2 + MIN_FREQ)/100);
  tft.setCursor(offset*2 +6, tinggi-8);
  tft.print(MIN_FREQ / 100);
 
  // VOLUME
  tft.drawRect(max_width-(offset*2), 0, offset*2, tinggi, WHITE);
  tft.setCursor(max_width-(offset*4)-4, 1);
  tft.print("Max");
  tft.setCursor(max_width-(offset*4)-4, tinggi-8);
  tft.print("Min");
 
  //Display_Alarm();
  Display_Jam();
  //tft.drawRoundRect(-10, max_height-38, 150, max_height+10, 5, BLACK);
}
 
void Display_Off() {
  tft.fillScreen(BLACK);
  // KIRI
  tft.drawLine(0, 179, 218, 179, WHITE);
  tft.drawLine(268, 0, 268, 129, WHITE);
  //tft.drawLine(200, max_height-radius-50, 270, 179, WHITE);
 
  //tft.fillCircle(269, 179, radius, BLACK);
  tft.drawCircle(269, 179, radius, WHITE);
  /*
  for (int i=1; i<8; i++) {
    tft.drawCircle(center_x, max_height-radius-1, (radius-4) - i, GRS);
  }
  */
 
  tft.setCursor(244, 168);
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.print("Off");
  Display_Off_Jam();
}
 
void Display_Off_Jam() {
  DateTime now = jam.now();
  tft.fillRect(0, 0, 267, 129, BLACK);
  tft.setTextSize(8);
  tft.setTextColor(WHITE);
  tft.setCursor(0, 34);
  if (now.hour() < 10) tft.print("0");
  tft.print(now.hour());
  tft.print(":");
  if (now.minute() < 10) tft.print("0");
  tft.print(now.minute());
 
  tft.setTextSize(2);
  tft.setTextColor(YELLOW);
  tft.setCursor(0, 114);
  if (now.day() < 10) tft.print("0");
  tft.print(now.day());
  tft.print("/");
  if (now.month() < 10) tft.print("0");
  tft.print(now.month());
  tft.print("/");
  tft.print(now.year());
 
  tft.print("  ");
  tft.print(DOW[now.dayOfTheWeek()]);
}
 
void Display_Jam() {
  DateTime now = jam.now();
  tft.fillRect(center_x+1, 0, max_width-center_x-50, tinggi+offset, BLACK);
 
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.setCursor(center_x+20, 15);
  tft.print(DOW[now.dayOfTheWeek()]);
   
  // JAM
  tft.setCursor(center_x+20, 30);
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  if (now.hour() < 10) tft.print("0");
  tft.print(now.hour(), DEC);
  tft.setTextSize(2);
  tft.setTextColor(YELLOW);
  tft.print(":");
  if (now.minute() < 10) tft.print("0");
  tft.print(now.minute(), DEC);
 
  // TANGGAL
  //tft.fillRect(0, max_height-40, 130, 40, BLACK);
  tft.fillRoundRect(-10, max_height-36, 140, max_height+10, 12, GRS);
  tft.setCursor(2, max_height-24);
  tft.setTextColor(BLACK);
  if (now.day() < 10) tft.print("0");
  tft.print(now.day());
  tft.print("/");
  if (now.month() < 10) tft.print("0"); tft.print(now.month()); tft.print("/"); tft.print(now.year()); } /* void Display_Alarm() { // ALARM SYMBOL int _cy = 224; tft.drawCircle(center_x, _cy, 22, WHITE); tft.drawCircle(center_x, _cy, 18, WHITE); tft.fillRect(center_x-22, _cy-22, 44, 10, BLACK); tft.fillRect(center_x-22, _cy+12, 44, 10, BLACK); tft.fillCircle(center_x, _cy, 10, BLACK); tft.drawCircle(center_x, _cy, 14, WHITE); tft.setTextColor(WHITE); tft.setTextSize(2); tft.setCursor(center_x-4, _cy-7); tft.print("A"); tft.setTextColor(GRS); tft.setTextSize(2); tft.setCursor(center_x + 40, _cy-7); tft.print("Alarm Off"); } */ void Display_Freq() { int f = radio.getFrequency(); tft.fillRect(50, 0, center_x - 51, tinggi+offset, BLACK); tft.setTextSize(1); tft.setTextColor(WHITE); tft.setCursor(55, 15); tft.print("FM Radio"); tft.setCursor(55, 30); tft.setTextSize(3); tft.setTextColor(WHITE); tft.print(f/100); tft.setTextSize(2); tft.setTextColor(YELLOW); tft.print(":"); tft.print(f%100); int posy = map(f, MIN_FREQ, MAX_FREQ, tinggi-6, 1); tft.fillRect(1, 1, offset*2-2, tinggi-2, BLACK); tft.fillRect(1, posy, 18, 5, RED); } void Display_Seek() { //tft.fillRect(lebar-offset, 0, 140, tinggi+offset, BLACK); tft.fillRect(50, 0, center_x - 51, tinggi+offset, BLACK); tft.setCursor(55, (tinggi-8)/2); tft.setTextSize(1); tft.setTextColor(WHITE); tft.print("Searching..."); } void Display_Saving() { //tft.fillRect(lebar-offset, 0, 140, tinggi+offset, BLACK); tft.fillRect(50, 0, center_x - 51, tinggi+offset, BLACK); tft.setCursor(55, (tinggi-8)/2); tft.setTextSize(1); tft.setTextColor(WHITE); tft.print("Saving..."); } void Display_Volume() { //tft.drawRoundRect(80, 36, 160, 183, 20, GRS); int v = map(radio_vol, 0, MAX_VOLUME, 0, tinggi-6); tft.fillRect(max_width-(offset*2)+1, 1, offset*2-2, tinggi-2, BLACK); tft.fillRect(max_width-(offset*2)+3, tinggi-v-3, offset*2-6, v, GREEN); } void Touch_Coordinate(TSPoint p) { tft.fillRect(0, 0, max_width, 50, BLACK); tft.setTextSize(2); tft.setTextColor(WHITE); tft.setCursor(0,5); tft.print(p.x); tft.print(", "); tft.print(p.y); } void radio_read_mem() { radio_vol = EEPROM.read(eeprom_vol); if (radio_vol > 14) {
    radio_vol = 2;
    EEPROM.write(eeprom_vol, radio_vol);
  }
  radio.setVolume(radio_vol);
 
  int radio_freq = (EEPROM.read(eeprom_freq1) * 100 + EEPROM.read(eeprom_freq2));
  if (radio_freq < MIN_FREQ || radio_freq > MAX_FREQ) {
    radio_freq = MIN_FREQ;
    EEPROM.write(eeprom_freq1, radio_freq/100);
    EEPROM.write(eeprom_freq2, radio_freq%100);
  }
  radio.setBandFrequency(RADIO_BAND_FM, radio_freq);
}
 
void radio_write_mem() {
  int f = radio.getFrequency();
  EEPROM.write(eeprom_vol, radio_vol);
  EEPROM.write(eeprom_freq1, f/100);
  EEPROM.write(eeprom_freq2, f%100);
  if (display_mode == 0) {
    Display_Saving();
    delay(1000);
    Display_Freq();
  }
  radio_ubah = false;
  _millis_radio = millis();
}
 
void Display_Set_Freq() {
  display_mode = 3;
  tft.fillScreen(BLACK);
  tft.fillRect(0, 0, 320, 49, RED);
 
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(lebar-20, 15);
  tft.print("Set Frequency");
 
  tft.setTextSize(3);
  tft.setCursor(offset, 100);
  tft.print("<"); tft.setCursor(max_width-30, 100); tft.print(">");
 
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(0, 215);
  tft.print("Back");
  tft.setCursor(max_width-50, 215);
  tft.print("Save");
 
  edit_freq = radio.getFrequency();
  Display_Set_Freq_Update();
}
 
void Display_Set_Freq_Update() {
  tft.fillRoundRect(60, 60, 200, 100, 15, GRS);
  tft.setTextSize(5);
  tft.setTextColor(BLACK);
  tft.setCursor(90, 94);
  tft.print(edit_freq/100);
  tft.setTextSize(3);
  tft.print(".");
  tft.print(edit_freq%100);
}
 
void Display_Edit_Date() {
  display_mode = 1;
  tft.fillScreen(BLACK);
  tft.fillRect(0, 0, 320, 49, RED);
 
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(lebar, 15);
  tft.print("Edit Date");
  tft.drawLine(0, 50, 320, 50, WHITE);
  tft.drawLine(0, 100, 320, 100, GRS);
  tft.drawLine(0, 150, 320, 150, GRS);
  tft.drawLine(0, 200, 320, 200, GRS);
 
  tft.setTextColor(GRS);
  tft.setTextSize(2);
  tft.setCursor(0, 65);
  tft.print("Day");
  tft.setCursor(0, 115);
  tft.print("Month");
  tft.setCursor(0, 165);
  tft.print("Year");
 
  //tft.drawLine(max_width-60, 60, max_width-60, 90, GRS);
  //tft.drawLine(max_width-120, 60, max_width-120, 90, GRS);
 
  int y_start = 50;
  tft.setTextSize(3);
  tft.setTextColor(GRS);
  for (int i = 1; i < 4; i++) { tft.drawLine(max_width-60, y_start * i + 15, max_width-60, y_start * i + 35, GRS); tft.drawLine(max_width-120, y_start * i + 15, max_width-120, y_start * i + 35, GRS); tft.setCursor(max_width-40, y_start * i + 15); tft.print(">");
    tft.setCursor(max_width-100, y_start * i + 15);
    tft.print("<");
  }
 
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(0, 215);
  tft.print("Back");
  tft.setCursor(max_width-50, 215);
  tft.print("Save");
 
  DateTime now = jam.now();
  edit_hour = now.hour();
  edit_minute = now.minute();
  edit_day = now.day();
  edit_month = now.month();
  edit_year = now.year();
  Display_Edit_Date_Update();
 
}
 
void Display_Edit_Date_Update() {
  // Hapus
  tft.fillRect(lebar-10, 51, lebar, 48, BLACK);
  tft.fillRect(lebar-10, 101, lebar, 48, BLACK);
  tft.fillRect(lebar-10, 151, lebar, 48, BLACK);
   
  // Tampilkan
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(lebar-10, 65);
  tft.print(edit_day);
  tft.setCursor(lebar-10, 115);
  tft.print(edit_month);
  tft.setCursor(lebar-10, 165);
  tft.print(edit_year);
}
 
void Display_Edit_Clock() {
  display_mode = 2;
  tft.fillScreen(BLACK);
  tft.fillRect(0, 0, 320, 49, RED);
 
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(lebar, 15);
  tft.print("Edit Clock");
  tft.drawLine(0, 50, 320, 50, WHITE);
  tft.drawLine(0, 100, 320, 100, GRS);
  tft.drawLine(0, 150, 320, 150, GRS);
  tft.drawLine(0, 200, 320, 200, GRS);
 
  tft.setTextColor(GRS);
  tft.setTextSize(2);
  tft.setCursor(0, 65);
  tft.print("Hour");
  tft.setCursor(0, 115);
  tft.print("Minute");
 
  int y_start = 50;
  tft.setTextSize(3);
  tft.setTextColor(GRS);
  for (int i = 1; i < 3; i++) { tft.drawLine(max_width-60, y_start * i + 15, max_width-60, y_start * i + 35, GRS); tft.drawLine(max_width-120, y_start * i + 15, max_width-120, y_start * i + 35, GRS); tft.setCursor(max_width-40, y_start * i + 15); tft.print(">");
    tft.setCursor(max_width-100, y_start * i + 15);
    tft.print("<");
  }
 
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(0, 215);
  tft.print("Back");
  tft.setCursor(max_width-50, 215);
  tft.print("Save");
 
  DateTime now = jam.now();
  edit_hour = now.hour();
  edit_minute = now.minute();
  edit_day = now.day();
  edit_month = now.month();
  edit_year = now.year();
  Display_Edit_Clock_Update();
}
 
void Display_Edit_Clock_Update() {
  // Hapus
  tft.fillRect(lebar-10, 51, lebar, 48, BLACK);
  tft.fillRect(lebar-10, 101, lebar, 48, BLACK);
   
  // Tampilkan
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(lebar-10, 65);
  tft.print(edit_hour);
  tft.setCursor(lebar-10, 115);
  tft.print(edit_minute);
}
