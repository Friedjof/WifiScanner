#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <I2S.h>
#include <Wire.h>
#include <sqlite3.h>
#include <WiFi.h>

#define SAMPLE_RATE 16000U
#define SAMPLE_BITS 16

void insert_into_database(const char* ssid, const char* bssid, int32_t rssi, const char* encryptionType, int32_t channel);
void wifi_scanner();


void setup() {
  Serial.begin(115200);

  I2S.setAllPins(-1, 42, 41, -1, -1);
  if (!I2S.begin(PDM_MONO_MODE, SAMPLE_RATE, SAMPLE_BITS)) {
    Serial.println("Failed to initialize I2S!");
    while (1) {
      delay(10);
      Serial.println("Failed to initialize I2S!");
    }
  }
  Serial.println("I2S initialized");

  // Initialize the MicroSD card
  if (!SD.begin(21)) {
    Serial.println("Failed to mount MicroSD Card!");
    while (1) {
      delay(10);
      Serial.println("Failed to mount MicroSD Card!");
    }
  }
  Serial.println("MicroSD Card mounted");

  Serial.println("Setup done");
}

void loop() {
  wifi_scanner();
  Serial.println("Waiting 4 seconds...");

  delay(4000);
}

void wifi_scanner() {
  Serial.println("Scanning WiFi networks...");

  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    String ssid = WiFi.SSID(i);

    String encryptionType;
    switch (WiFi.encryptionType(i)) {
      case WIFI_AUTH_OPEN:
        encryptionType = "Open";
        break;
      case WIFI_AUTH_WEP:
        encryptionType = "WEP";
        break;
      case WIFI_AUTH_WPA_PSK:
        encryptionType = "WPA_PSK";
        break;
      case WIFI_AUTH_WPA2_PSK:
        encryptionType = "WPA2_PSK";
        break;
      case WIFI_AUTH_WPA_WPA2_PSK:
        encryptionType = "WPA_WPA2_PSK";
        break;
      case WIFI_AUTH_WPA2_ENTERPRISE:
        encryptionType = "WPA2_ENTERPRISE";
        break;
      default:
        encryptionType = "Unknown";
        break;
    }

    int32_t rssi = WiFi.RSSI(i);
    String bssid = WiFi.BSSIDstr(i);
    int32_t channel = WiFi.channel(i);

    Serial.printf(">> SSID: %s, BSSID: %s, RSSI: %d, Encryption: %s, Channel: %d\n", ssid.c_str(), bssid.c_str(), rssi, encryptionType.c_str(), channel);

    // Einfügen der Daten in die Datenbank
    insert_into_database(ssid.c_str(), bssid.c_str(), rssi, encryptionType.c_str(), channel);

    delay(10);
  }
  delay(1000);
}

void insert_into_database(const char* ssid, const char* bssid, int32_t rssi, const char* encryptionType, int32_t channel) {
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;

  // Open database
  rc = sqlite3_open("/sd/wifi.sqlite3", &db);
  if (rc) {
    Serial.printf("Can't open database: %s\n", sqlite3_errmsg(db));
    return;
  }

  // Create SQL statement
  char sql[200];
  snprintf(sql, sizeof(sql), "INSERT INTO wifis (ssid,bssid,rssid,encryption,channel) VALUES ('%s','%s',%d,'%s',%d);", ssid, bssid, rssi, encryptionType, channel);

  // Execute SQL statement
  rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    Serial.printf("SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  sqlite3_close(db);
}
