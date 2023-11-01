//1번 아두이노
//카드인식
//카드판별
//휠체어 이용 유무 확인
//2번아두이노에 값 전송

//--------------------------------------------------------------------//
//NFC
#include <deprecated.h>
#include <require_cpp11.h>
#include <SPI.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#define SS_PIN 10
#define RST_PIN 7

MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class

MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[4];

int count = 0;
//--------------------------------------------------------------------//
//허스키렌즈
#include "HUSKYLENS.h"

HUSKYLENS huskylens;

//HUSKYLENS green line >> SDA; blue line >> SCL
void printResult(HUSKYLENSResult result);

//--------------------------------------------------------------------//
//2번아두이노 값전송
#include "SoftwareSerial.h"

SoftwareSerial myArduino(2, 3);

//--------------------------------------------------------------------//
//피에조
int bz = 4;
int nTones = 2735;
void setup() {
  //피에조
  pinMode(bz, OUTPUT);
  // put your setup code here, to run once:
  //2번아두이노 값전송
  myArduino.begin(9600);
  //NFC
  Serial.begin(9600);
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

  //허스키렌즈
  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  //NFC
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial())
    return;



  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  //Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    //Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  //NFC인식시

  if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || rfid.uid.uidByte[3] != nuidPICC[3]) {


    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }


    if (rfid.uid.uidByte[0] == 144) {  //장애인카드일때
      Serial.println("장애인 전용 카드입니다");
      tone(bz, nTones, 80);
      //--------Huskylens----------------------------------------------------//
      if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
      else if (!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
      else if (!huskylens.available()) {
        Serial.println(F("No block or arrow appears on the screen!"));
      } else {
        //Serial.println(F("###########"));
        while (huskylens.available()) {
          HUSKYLENSResult result = huskylens.read();
          printResult(result);
        }
      }
      delay(1000);
    }

    else if (rfid.uid.uidByte[0] == 211) {  //일반인카드일때
      Serial.println("비장애인 카드입니다");
      tone(bz, nTones, 80);
    }
  }
  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

void printResult(HUSKYLENSResult result) {

  if (result.command == COMMAND_RETURN_BLOCK) {
    if (result.ID == 3) {
      char data = '1';
      //2번아두이노에 값전송
      myArduino.write(data);
      if (myArduino.write(data) == 1) count++;
      Serial.println(myArduino.write(data));
      delay(100);
    } else {
      Serial.println("값 못넘김");
      myArduino.write('0');
    }
  } else if (result.command == COMMAND_RETURN_ARROW) {
  }
}