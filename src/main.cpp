#include <Arduino.h>
#include <PS2Keyboard.h>
#include <PololuLedStrip.h>
#include <TasmotaClient.h>

#define BARCODE_LENGTH 13
#define BARCODE_ITEMS 7
#define LED_COUNT 7
#define LED_INTENSITY 64

const int DataPin = 5;
const int IRQpin =  3;
const int ExitPin = 13;
const int ScannerPin = 7;

TasmotaClient client(&Serial);

PololuLedStrip<A0> ledStrip;
rgb_color colors[LED_COUNT];
const rgb_color red = rgb_color(LED_INTENSITY,0,0);
const rgb_color green = rgb_color(0,LED_INTENSITY,0);
const rgb_color black = rgb_color(0,0,0);
PS2Keyboard keyboard;
char items[BARCODE_ITEMS][BARCODE_LENGTH] = {"5410673052001", "5201005079779", "8710447246795", "5201066111128", "5053990155354", "5201004020338", "5202178083747"};

void flashLEDs(rgb_color rgbc);
void user_FUNC_RECEIVE(char *data);

void setup() {
  pinMode(ExitPin, OUTPUT);
  digitalWrite(ExitPin, LOW);
  pinMode(ScannerPin, OUTPUT);
  digitalWrite(ScannerPin, LOW);
  keyboard.begin(DataPin, IRQpin, PS2Keymap_US);
  Serial.begin(115200);
  client.attach_FUNC_COMMAND_SEND(user_FUNC_RECEIVE);

  PololuLedStripBase::interruptFriendly = true;
  flashLEDs(black);
}

void loop() {
  static char barcode[BARCODE_LENGTH+1] = {};
  static uint8_t index = 0;
  if (keyboard.available()) {
    char c = keyboard.read();
    if (c == '\r') { // Received barcode
      barcode[index] = '\0';
      index = 0;
      client.SendTele(barcode);
    } else if (index < BARCODE_LENGTH) {
      barcode[index] = c;
      index++;
    } else {
      index = 0;
    }
  }
  client.loop();
}

void flashLEDs(rgb_color rgbc) {
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    colors[i] = rgbc;
  }
  ledStrip.write(colors, LED_COUNT);
}

void user_FUNC_RECEIVE(char *data)
{
  String cmnd = data;
  if (cmnd.equals("ON")) {
    digitalWrite(ScannerPin, HIGH);
  }if (cmnd.equals("OFF")) {
    digitalWrite(ScannerPin, LOW);
  }
  if (cmnd.equals("OPEN")) {
    digitalWrite(ExitPin, HIGH);
    flashLEDs(green);
    delay(500);
    flashLEDs(black);
    delay(500);
    flashLEDs(green);
    delay(500);
    flashLEDs(black);
    delay(500);
  }
  if (cmnd.equals("CLOSE")) {
    digitalWrite(ExitPin, LOW);
  }
  if (cmnd.equals("FLASH_RED")) {
    flashLEDs(red);
    delay(500);
    flashLEDs(black);
    delay(500);
    flashLEDs(red);
    delay(500);
    flashLEDs(black);
    delay(500);
  }
  if (cmnd.startsWith("GREEN")) {
    if (cmnd[5] < '0' || cmnd[5] > '7') return;
    uint8_t i = cmnd[5] - 48;
    colors[i] = green;
    ledStrip.write(colors, LED_COUNT);
  }
}
