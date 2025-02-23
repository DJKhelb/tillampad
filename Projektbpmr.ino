#include "U8glib.h"
#include <Wire.h>
#include <RTClib.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);
RTC_DS3231 rtc;

const int buttonPin = 0;
const int resetPin = 1;

bool started = false;
int count = 0;
unsigned long startTime = 0;
unsigned long totalTime = 0;
bool clicked = false;
float bpm = 0.0;

float highestBPM = 0.0;
float lowestBPM = 0.0;

unsigned long lastResetPress = 0;
const int debounceDelay = 200;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(resetPin, INPUT_PULLUP);
  Wire.begin();
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  u8g.setFont(u8g_font_5x8);
  oledWrite("BPM:  0", "Time:  0:00", "Highest:  0", "Lowest:  0");
}

void loop() {
  if (started) {
    totalTime = millis() - startTime;
  }

  if (digitalRead(buttonPin) == LOW && !clicked) {
    unsigned long currentTime = millis();

    if (!started) {
      started = true;
      startTime = currentTime;
    } else {
      count++;
      float elapsedTime = (currentTime - startTime) / 60000.0;

      if (elapsedTime > 0) {
        bpm = count / elapsedTime;
      }
    }

    if (bpm > highestBPM) {
      highestBPM = bpm;
    }

    if (count == 1) {
      lowestBPM = bpm;
    } else if (bpm < lowestBPM) {
      lowestBPM = bpm;
    }

    oledWrite("BPM: " + String(bpm, 0), formatTime(totalTime), 
              "Highest: " + String(highestBPM, 0), 
              "Lowest: " + String(lowestBPM, 0));

    clicked = true;
  }

  if (digitalRead(buttonPin) == HIGH) {
    clicked = false;
  }

  if (digitalRead(resetPin) == LOW) {
    delay(50);
    if (digitalRead(resetPin) == LOW && millis() - lastResetPress > debounceDelay) {
      resetBPM();
      lastResetPress = millis();
    }
  }
}

void resetBPM() {
  started = false;
  count = 0;
  bpm = 0.0;
  highestBPM = 0.0;
  lowestBPM = 0.0;
  startTime = 0;
  totalTime = 0;
  oledWrite("BPM:  0", "Time:  0:00", "Highest:  0", "Lowest:  0");
}

String formatTime(unsigned long milliseconds) {
  unsigned long totalSeconds = milliseconds / 1000;
  unsigned long minutes = totalSeconds / 60;
  unsigned long seconds = totalSeconds % 60;
  String timeStr = "Time:  " + String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);
  return timeStr;
}

void oledWrite(String bpmText, String timeText, String highestText, String lowestText) {
  u8g.firstPage();
  do {
    u8g.drawStr(0, 10, bpmText.c_str());
    u8g.drawStr(0, 20, timeText.c_str());
    u8g.drawStr(0, 30, highestText.c_str());
    u8g.drawStr(0, 40, lowestText.c_str());
  } while (u8g.nextPage());
}
