#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// === Pin Setup ===
#define SS_PIN 10
#define RST_PIN 9
#define BUZZER_PIN 8

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 20, 4); // Try 0x3F if needed

float totalBill = 0.0;

// === Product Database ===
struct Product {
  byte uid[4];
  String name;
  float price;
};

// Replace with your actual UIDs
Product products[] = {
  {{0x4F, 0xB6, 0xB0, 0xDE}, "Product 1", 2.0},
  {{0x5F, 0x18, 0xB2, 0xDE}, "Product 2", 3.5},
  {{0x94, 0x02, 0x3D, 0x05}, "Product 3", 5.0},
  {{0xEE, 0x3C, 0x39, 0x05}, "Product 4", 7.5}
};
const int numProducts = sizeof(products) / sizeof(products[0]);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Billing");
  lcd.setCursor(0, 1);
  lcd.print("Scan your items");

  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  byte readUID[4];
  for (byte i = 0; i < 4; i++) {
    readUID[i] = rfid.uid.uidByte[i];
  }

  bool found = false;
  for (int i = 0; i < numProducts; i++) {
    if (compareUID(readUID, products[i].uid)) {
      totalBill += products[i].price;
      beep(1); // ✅ Valid item = 1 beep

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(products[i].name);
      lcd.setCursor(0, 1);
      lcd.print("Price: $");
      lcd.print(products[i].price, 2);
      lcd.setCursor(0, 2);
      lcd.print("Total: $");
      lcd.print(totalBill, 2);

      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Scan next item");
      found = true;
      break;
    }
  }

  if (!found) {
    beep(2); // ❌ Unknown tag = 2 quick beeps
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unknown item");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan next item");
  }

  rfid.PICC_HaltA();
}

// === Compare two UIDs ===
bool compareUID(byte *a, byte *b) {
  for (byte i = 0; i < 4; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

// === Beep Buzzer (count = how many beeps) ===
void beep(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}
