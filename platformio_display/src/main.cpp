#include "Arduino.h"
#include <UTFT.h>

// Constants
const char* months[] = {"January",
"February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const int PADDING = 10;
const int MaxSize = 20;
const int BIG_H = 16; // BigFont height
const int BIG_W = 16; // BigFont width
const int SEG_H = 50; // Segment font height
const int SML_H = 12; // SmallFont height
const int UBU_H = 32; // Ubuntu font height
const int WTR_W = 48; // WeatherFont width

// Fonts
extern uint8_t BigFont[]; // dim = 16 x 16
extern uint8_t SevenSegmentFull[]; // dim = 32 x 50
extern uint8_t SmallFont[]; // dim = 8 x 12
extern uint8_t Ubuntu[]; //dim = 24 x 32
extern uint8_t WeatherFont[]; // dim = 48 x 37

// Variables
char package[MaxSize];
int index = 0;
int screen_h;
int screen_w;

// Objects
UTFT myGLCD(SSD1289, 38, 39, 40, 41);

void display_wait_msg() {
  char waiting[] = "Waiting...";
  myGLCD.setFont(BigFont);
  myGLCD.print(waiting, PADDING, PADDING);
}

void test_display() {
  myGLCD.setFont(BigFont);
  myGLCD.print(package, PADDING, PADDING);
}

void display_info(char *msg) {
  myGLCD.setFont(BigFont);
  myGLCD.print(msg, PADDING, screen_h - PADDING - BIG_H);
}

void display_time() {
  char now[6];
  now[0] = package[0];
  now[1] = package[1];
  now[2] = ':';
  now[3] = package[2];
  now[4] = package[3];
  now[5] = '\0';

  myGLCD.setFont(SevenSegmentFull);
  myGLCD.print(now, PADDING, PADDING);
}

void display_date() {
  char date[24];
  char s_day[2];
  char s_month[3];
  int day;
  int month;
  int date_index = 0;

  s_day[0] = package[4];
  s_day[1] = '\0';
  day = atoi(s_day);

  s_month[0] = package[5];
  s_month[1] = package[6];
  s_month[2] = '\0';
  month = atoi(s_month) - 1;

  for (int i = 0; i < strlen(days[day]); i++) {
    date[i] = days[day][i];
    date_index++;
  }

  date[date_index] = ',';
  date_index++;
  date[date_index] = ' ';
  date_index++;

  for (int i = 0; i < strlen(months[month]); i++) {
    date[date_index] = months[month][i];
    date_index++;
  }

  date[date_index] = ' ';
  date_index++;
  date[date_index] = package[7];
  date_index++;
  date[date_index] = package[8];
  date_index++;
  date[date_index] = '\0';

  myGLCD.setFont(SmallFont);
  myGLCD.print(date, PADDING, 2 * PADDING + SEG_H);
}

void display_temp() {
  char temp[5];
  temp[0] = package[9];
  temp[1] = package[10];
  temp[2] = '`';
  temp[3] = 'C';
  temp[4] = '\0';

  myGLCD.setFont(Ubuntu);
  myGLCD.print(temp, PADDING, 3 * PADDING + SEG_H + SML_H);
}

void display_weather() {
  char weather[2];
  weather[0] = package[11];
  weather[1] = '\0';

  myGLCD.setFont(WeatherFont);
  myGLCD.print(weather, screen_w - PADDING - WTR_W, PADDING);
}

void display_cpu_and_ram() {
  char cpu_label[] = "CPU: ";
  char ram_label[] = "RAM: ";
  char cpu[6];
  char ram[6];

  cpu[0] = package[12];
  cpu[1] = package[13];
  cpu[2] = package[14];
  cpu[3] = package[15];
  cpu[4] = '%';
  cpu[5] = '\0';

  ram[0] = package[16];
  ram[1] = package[17];
  ram[2] = package[18];
  ram[3] = package[19];
  ram[4] = '%';
  ram[5] = '\0';

  myGLCD.setFont(BigFont);
  myGLCD.print(cpu_label, PADDING, 4 * PADDING + SEG_H + SML_H + UBU_H);
  myGLCD.print(cpu, PADDING + strlen(cpu_label) * BIG_W, 4 * PADDING + SEG_H + SML_H + UBU_H);
  myGLCD.print(ram_label, PADDING, 5 * PADDING + SEG_H + SML_H + UBU_H + BIG_H);
  myGLCD.print(ram, PADDING + strlen(ram_label) * BIG_W, 5 * PADDING + SEG_H + SML_H + UBU_H + BIG_H);
}

void update_display() {
  display_time();
  display_date();
  display_temp();
  display_weather();
  display_cpu_and_ram();
}

void setup() {
  myGLCD.InitLCD();
  myGLCD.clrScr();
  Serial.begin(9600);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(VGA_WHITE);
  display_wait_msg();
  screen_h = myGLCD.getDisplayYSize();
  screen_w = myGLCD.getDisplayXSize();
}

void loop() {}

void serialEvent() {
  while (Serial.available()) {
    char ch = Serial.read();
    if (index < MaxSize && ch != '!' && ch != '*') {
      package[index] = ch;
      index++;
    } else if (ch == '*') {
      myGLCD.clrScr();
      display_wait_msg();
      index = 0;
    }else {
      update_display();
      index = 0;
    }
  }
}
