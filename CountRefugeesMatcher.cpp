/*
 * 避難者数カウントパターンへのマッチング
 */

#include <Arduino.h>
#include "CountRefugeesMatcher.h"

// コンストラクタ
CountRefugeesMatcher::CountRefugeesMatcher() :
  state_(READING),
  read_state_(WAITING_AT),
  n_digits_(0)
{
}

// 状態を初期化する
void CountRefugeesMatcher::reset() {
  state_ = READING;
  read_state_ = WAITING_AT;
  n_digits_ = 0;
}

// 有限状態機械の状態を取得する
CountRefugeesMatcher::State CountRefugeesMatcher::getState() {
  return state_;
}

// 文字を入力する
// 次の状態を返す
CountRefugeesMatcher::State CountRefugeesMatcher::put(char c) {
  if (state_ != READING) {
    return state_;
  }

  switch (read_state_) {
  case WAITING_AT:
    read(c, '@', WAITING_0);
    break;
  case WAITING_0:
    read(c, '0', WAITING_1);
    break;
  case WAITING_1:
    read(c, '1', WAITING_9);
    break;
  case WAITING_9:
    read(c, '9', WAITING_U_1);
    break;
  case WAITING_U_1:
    read(c, 'U', WAITING_N);
    break;
  case WAITING_N:
    read(c, 'N', WAITING_U_2);
    break;
  case WAITING_U_2:
    read(c, 'U', WAITING_SP);
    break;
  case WAITING_SP:
    read(c, ' ', WAITING_NUM_1);
    break;
  case WAITING_NUM_1:
    readDigit(c, WAITING_NUM_2);
    break;
  case WAITING_NUM_2:
    readDigit(c, WAITING_NUM_3, true);
    break;
  case WAITING_NUM_3:
    readDigit(c, WAITING_NUM_4, true);
    break;
  case WAITING_NUM_4:
    readDigit(c, WAITING_CR, true);
    break;
  case WAITING_CR:
    read(c, '\r', WAITING_LF);
    break;
  case WAITING_LF:
    state_ = (c == '\n') ? ACCEPTED : REJECTED;
    break;
  default:
    state_ = REJECTED;
    break;
  }

  return state_;
}

// 文字を読む
  // c: 文字
  // expected: 予想される文字
  // next_state: 次の状態
void CountRefugeesMatcher::read(
  char c, char expected, CountRefugeesMatcher::ReadState next_state
) {
  if (c == expected) {
    read_state_ = next_state;
  } else {
    state_ = REJECTED;
  }
}

// 数字を読む
  // c: 文字
  // next_state: 次の状態
  // allow_cr: '\r' の入力を許可する（その場合受理状態へ遷移する）
void CountRefugeesMatcher::readDigit(
    char c, ReadState next_state, bool allow_cr
) {
  if (isDigit(c)) {
    num_buffer_[n_digits_] = c;
    ++n_digits_;

    read_state_ = next_state;
  } else if (allow_cr && c == '\r') {
    read_state_ = WAITING_LF;
  } else {
    state_ = REJECTED;
  }
}

// 数字列を文字配列へコピーする
void CountRefugeesMatcher::copyDigitsTo(char* s) {
  s[0] = s[1] = s[2] = s[3] = '0';

  for (int i = 0; i < n_digits_; ++i) {
    s[4 - n_digits_ + i] = num_buffer_[i];
  }
}
