/*
 * 4 桁 7 セグ LED で避難者数を表示する
 */

#include <SPI.h>
#include <MsTimer2.h>
#include "CountRefugeesMatcher.h"

// ラッチ動作出力ピン
constexpr int RCK_PIN = 10;

// 桁数
constexpr int N_DIGITS = 4;
// カソードのピン
constexpr int KATHODE_PIN[N_DIGITS] = { 4, 5, 6, 7 };
// 各桁の点灯時間
constexpr int LIGHTING_LENGTH = 4;

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

// 現在のカソード
int currentKathode = 0;

// 各桁のデータ
int segLedData[N_DIGITS] = {
  SEG_LED_NUM_DATA[0],
  SEG_LED_NUM_DATA[0],
  SEG_LED_NUM_DATA[0],
  SEG_LED_NUM_DATA[0]
};

// 数字を入れるバッファ
char numBuff[N_DIGITS];
// 避難者数パターンへのマッチャ
CountRefugeesMatcher matcher;

void setup() {
  Serial.begin(9600);
  // シリアルポート接続待ち
  while (!Serial) {
  }

  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setDataMode(SPI_MODE0);

  for (int i = 0; i < N_DIGITS; ++i) {
    pinMode(KATHODE_PIN[i], OUTPUT);
    digitalWrite(KATHODE_PIN[i], i == 0);
  }

  // タイマー割り込みの設定
  MsTimer2::set(LIGHTING_LENGTH, shiftDigit);
  MsTimer2::start();
}

void loop() {
  CountRefugeesMatcher::State matcherState;

  // 避難者数要求送信
  Serial.println("@019DNU");

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

    delay(1000);
  } else {
    delay(100);
  }
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
  // カソードの移動
  ++currentKathode;
  if (currentKathode >= N_DIGITS) {
    currentKathode = 0;
  }

  for (int i = 0; i < N_DIGITS; ++i) {
    digitalWrite(KATHODE_PIN[i], i == currentKathode);
  }

  // 桁に対応したデータを出力する
  digitalWrite(RCK_PIN, LOW);
  SPI.transfer(segLedData[currentKathode]);
  digitalWrite(RCK_PIN, HIGH);
}
