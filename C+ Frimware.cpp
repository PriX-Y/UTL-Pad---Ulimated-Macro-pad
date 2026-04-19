
C++#include <Wire.h>
#include "Keyboard.h"

#define PCF_ADDR     0x20

#define ENC1_A       4
#define ENC1_B       5
#define ENC2_A       6
#define ENC2_B       7
#define MIC_PIN      10

const char keyMap[3][3] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'}
};

uint8_t rowPins[3] = {2, 1, 0};
uint8_t colBits[3] = {3, 4, 5};

int lastEnc1 = 0;
int lastEnc2 = 0;

void setup() {
  Wire.setSDA(9);
  Wire.setSCL(8);
  Wire.begin();

  Wire.beginTransmission(PCF_ADDR);
  Wire.write(0xFF);
  Wire.endTransmission();

  pinMode(ENC1_A, INPUT_PULLUP);
  pinMode(ENC1_B, INPUT_PULLUP);
  pinMode(ENC2_A, INPUT_PULLUP);
  pinMode(ENC2_B, INPUT_PULLUP);
  pinMode(MIC_PIN, INPUT_PULLUP);

  Keyboard.begin();
}

void loop() {
  char key = getMatrixKey();
  if (key != '\0') {
    Keyboard.press(key);
    delay(60);
    Keyboard.release(key);
    delay(100);
  }

  handleEncoder(ENC1_A, ENC1_B, &lastEnc1, 'Q', 'W');
  handleEncoder(ENC2_A, ENC2_B, &lastEnc2, 'A', 'S');

  if (digitalRead(MIC_PIN) == LOW) {
    Keyboard.press(KEY_M);
    delay(100);
    Keyboard.release(KEY_M);
    delay(150);
  }

  delay(10);
}

char getMatrixKey() {
  for (uint8_t r = 0; r < 3; r++) {
    uint8_t port = 0xFF & ~(1 << rowPins[r]);

    Wire.beginTransmission(PCF_ADDR);
    Wire.write(port);
    Wire.endTransmission();

    Wire.requestFrom(PCF_ADDR, (uint8_t)1);
    uint8_t cols = ~Wire.read();

    for (uint8_t c = 0; c < 3; c++) {
      if (cols & (1 << colBits[c])) {
        return keyMap[r][c];
      }
    }
  }
  return '\0';
}

void handleEncoder(int pinA, int pinB, int *lastState, char leftKey, char rightKey) {
  int currentState = (digitalRead(pinA) << 1) | digitalRead(pinB);

  if (currentState != *lastState) {
    if ((*lastState == 0b00 && currentState == 0b10) || 
        (*lastState == 0b10 && currentState == 0b11) || 
        (*lastState == 0b11 && currentState == 0b01) || 
        (*lastState == 0b01 && currentState == 0b00)) {
      Keyboard.press(leftKey);
      Keyboard.release(leftKey);
    } 
    else if ((*lastState == 0b00 && currentState == 0b01) || 
             (*lastState == 0b01 && currentState == 0b11) || 
             (*lastState == 0b11 && currentState == 0b10) || 
             (*lastState == 0b10 && currentState == 0b00)) {
      Keyboard.press(rightKey);
      Keyboard.release(rightKey);
    }
    *lastState = currentState;
  }
}
