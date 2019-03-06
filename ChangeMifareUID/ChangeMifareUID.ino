#include <SPI.h>
#include <MFRC522.h>
#include "CardList.h"

#define RST_PIN   9     // Configurable, see typical pin layout above
#define SS_PIN    10    // Configurable, see typical pin layout above
#define BYTE(sht, in)  (in & ((uint32_t)0xFF<<(8*sht)))>>(8*sht)

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
char menu_banner[] = "\n"
                     "******************\n"
                     "Configuration Menu\n"
                     "******************\n"
                     "Select options:\n"
                     "1: Read card number\n"
                     "2: Chage card number\n"
                     "3: Show current cards\n\n";

void setup() {
  Serial.begin(9600);
  while (!Serial);
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522 card
}

String getCardUserName(uint32_t no) {
  String user = "";
  for (int i = 0; i < sizeof(cards) / sizeof(cards[0]); i++) {
    if (no == cards[i].cardNo) {
      //Serial.println(cards[i].userName);
      user = cards[i].userName;
      break;
    }
  }
  return user;
}

uint32_t uidToCardNum(MFRC522 mf) {
  uint32_t cardNum = 0;
  for (byte i = 0; i < mf.uid.size; i++) {
    cardNum |= ((uint32_t)mf.uid.uidByte[i]) << (8 * i);
    //Serial.println(cardNum, HEX);
  }
  return cardNum;
}

int isCardPresent() {
  int timeout = 0;
  Serial.print("INFO> Bring your card closer to the sensing area.");

  while ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(500);
    timeout++;
    if (timeout % 2 == 0) {
      Serial.print(".");
    }

    if (timeout > 20) {
      Serial.println("TimeOut!");
      return -1;
    }
  }

  return 0;
}

void readCardUID() {
  if (isCardPresent() < 0) return;

  uint32_t cardno = uidToCardNum(mfrc522);
  String uname = getCardUserName(cardno);
  Serial.println();
  Serial.print("* Card No.: ");
  Serial.print(cardno);
  Serial.println(uname.equals("") ? "" : " (" + uname + ")");

  Serial.print(F("* Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();
}

void changeCardUID() {

  Serial.println("INFO> input new card number (10-digit) and press enter.(Ex: 3775000028)");
  Serial.readString(); /* __FIXME__ clean buffer, reset Serial.available() = 0 */
  delay(100);
  //Serial.println(Serial.available());

  while (true) {
    if (Serial.available() > 0) {
      String str = Serial.readStringUntil('\n');
      unsigned long newCardNum = atol(str.c_str());
      String uname = getCardUserName(newCardNum);
      Serial.print("\nINFO> prepare to write new card number:");
      Serial.print(newCardNum);
      Serial.println(uname.equals("") ? "" : " (" + uname + ")");

      byte newUid[] = { BYTE(0, newCardNum),
                        BYTE(1, newCardNum),
                        BYTE(2, newCardNum),
                        BYTE(3, newCardNum)
                      };
      //for (int i = 0; i < 4; i++) {
      //  Serial.println(newUid[i], HEX);
      //}

      if (isCardPresent() < 0) return;

      if ( mfrc522.MIFARE_SetUid(newUid, (byte)4, true) ) {
        Serial.println(F("\nINFO> Change new card number done."));
      }
      break;
    }
  }
}

void showCardList() {
  for (int i = 0; i < sizeof(cards) / sizeof(cards[0]); i++) {
    Serial.print(cards[i].userName + "\t");
    Serial.println(cards[i].cardNo);
  }
}

void showMenu() {
  Serial.print(menu_banner);
  for (;;) {
    switch (Serial.read()) {
      case '1': readCardUID(); return; break;
      case '2': changeCardUID(); return; break;
      case '3': showCardList(); return; break;
      default:
        delay(100);
        continue;  // includes the case 'no input'
    }
  }
}

void loop() {
  showMenu();
}
