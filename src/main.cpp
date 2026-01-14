#include <Arduino.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <Wire.h>

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

void setup() {
  Serial.begin(115200);
  nfc.begin();
  nfc.SAMConfig();
  // Alacsonyabb retries, hogy ne akadjon össze a telefonnal
  nfc.setPassiveActivationRetries(0x01); 
  Serial.println("Várakozás telefonra...");
}

void loop() {
  uint8_t success;
  uint8_t response[64];
  uint8_t responseLength = 64;

  // 1. Megpróbáljuk listázni a célpontot
  success = nfc.inListPassiveTarget();

  if (success) {
    // Ha ide eljutunk, van valami az olvasónál (kártya vagy telefon)
    
    // 2. Próbáljuk meg elküldeni a SELECT AID-et
    uint8_t selectAid[] = { 
      0x00, /* CLA */
      0xA4, /* INS: Select File */
      0x04, /* P1: Select by AID */
      0x00, /* P2: First or only occurrence */
      0x07, /* Lc: AID hossza (7 bájt) */
      0xA0, 0x00, 0x00, 0x02, 0x47, 0x10, 0x01, /* Az AID-ed */
      0x00  /* Le: Várható válaszhossz nincs meghatározva */
    };
    
    if (nfc.inDataExchange(selectAid, sizeof(selectAid), response, &responseLength)) {
      // SIKER: Ez egy telefon, ami válaszolt az AID-re
      Serial.print("TELEFON DETEKTÁLVA. Válasz: ");
      for (uint8_t i = 0; i < responseLength; i++) {
        Serial.print(response[i], HEX); Serial.print(" ");
      }
      Serial.println();
    } 
    else {
      // HIBA az adatcserében: Ez valószínűleg egy sima kártya, nem egy HCE telefon
      // Ilyenkor lekérjük az UID-t a PN532-től
      Serial.println("SIMA KÁRTYA DETEKTÁLVA.");
      
      uint8_t uid[7]; 
      uint8_t uidLength;
      
      // Megjegyzés: A Seeed könyvtárban az inListPassiveTarget után 
      // az UID már a belső pufferben van, de a biztonság kedvéért 
      // lekérhetjük a readPassiveTargetID-vel is, ha a kártyát ott tartják.
      if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        Serial.print("Kártya UID: ");
        for (uint8_t i = 0; i < uidLength; i++) {
          Serial.print(uid[i], HEX); Serial.print(" ");
        }
        Serial.println();
      }
    }
    
    // Megvárjuk, amíg elviszik az eszközt, hogy ne loopoljon azonnal
    delay(1000);
  }
}