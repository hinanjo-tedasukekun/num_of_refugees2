/*
 * 避難者数カウントパターンへのマッチング
 */

class CountRefugeesMatcher {
public:
  // 有限状態機械の状態を表す型
  enum State {
    // 読み取り中
    READING,
    // 受理
    ACCEPTED,
    // 却下
    REJECTED
  };

  // コンストラクタ
  CountRefugeesMatcher();
  // 状態を初期化する
  void reset();

  // 有限状態機械の状態を取得する
  State getState();
  // 文字を入力する
  // 次の状態を返す
  State put(char c);

  // 数字列を文字配列へコピーする
  void copyDigitsTo(char* s);

private:
  // 読み取り状態を表す型
  enum ReadState {
    // '@' 待ち
    WAITING_AT,
    // '0' 待ち
    WAITING_0,
    // '1' 待ち
    WAITING_1,
    // '9' 待ち
    WAITING_9,
    // 'U' 待ち 1 回目
    WAITING_U_1,
    // 'N' 待ち
    WAITING_N,
    // 'U' 待ち 2 回目
    WAITING_U_2,
    // ' ' 待ち
    WAITING_SP,
    // 数字待ち 1 回目
    WAITING_NUM_1,
    // 数字待ち 2 回目
    WAITING_NUM_2,
    // 数字待ち 3 回目
    WAITING_NUM_3,
    // 数字待ち 4 回目
    WAITING_NUM_4,
    // '\r' 待ち
    WAITING_CR,
    // '\n' 待ち
    WAITING_LF
  };

  // 有限状態機械の状態
  State state_;
  // 読み取り状態
  ReadState read_state_;
  // 数字の数
  int n_digits_;
  // 数字を溜めるバッファ
  char num_buffer_[4];

  // 文字を読む
  // c: 文字
  // expected: 予想される文字
  // next_state: 次の状態
  void read(char c, char expected, ReadState next_state);
  // 数字を読む
  // c: 文字
  // next_state: 次の状態
  // allow_cr: '\r' の入力を許可する（その場合受理状態へ遷移する）
  void readDigit(char c, ReadState next_state, bool allow_cr = false);
};
