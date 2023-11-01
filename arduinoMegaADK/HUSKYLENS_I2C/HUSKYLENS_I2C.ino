#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11);

#include <DFPlayer_Mini_Mp3.h>

#include <Adafruit_NeoPixel.h>
#define LED 8
#define LED_COUNT 15
Adafruit_NeoPixel strip(LED_COUNT, LED, NEO_GRBW + NEO_KHZ800);

char state;

int count = 0;

#include "HUSKYLENS.h"
HUSKYLENS huskylens;
//HUSKYLENS green line >> SDA; blue line >> SCL
void printResult(HUSKYLENSResult result);

#include <LiquidCrystal.h>
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
LiquidCrystal lcd2(39, 41, 43, 45, 47, 49);

int flag = 0;


void setup() {
  //LCD
  lcd.begin(16, 2);
  lcd2.begin(16, 2);
  lcd.clear();   // 화면 지우기
  lcd2.clear();  // 화면 지우기
  Serial.begin(9600);
  mySerial.begin(9600);

  //네오픽셀
  strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();             // 네오픽셀에 빛을 출력하기 위한 것인데 여기서는 모든 네오픽셀을 OFF하기 위해서 사용한다.
  strip.setBrightness(50);  // 네오픽셀의 밝기 설정(최대 255까지 가능)

  //허스키렌즈
  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }

  //DFPlayer
  Serial3.begin(9600);
  mp3_set_serial(Serial3);
  delay(1);
  mp3_set_volume(30);

  //mp3_play(2); //장애인탑승예정이니 비켜
  //mp3_play(2);
}

void loop() {

  mySerial.listen();
  if (mySerial.available() > 0) {
    while (mySerial.available() > 0) {
      Serial.print("mySerial.listen() : ");
      Serial.println(mySerial.listen());
      state = mySerial.read();  //수신된 데이터를 1byte 읽어와서 state 변수에 저장함
    }
    Serial.print("state: ");
    Serial.println(state);  //수신되었는지 상태 확인
    if (state == '1') {
      flag = 0;
      if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
      else if (!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
      else if (!huskylens.available()) {
        Serial.println(F("No block or arrow appears on the screen!"));
      } else {
        //Serial.println(F("###########"));
        while (huskylens.available()) {
          HUSKYLENSResult result = huskylens.read();
          flag = 0;
          printResult(result);
          // for (int i = 0; i < 1; i++) {
          //   neo_on();
          //   neo_off();
          // }
          if (flag == 0) {
            mp3_play(1);
            for (int i = 0; i < 6; i++) {
              neo_on();
              neo_off();
            }
            mp3_play(2);
            for (int i = 0; i < 4; i++) {
              neo_on();
              neo_off();
            }
          } else {
            mp3_play(1);
            delay(9500);
            mp3_play(2);
          }
        }
      }
    }
  }
  flag = 0;
}


//객차내 휠체어 전용좌석 네오픽셀
void neo_on() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 0));
    strip.show();
    delay(30);
  }
  delay(500);
}

void neo_off() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 0, 0, 0);
    strip.show();
    delay(30);
  }
  delay(1);
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    if (result.ID == 3) {
      flag = 1;
      //2번아두이노에 값전송
      Serial.println("5번칸으로 이동하세요");
      lcd_print(5);
      lcd2_print(5);
      mp3_play(4);
      delay(3800);
    } else {
      Serial.println("3번칸으로 이동하세요");
      lcd_print(3);
      lcd2_print(3);
      mp3_play(3);
      delay(3800);
    }
  } else if (result.command == COMMAND_RETURN_ARROW) {
  }
}

void lcd_print(int car) {
  lcd.setCursor(0, 0);
  lcd.print("Go to car");
  lcd.setCursor(0, 1);
  lcd.print("No.");
  lcd.print(car);
}

void lcd2_print(int car) {
  lcd2.setCursor(0, 0);
  lcd2.print("Go to car");
  lcd2.setCursor(0, 1);
  lcd2.print("No.");
  lcd2.print(car);
}