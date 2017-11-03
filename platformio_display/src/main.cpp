#include "Arduino.h"
#include <UTFT.h>
#include <URTouch.h>

// Constants
const char* months[] = {"January", "February", "March", "April", "May", "June",
                        "July", "August", "September", "October", "November",
                        "December"};
const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
                      "Friday", "Saturday"};
const int PADDING = 10;
const int MAXSIZE = 20;
const int BIG_H = 16; // BigFont height
const int BIG_W = 16; // BigFont width
const int PWR_D = 48; // PowerButton width and height
const int SEG_H = 50; // Segment font height
const int SML_H = 12; // SmallFont height
const int UBU_H = 32; // Ubuntu font height
const int WTR_W = 48; // WeatherFont width
const int WARNING = 9; // Threshold warning indicator

// Fonts
extern uint8_t BigFont[]; // dim = 16 x 16
extern uint8_t PowerButton[]; // dim = 48 x 48
extern uint8_t SevenSegmentFull[]; // dim = 32 x 50
extern uint8_t SmallFont[]; // dim = 8 x 12
extern uint8_t Ubuntu[]; //dim = 24 x 32
extern uint8_t WeatherFont[]; // dim = 48 x 37

// Variables
bool first_ser = true; // First serial package
char package[MAXSIZE];
int index = 0;
int screen_h;
int screen_w;
int touch_x;
int touch_y;
int pwr_ux; // pwr button upper x coord
int pwr_uy; // pwr button upper y coord

// Objects
UTFT myGLCD(SSD1289, 38, 39, 40, 41);
URTouch myTouch(6, 5, 4, 3, 2);

void display_msg(int i) {
  char *messages[] = {"Waiting...", "Connecting..."};

  myGLCD.setFont(BigFont);
  myGLCD.print(messages[i], PADDING, PADDING);
}

void test_display() {
  // Display the package as it was received for testing purposes
  myGLCD.setFont(BigFont);
  myGLCD.print(package, PADDING, PADDING);
}

void display_info(char *msg) {
  //  Display a message on the bottom of the screen
  myGLCD.setFont(BigFont);
  myGLCD.print(msg, PADDING, screen_h - PADDING - BIG_H);
}

void display_time() {
  // Unpack and display the current time
  char now[] = {package[0], package[1], ':', package[2], package[3], '\0'};

  myGLCD.setFont(SevenSegmentFull);
  myGLCD.print(now, PADDING, PADDING);
}

void display_date() {
  // Unpack, find appropriate day and month, and display the current date
  int buf_size = 24;
  char date[buf_size];
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

  while (date_index < buf_size - 1) {
    date[date_index] = ' '; // Fill with spaces to overwrite older characters
    date_index++;
  }

  date[date_index] = '\0';

  myGLCD.setFont(SmallFont);
  myGLCD.print(date, PADDING, 2 * PADDING + SEG_H);
}

void display_temp() {
  // Unpack and display the current temperature
  char temp[] = {package[9], package[10], package[11], '`', 'C', '\0'};

  myGLCD.setFont(Ubuntu);
  myGLCD.print(temp, PADDING, 3 * PADDING + SEG_H + SML_H);
}

void display_weather() {
  // Unpack and display the current weather (icon font)
  char weather[] = {package[12], '\0'};

  if (weather[0] != '_') {
    myGLCD.setFont(WeatherFont);
    myGLCD.print(weather, screen_w - 2 * PADDING - WTR_W, 2 * PADDING);
  }
}

bool warning_indicator(char a) {
  // Determine if the indicator condition is true
  if (a == ' ') {
    return false;
  }

  int x = (int)a - '0';
  return (x >= WARNING);
}

void display_cpu_and_ram() {
  /* Unpack and display ram; change background color if indicator warning
  condition is true
  */
  char cpu[] = {'C', 'P', 'U', ':', ' ', package[13], package[14], package[15],
                package[16], '%', '\0'};
  char ram[] = {'R', 'A', 'M', ':', ' ', package[17], package[18], package[19],
                package[20], '%', '\0'};

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
  // Display the screen size (w x h) on the center of the screen
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

void display_power_button() {
  myGLCD.setFont(PowerButton);
  myGLCD.print("A", pwr_ux, pwr_uy);
}

void update_display() {
  display_time();
  display_date();
  display_temp();
  display_weather();
  display_cpu_and_ram();
  display_power_button();
}

void initial_display() {
  display_msg(0);
  display_screen_size();
}

void setup() {
  myGLCD.InitLCD();
  myGLCD.clrScr();

  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);

  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(VGA_WHITE);

  Serial.begin(9600);

  screen_h = myGLCD.getDisplayYSize();
  screen_w = myGLCD.getDisplayXSize();
  pwr_ux = screen_w - 2 * PADDING - PWR_D;
  pwr_uy = screen_h - 2 * PADDING - PWR_D;

  initial_display();
}

void loop() {
  if (myTouch.dataAvailable()) {
    myTouch.read();
    touch_x = myTouch.getX();
    touch_y = myTouch.getY();
    Serial.println(touch_x);
    Serial.println(touch_y);
    if ((touch_x >= pwr_ux) && (touch_y >= pwr_uy)) {
      if ((touch_x <= pwr_ux + PWR_D) && (touch_y <= pwr_uy + PWR_D)) {
        Serial.println("BTN");
      }
    }
    delay(1000);
  }
}

void serialEvent() {
  if (first_ser) {
    myGLCD.clrScr();
    first_ser = false;
  }
  while (Serial.available()) {
    char ch = Serial.read();
    if (ch == '*') {
      myGLCD.clrScr();
      initial_display();
      index = 0;
      first_ser = true;
    } else if (ch == '#') {
      display_msg(1);
    } else if (index < MAXSIZE && ch != '!') {
      package[index] = ch;
      index++;
    } else {
      update_display();
      index = 0;
    }
  }
}
