#include "Arduino.h"
#include <UTFT.h>

// Constants
const char* months[] = {"January", "February", "March", "April", "May", "June",
                        "July", "August", "September", "October", "November",
                        "December"};
const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
                      "Friday", "Saturday"};
const int PADDING = 10;
const int MaxSize = 20;
const int BIG_H = 16; // BigFont height
const int BIG_W = 16; // BigFont width
const int SEG_H = 50; // Segment font height
const int SML_H = 12; // SmallFont height
const int UBU_H = 32; // Ubuntu font height
const int WTR_W = 48; // WeatherFont width
const int WARNING = 9; // Threshold warning indicator

// Fonts
extern uint8_t BigFont[]; // dim = 16 x 16
extern uint8_t SevenSegmentFull[]; // dim = 32 x 50
extern uint8_t SmallFont[]; // dim = 8 x 12
extern uint8_t Ubuntu[]; //dim = 24 x 32
extern uint8_t WeatherFont[]; // dim = 48 x 37

// Variables
bool first_ser = true;
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
  char now[] = {package[0], package[1], ':', package[2], package[3], '\0'};

  myGLCD.setFont(SevenSegmentFull);
  myGLCD.print(now, PADDING, PADDING);
}

void display_date() {
  char date[24];
  char s_day[] = {package[4], '\0'};
  char s_month[] = {package[5], package[6], '\0'};
  int day;
  int month;
  int date_index = 0;

  day = atoi(s_day);
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
  char temp[] = {package[9], package[10], '`', 'C', '\0'};

  myGLCD.setFont(Ubuntu);
  myGLCD.print(temp, PADDING, 3 * PADDING + SEG_H + SML_H);
}

void display_weather() {
  char weather[] = {package[11], '\0'};

  if (weather[0] != '_') {
    myGLCD.setFont(WeatherFont);
    myGLCD.print(weather, screen_w - 2 * PADDING - WTR_W, 2 * PADDING);
  }
}

bool warning_indicator(char a) {
  if (a == ' ') {
    return false;
  }

  int x = (int)a - '0';
  return (x >= WARNING);
}

void display_cpu_and_ram() {
  char cpu[] = {'C', 'P', 'U', ':', ' ', package[12], package[13], package[14],
                package[15], '%', '\0'};
  char ram[] = {'R', 'A', 'M', ':', ' ', package[16], package[17], package[18],
                package[19], '%', '\0'};

  myGLCD.setFont(BigFont);
  if (warning_indicator(cpu[5])) {
    myGLCD.setBackColor(VGA_RED);
  }
  myGLCD.print(cpu, PADDING, 4 * PADDING + SEG_H + SML_H + UBU_H);
  myGLCD.setBackColor(VGA_BLACK);

  if (warning_indicator(ram[5])) {
    myGLCD.setBackColor(VGA_RED);
  }
  myGLCD.print(ram, PADDING, 5 * PADDING + SEG_H + SML_H + UBU_H + BIG_H);
  myGLCD.setBackColor(VGA_BLACK);
}

void display_screen_size() {
  char screen_sz[8];
  char w[4];
  char h[4];
  int i = 0;
  String width = String(screen_w);
  String height = String(screen_h);
  width.toCharArray(w, 4);
  height.toCharArray(h, 4);
  for (int j = 0; j < strlen(w); j++) {
    screen_sz[i] = w[j];
    i++;
  }
  screen_sz[i] = 'x';
  i++;
  for (int j = 0; j < strlen(h); j++) {
    screen_sz[i] = h[j];
    i++;
  }
  screen_sz[i] = '\0';
  myGLCD.setFont(BigFont);
  myGLCD.print(screen_sz, CENTER, screen_h / 2);
}

void update_display() {
  display_time();
  display_date();
  display_temp();
  display_weather();
  display_cpu_and_ram();
}

void initial_display() {
  display_wait_msg();
  display_screen_size();
}

void setup() {
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(VGA_WHITE);

  Serial.begin(9600);

  screen_h = myGLCD.getDisplayYSize();
  screen_w = myGLCD.getDisplayXSize();

  initial_display();
}

void loop() {}

void serialEvent() {
  if (first_ser) {
    myGLCD.clrScr();
    first_ser = false;
  }
  while (Serial.available()) {
    char ch = Serial.read();
    if (index < MaxSize && ch != '!' && ch != '*') {
      package[index] = ch;
      index++;
    } else if (ch == '*') {
      myGLCD.clrScr();
      initial_display();
      index = 0;
      first_ser = true;
    }else {
      update_display();
      index = 0;
    }
  }
}
