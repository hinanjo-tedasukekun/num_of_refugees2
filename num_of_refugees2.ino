/*
   4 桁 7 セグ LED で避難者数を表示する
*/

#include <SPI.h>
#include <MsTimer2.h>
#include "CountRefugeesMatcher.h"

// トグルスイッチのピン
constexpr int SW_PIN = 3;

// ラッチ動作出力ピン
constexpr int RCK_PIN = 10;
// 合計世帯人数のSWピン
constexpr int SW_ALL_PIN = A0;
// 登録避難者数のSWピン
constexpr int SW_REGISTERED_PIN = A1;
// 在室避難者数のSWピン
constexpr int SW_PRESENT_PIN = A2;

// 合計世帯人数のLEDピン
constexpr int LED_ALL_PIN = A3;
// 登録避難者数のLEDピン
constexpr int LED_REGISTERED_PIN = A4;
// 在室避難者数のLEDピン
constexpr int LED_PRESENT_PIN = A5;

// 桁数
constexpr int N_DIGITS = 4;
// アノードのピン
constexpr int ANODE_PIN[N_DIGITS] = { 4, 5, 6, 7 };
// 各桁の点灯時間
constexpr int LIGHTING_LENGTH = 2;

// 7 セグ LED の数字表示データ
constexpr byte SEG_LED_NUM_DATA[] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4

  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111, // 9

  0b01110111, // A
  0b01111100, // b
  0b00111001, // C
  0b01011110, // d
  0b01111001, // E
  0b01110001, // F
};

// 7 セグ LED の消灯データ
constexpr byte SEG_LED_OFF = 0b00000000;

// 現在のアノード
int currentAnode = 0;

// 各桁のデータ
int segLedData[N_DIGITS] = {
  SEG_LED_NUM_DATA[0],
  SEG_LED_NUM_DATA[1],
  SEG_LED_NUM_DATA[2],
  SEG_LED_NUM_DATA[3]
};

// 数字を入れるバッファ
char numBuff[N_DIGITS];
// 避難者数パターンへのマッチャ
CountRefugeesMatcher matcher;

int swAllState = HIGH;
int swRegisteredState = HIGH;
int swPresentState = HIGH;

void setup() {
  Serial.begin(9600);
  // シリアルポート接続待ち
  while (!Serial) {
  }

  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setDataMode(SPI_MODE0);

  pinMode(SW_ALL_PIN, INPUT_PULLUP); // Inputモードでプルアップ抵抗を有効に
  pinMode(SW_REGISTERED_PIN, INPUT_PULLUP); // Inputモードでプルアップ抵抗を有効に
  pinMode(SW_PRESENT_PIN, INPUT_PULLUP); // Inputモードでプルアップ抵抗を有効に

  pinMode(LED_ALL_PIN, OUTPUT);
  pinMode(LED_REGISTERED_PIN, OUTPUT);
  pinMode(LED_PRESENT_PIN, OUTPUT);

  for (int i = 0; i < N_DIGITS; ++i) {
    pinMode(ANODE_PIN[i], OUTPUT);
    digitalWrite(ANODE_PIN[i], i == 0);
  }

  // タイマー割り込みの設定
  MsTimer2::set(LIGHTING_LENGTH, shiftDigit);
  MsTimer2::start();
}

void loop() {
  CountRefugeesMatcher::State matcherState;

  // シリアル通信の受信バッファを空にする
  while (Serial.available() > 0) {
    Serial.read();
  }

  swAllState = digitalRead(SW_ALL_PIN);
  swRegisteredState = digitalRead(SW_REGISTERED_PIN);
  swPresentState = digitalRead(SW_PRESENT_PIN);

  if (swAllState == LOW)
  {
    digitalWrite(LED_ALL_PIN, LOW);
    digitalWrite(LED_REGISTERED_PIN, HIGH);
    digitalWrite(LED_PRESENT_PIN, HIGH);
  }
  else if (swRegisteredState == LOW)
  {
    digitalWrite(LED_ALL_PIN, HIGH);
    digitalWrite(LED_REGISTERED_PIN, LOW);
    digitalWrite(LED_PRESENT_PIN, HIGH);

  }
  else if (swPresentState == LOW)
  {
    digitalWrite(LED_ALL_PIN, HIGH);
    digitalWrite(LED_REGISTERED_PIN, HIGH);
    digitalWrite(LED_PRESENT_PIN, LOW);
  }

  /*
    // 避難者数要求送信
    Serial.println(sw == LOW ? "@019DNU" : "@019DNP");

    matcher.reset();
    while (true) {
      // 文字が送られるまで待つ
      while (Serial.available() <= 0) {
      }

      matcher.put(Serial.read());

      matcherState = matcher.getState();
      if (matcherState != CountRefugeesMatcher::State::READING) {
        break;
      }
    }

    if (matcherState == CountRefugeesMatcher::State::ACCEPTED) {
      matcher.copyDigitsTo(numBuff);
      setSegLedData();
    }
  */
  delay(250);
}

// 7 セグ LED のデータを設定する
void setSegLedData() {
  int c;

  for (int i = 0; i < N_DIGITS; ++i) {
    c = numBuff[i];
    segLedData[i] = isDigit(c) ? SEG_LED_NUM_DATA[c - '0'] : SEG_LED_OFF;
  }
}

// タイマー割り込みハンドラ：桁の移動
void shiftDigit() {
  // アノードの移動
  ++currentAnode;
  if (currentAnode >= N_DIGITS) {
    currentAnode = 0;
  }

  for (int i = 0; i < N_DIGITS; ++i) {
    digitalWrite(ANODE_PIN[i], i == currentAnode);
  }

  // 桁に対応したデータを出力する
  digitalWrite(RCK_PIN, LOW);
  SPI.transfer(segLedData[currentAnode]);
  digitalWrite(RCK_PIN, HIGH);
}
