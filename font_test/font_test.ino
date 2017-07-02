#include <UTFT.h>

// Constants
const char* CHARS[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K"};
const int PADDING = 10;
const int CHAR_CNT = 11;

// Create Objects
UTFT myGLCD(SSD1289, 38, 39, 40, 41);

// Define Variables
extern uint8_t WeatherFont[]; // dim = 48 x 37

void setup() {
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setFont(WeatherFont);
}

void loop() {
  for (int i = 0; i < CHAR_CNT; i++) {
    myGLCD.print(CHARS[i], PADDING, PADDING);
    delay(5000);
  }
}
