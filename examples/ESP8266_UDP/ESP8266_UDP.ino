/*
    Accepts UDP packets containing the full bitmap.
    Each column is represented by a byte, so the number
    of bytes in the packet equals the total number of
    columns of the display. Each byte contains the 7 values
    for that row with the MSB being ignored.

    Column ordering is top-left to bottom-right.
*/

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <FlipDotMatrix.h>

#ifndef WIFIAUTH
// Change to a valid Wi-Fi connection
#define SSID "SSID_HERE"
#define PASS "PASSWORD_HERE"
#endif

const char *ssid = SSID;
const char *password = PASS;

WiFiUDP Udp;
unsigned int listenPort = 4210;

// two 7x28 panels
#define ROWS_IN_COL 7
#define PACKET_SIZE 28 * 2
uint8_t incomingPacket[PACKET_SIZE];

// 28x7 panel type, two panels total, 1 panel per row, 57600 baud rate
FlipDotMatrix matrix = FlipDotMatrix(FlipDotController::PanelType::p28x7, 2, 1, &Serial, 57600);

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Udp.begin(listenPort);
  matrix.start();
}

void acceptPackets() {
    if (Udp.parsePacket() != PACKET_SIZE) {
      return;
    }

    int len = Udp.read(incomingPacket, PACKET_SIZE);
    if (len != PACKET_SIZE) {
      return;
    }

    matrix.fillScreen(0);

    // expected format: top-left to bottom-right packed byte columns
    // 7 of the 8 bits are used to represent the 7 rows in a column
    for (int col = 0; col < PACKET_SIZE; col++) {
      uint8_t x = col % matrix.width();
      uint8_t yOffset = (col / matrix.width()) * ROWS_IN_COL;

      for (int y = 0; y < ROWS_IN_COL; y++) {
        uint8_t *colPtr = incomingPacket + col;
        matrix.drawPixel(x, y + yOffset, *colPtr & (1 << y));
      }
    }

    matrix.show();
}

void loop() {
  acceptPackets();
  delay(50);
}
