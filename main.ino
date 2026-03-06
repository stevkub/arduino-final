#include "rfid1.h"
#include <LiquidCrystal.h>
#include <Servo.h>
#include <DHT.h>

RFID1 rfid;
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
Servo servo1;

#define DHTPIN 32
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const int buzzer = 52;
const int tempPin = A1;
const int servoPin = 50;

const int red = 22;
const int green = 24;
const int blue = 28;

const int startTime = 5; // starting countdown

enum State { COUNTDOWN, ALARM, TEMP };
State state = COUNTDOWN;

int timeLeft = startTime;
unsigned long lastTick = 0;

unsigned long lastBuzz = 0;
bool buzzOn = false;

int pos = 0;
int dir = 1;
unsigned long lastServo = 0;

int color = 0;
unsigned long lastColor = 0;

void setColor(int r, int g, int b)
{
  digitalWrite(red, r); // red led
  digitalWrite(green, g); // green led
  digitalWrite(blue, b); // blue led
}

void updateLed()
{
  if (millis() - lastColor > 600) // change color every like 0.6s
  {
    lastColor = millis();
    color++;

    if (color > 5)
    {
      color = 0; // loop colors
    }

    switch (color)
    {
      case 0: setColor(1,0,0); break; // red
      case 1: setColor(0,1,0); break; // green
      case 2: setColor(0,0,1); break; // blue
      case 3: setColor(1,1,0); break; // yellow
      case 4: setColor(0,1,1); break; // cyan
      case 5: setColor(1,0,1); break; // purple
    }
  }
}

void updateServo()
{
  if (millis() - lastServo > 10) // move servo often
  {
    lastServo = millis();

    pos = pos + dir * 3; // move position

    if (pos >= 180)
    {
      pos = 180;
      dir = -1; // reverse direction
    }

    if (pos <= 0)
    {
      pos = 0;
      dir = 1; // reverse direction
    }

    servo1.write(pos); // send angle to servo
  }
}

void showTime(int t)
{
  lcd.setCursor(0, 0); // first row
  lcd.print("countdown:");

  lcd.setCursor(0, 1); // second row
  lcd.print(t);
  lcd.print(" sec   "); // spaces clear old numbers
}

float getTemp()
{
  float v = analogRead(tempPin) * (5.0 / 1023.0); // read voltage
  float c = (v - 0.5) * 100.0; // convert to celsius
  return c * 9.0 / 5.0 + 32.0; // convert to fahrenheit
}

float getHum()
{
  float h = dht.readHumidity(); // read humidity

  if (isnan(h))
  {
    return -1; // sensor failed
  }

  return h;
}

bool scanCard()
{
  uchar status;
  uchar str[MAX_LEN];

  status = rfid.request(PICC_REQIDL, str); // look for card
  if (status != MI_OK) return false;

  status = rfid.anticoll(str); // read card id
  if (status != MI_OK) return false;

  Serial.print("card: ");
  for (int i = 0; i < 4; i++)
  {
    Serial.print(str[i], HEX); // print id bytes
    Serial.print(" ");
  }
  Serial.println();

  rfid.halt(); // stop card
  return true;
}

void setup()
{
  Serial.begin(9600);

  pinMode(buzzer, OUTPUT); // buzzer output

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT); // rgb led pins

  servo1.attach(servoPin); // connect servo
  servo1.write(90); // start in middle

  dht.begin(); // start humidity sensor

  lcd.begin(16, 2); // lcd size
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("starting...");
  delay(800);

  rfid.begin(9, 12, 11, 10, 13, 8); // start rfid
  delay(100);
  rfid.init();

  timeLeft = startTime;
  lastTick = millis(); // start timer

  lcd.clear();
  showTime(timeLeft); // show first time
}

void loop()
{
  unsigned long now = millis();

  if (state == COUNTDOWN)
  {
    if (now - lastTick >= 1000) // every second
    {
      lastTick = now;
      timeLeft--; // decrease timer

      showTime(timeLeft);

      if (timeLeft <= 0)
      {
        state = ALARM;

        lcd.clear();
        lcd.setCursor(0, 0); // line 1
        lcd.print("time up!");
        lcd.setCursor(0, 1); // line 2
        lcd.print("scan rfid");

        lastBuzz = now;
        buzzOn = false;
      }
    }
  }
  else if (state == ALARM)
  {
    updateServo(); // sweep servo
    updateLed(); // change led color

    if (now - lastBuzz >= 200) // buzzer toggle
    {
      lastBuzz = now;
      buzzOn = !buzzOn;

      if (buzzOn)
      {
        tone(buzzer, 1000); // beep
      }
      else
      {
        noTone(buzzer); // stop beep
      }
    }

    if (scanCard()) // check rfid
    {
      noTone(buzzer);

      servo1.write(90); // stop servo
      setColor(0,0,0); // leds off

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("alarm stopped");

      delay(700);

      state = TEMP; // move to sensor display
      lcd.clear();
    }
  }
  else if (state == TEMP)
  {
    float temp = getTemp(); // read temp
    float hum = getHum(); // read humidity

    lcd.setCursor(0, 0);
    lcd.print("temp f: ");
    lcd.print(temp, 1);
    lcd.print("   ");

    lcd.setCursor(0, 1);
    lcd.print("hum: ");

    if (hum < 0)
    {
      lcd.print("error   "); // sensor error
    }
    else
    {
      lcd.print(hum, 0);
      lcd.print("%   ");
    }

    delay(1000); // update every second
  }
}
