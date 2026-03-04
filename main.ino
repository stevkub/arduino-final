#include <LiquidCrystal.h>

// lcd pins shifted 1 back
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// buzzer pin - needed to put on bottom not enough space on sides
const int buzzer = 52;

// start time (until alarm)
const int START_SECONDS = 30;
int secondsLeft = START_SECONDS;
unsigned long lastSecondTick = 0;

// alarm control
bool alarmActive = false;

void showCountdown(int secs) {
  lcd.setCursor(0, 0);
  lcd.print("countdown:");

  lcd.setCursor(0, 1);
  lcd.print(secs);
  lcd.print(" sec      ");
}

void setup() {

  pinMode(buzzer, OUTPUT);

  // start lcd
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("starting...");
  delay(800);

  // start timer
  secondsLeft = START_SECONDS;
  lastSecondTick = millis();

  lcd.clear();
  showCountdown(secondsLeft);
}

void loop() {

  unsigned long now = millis();

  // countdown
  if (!alarmActive && now - lastSecondTick >= 1000) {
    lastSecondTick += 1000;
    secondsLeft--;

    showCountdown(secondsLeft);

    if (secondsLeft <= 0) {
      alarmActive = true;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("time up!");
      lcd.setCursor(0, 1);
      lcd.print("alarm on");
    }
  }

  // buzzer alarm
  if (alarmActive) {
    tone(buzzer, 1000); // 1khz
  }
}
