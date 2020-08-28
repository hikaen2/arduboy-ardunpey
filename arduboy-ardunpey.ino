#include <Arduboy2.h>


/*
 * enum
 */
enum panel_t : uint8_t {
  EMPTY      = 0, // ' '
  SLASH      = 1, // '/'
  BACK_SLASH = 2, // '\'
  AND        = 3, // '∧'
  OR         = 4, // '∨'
};


/*
 * consts
 */

const uint8_t PROGMEM BOX_BITMAP[] = {
  0x07, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x07, 0x1c, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x1c, // EMPTY
  0x07, 0x07, 0x0d, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x00, 0x01, 0x01, 0x07, 0x1c, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x16, 0x1c, 0x1c, // SLASH
  0x07, 0x01, 0x01, 0x00, 0x80, 0xc0, 0x60, 0x30, 0x18, 0x0d, 0x07, 0x07, 0x1c, 0x1c, 0x16, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x1c, // BACK_SLASH
  0x07, 0x07, 0x0d, 0x18, 0xb0, 0xe0, 0x40, 0x00, 0x00, 0x01, 0x01, 0x07, 0x1c, 0x1c, 0x16, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x1c, // AND
  0x07, 0x01, 0x01, 0x00, 0x00, 0x40, 0xe0, 0xb0, 0x18, 0x0d, 0x07, 0x07, 0x1c, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x16, 0x1c, 0x1c, // OR
};

const uint8_t PROGMEM CURSOR_BITMAP[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
};

const uint8_t PROGMEM NUM_BITMAP[] = {
  0x03, 0x05, 0x05, 0x05, 0x06, // 0
  0x07, 0x02, 0x02, 0x03, 0x02, // 1
  0x07, 0x01, 0x06, 0x04, 0x03, // 2
  0x03, 0x04, 0x02, 0x04, 0x03, // 3
  0x04, 0x04, 0x07, 0x05, 0x01, // 4
  0x03, 0x04, 0x07, 0x01, 0x07, // 5
  0x03, 0x05, 0x07, 0x01, 0x06, // 6
  0x01, 0x01, 0x02, 0x04, 0x07, // 7
  0x03, 0x05, 0x07, 0x05, 0x06, // 8
  0x03, 0x04, 0x07, 0x05, 0x06, // 9
};

const int FILE_MAX = 5;
const int RANK_MAX = 10;
const int U = -1; // up
const int D = 1; // down
const int R = RANK_MAX; // right
const int L = -RANK_MAX; // left


/*
 * global vars 
 */

Arduboy2 arduboy;

/*
 *  0 10 20 30 40
 *  1 11 21 31 41
 *  2 12 22 32 42
 *  3 13 23 33 43
 *  4 14 24 34 44
 *  5 15 25 35 45
 *  6 16 26 36 46
 *  7 17 27 37 47
 *  8 18 28 38 48
 *  9 19 29 39 49
 */
panel_t BOARD[FILE_MAX * RANK_MAX];
bool REACHED[FILE_MAX * RANK_MAX];
bool SEEN[FILE_MAX * RANK_MAX];
int cur_file = 2;
int cur_rank = 4;
int frame_counter = 128;
int key_wait = 0;
int score = 0;


/*
 * functions
 */

void setup() {
  arduboy.begin();
  arduboy.initRandomSeed();
  arduboy.setFrameRate(30);
  // arduboy.invert(true);
}

void loop() {
  if (!arduboy.nextFrame()) return;
  arduboy.pollButtons();

  frame_counter++;

  memset(REACHED, 0, sizeof(REACHED));
  memset(SEEN, 0, sizeof(SEEN));
  for (int r = 0; r < RANK_MAX; r++) {
    search_r(0, r);
  }
  for (int a = 0; a < 50; a++) {
    if (REACHED[a]) {
      BOARD[a] = EMPTY;
      score++;
    }
  }

  if (frame_counter % 256 == 0 || arduboy.justPressed(B_BUTTON)) {
    panel_t b[] = {EMPTY, EMPTY, EMPTY, EMPTY, SLASH, BACK_SLASH, AND, OR};
    for (int f = 0; f < FILE_MAX; f++) BOARD[address(f, 0)] = EMPTY;
    for (int a = 0; a < 49; a++) BOARD[a] = BOARD[a + 1];
    for (int f = 0; f < FILE_MAX; f++) BOARD[address(f, 9)] = b[random(8)];
    cur_rank = max(cur_rank - 1, 0);
  }

  if (arduboy.justPressed(A_BUTTON)) {
    panel_t b = BOARD[address(cur_file, cur_rank)];
    BOARD[address(cur_file, cur_rank)] = BOARD[address(cur_file, cur_rank + 1)];
    BOARD[address(cur_file, cur_rank + 1)] = b;
  }

  key_wait = max(key_wait - 1, 0);
  if (key_wait == 0) {
    if (arduboy.pressed(RIGHT_BUTTON)) {
      cur_rank = max(cur_rank - 1, 0);
      key_wait = 4;
    }
    if (arduboy.pressed(LEFT_BUTTON)) {
      cur_rank = min(cur_rank + 1, RANK_MAX - 2);
      key_wait = 4;
    }
    if (arduboy.pressed(UP_BUTTON)) {
      cur_file = max(cur_file - 1, 0);
      key_wait = 4;
    }
    if (arduboy.pressed(DOWN_BUTTON)) {
      cur_file = min(cur_file + 1, FILE_MAX - 1);
      key_wait = 4;
    }
  }

  arduboy.clear();
  for (int f = 0; f < FILE_MAX; f++) {
    for (int r = 0; r < RANK_MAX; r++) {
      draw_panel(f, r, &BOX_BITMAP[24 * BOARD[address(f, r)]]);
    }
  }
  draw_cursor(cur_file, cur_rank);

  arduboy.drawBitmap(120, 48, &NUM_BITMAP[5 * (score / 1000 % 10)], 5, 3);
  arduboy.drawBitmap(120, 52, &NUM_BITMAP[5 * (score / 100 % 10)], 5, 3);
  arduboy.drawBitmap(120, 56, &NUM_BITMAP[5 * (score / 10 % 10)], 5, 3);
  arduboy.drawBitmap(120, 60, &NUM_BITMAP[5 * (score % 10)], 5, 3);

  arduboy.display();
}

int address(int file, int rank) {
  return RANK_MAX * file + rank;
}

int file(int address) {
  return address / RANK_MAX;
}

int rank(int address) {
  return address % RANK_MAX;
}

/**
 * パネルの右上にノードがあるか
 */
bool has_ru_node(panel_t p) {
  return p == SLASH || p == OR;
}

/**
 * パネルの右下にノードがあるか
 */
bool has_rd_node(panel_t p) {
  return p == BACK_SLASH || p == AND;
}

/**
 * パネルの左上にノードがあるか
 */
bool has_lu_node(panel_t p) {
  return p == BACK_SLASH || p == OR;
}

/**
 * パネルの左下にノードがあるか
 */
bool has_ld_node(panel_t p) {
  return p == SLASH || p == AND;  
}

/**
 * パネルの右端のノード（右上か右下）に接続されているエッジを探索する
 */
bool search_r(uint8_t f, uint8_t r) {
  if (FILE_MAX <= f || RANK_MAX <= r) return false;
  uint8_t a = address(f, r);
  if (SEEN[a] || REACHED[a]) return REACHED[a];
  SEEN[a] = true;

  REACHED[a] |= f == FILE_MAX - 1;
  REACHED[a] |= has_ru_node(BOARD[a]) && has_ld_node(BOARD[a + R + U]) && search_r(f + 1, r - 1); // パネルの右上のノードから右上に向かう
  REACHED[a] |= has_ru_node(BOARD[a]) && has_lu_node(BOARD[a + R    ]) && search_r(f + 1, r - 0); // パネルの右上のノードから右下に向かう
  REACHED[a] |= has_ru_node(BOARD[a]) && has_rd_node(BOARD[a + U    ]) && search_l(f + 0, r - 1); // パネルの右上のノードから左上に向かう
  REACHED[a] |= has_rd_node(BOARD[a]) && has_lu_node(BOARD[a + R + D]) && search_r(f + 1, r + 1); // パネルの右下のノードから右下に向かう
  REACHED[a] |= has_rd_node(BOARD[a]) && has_ld_node(BOARD[a + R    ]) && search_r(f + 1, r + 0); // パネルの右下のノードから右上に向かう
  REACHED[a] |= has_rd_node(BOARD[a]) && has_ru_node(BOARD[a + D    ]) && search_l(f + 0, r + 1); // パネルの右下のノードから左下に向かう

  SEEN[a] = false;
  return REACHED[a];
}

/**
 * パネルの左端のノード（左上か左下）に接続されているエッジを探索する
 */
bool search_l(uint8_t f, uint8_t r) {
  if (FILE_MAX <= f || RANK_MAX <= r) return false;
  uint8_t a = address(f, r);
  if (SEEN[a] || REACHED[a]) return REACHED[a];
  SEEN[a] = true;

  // REACHED[a] |= f == FILE_MAX - 1; // 左端探索のときは到達しない
  REACHED[a] |= has_lu_node(BOARD[a]) && has_rd_node(BOARD[a + L + U]) && search_l(f - 1, r - 1); // パネルの左上のノードから左上に向かう
  REACHED[a] |= has_lu_node(BOARD[a]) && has_ru_node(BOARD[a + L    ]) && search_l(f - 1, r - 0); // パネルの左上のノードから左下に向かう
  REACHED[a] |= has_lu_node(BOARD[a]) && has_ld_node(BOARD[a + U    ]) && search_r(f - 0, r - 1); // パネルの左上のノードから右上に向かう
  REACHED[a] |= has_ld_node(BOARD[a]) && has_ru_node(BOARD[a + L + D]) && search_l(f - 1, r + 1); // パネルの左下のノードから左下に向かう
  REACHED[a] |= has_ld_node(BOARD[a]) && has_rd_node(BOARD[a + L    ]) && search_l(f - 1, r + 0); // パネルの左下のノードから左上に向かう
  REACHED[a] |= has_ld_node(BOARD[a]) && has_lu_node(BOARD[a + D    ]) && search_r(f - 0, r + 1); // パネルの左下のノードから右下に向かう

  SEEN[a] = false;
  return REACHED[a];
}

void draw_panel(int file, int rank, const uint8_t* bitmap) {
  arduboy.drawBitmap(11 * (RANK_MAX - 1 - rank) + 3, 12 * file + 2, bitmap, 12, 13);
}

void draw_cursor(int file, int rank) {
  arduboy.drawBitmap(11 * (RANK_MAX - 2 - rank) + 3, 12 * file + 2, CURSOR_BITMAP, 23, 13, INVERT);
}
