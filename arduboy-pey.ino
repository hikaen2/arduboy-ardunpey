#include <Arduboy2.h>

Arduboy2 arduboy;

enum box_t : uint8_t {
  EMPTY      = 0, // ' '
  SLASH      = 1, // '/'
  BACK_SLASH = 2, // '\'
  AND        = 3, // '∧'
  OR         = 4, // '∨'
};

/*
 *  0 12 24 36 48 <- hidden rank
 *  1 13 25 37 49
 *  2 14 26 38 50
 *  3 15 27 39 51
 *  4 16 28 40 52
 *  5 17 29 41 53
 *  6 18 30 42 54
 *  7 19 31 43 55
 *  8 20 32 44 56
 *  9 21 33 45 57
 * 10 22 34 46 58
 * 11 23 35 47 59 <- hidden rank
 */
box_t BOARD[60];

const int U = -1;
const int D = 1;
const int R = 12;
const int L = -12;
const int RU = R + U;
const int RD = R + D;
const int LU = L + U;
const int LD = L + D;

const uint8_t PROGMEM BOX_BITMAP[] = {
  0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, // EMPTY
  0xff, 0x03, 0x05, 0x09, 0x11, 0x21, 0x41, 0x81, 0x01, 0x01, 0x01, 0x01, 0xff, 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x12, 0x14, 0x18, 0x1f, // SLASH
  0xff, 0x01, 0x01, 0x01, 0x01, 0x81, 0x41, 0x21, 0x11, 0x09, 0x05, 0x03, 0xff, 0x1f, 0x18, 0x14, 0x12, 0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, // BACK_SLASH
  0xff, 0x03, 0x05, 0x09, 0x11, 0xa1, 0x41, 0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 0x1f, 0x18, 0x14, 0x12, 0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, // AND
  0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x41, 0xa1, 0x11, 0x09, 0x05, 0x03, 0xff, 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x12, 0x14, 0x18, 0x1f, // OR
};

const uint8_t PROGMEM CURSOR_BITMAP[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
  0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
  0x1f, 0x1f,
};

int address(int file, int rank) {
  return 12 * file + rank;
}

int file(int address) {
  return address / 12;
}

int rank(int address) {
  return address % 12;
}

bool reachr(int address) {
  box_t b = BOARD[address];
  if (b == EMPTY) return false;
  if (file(address) == 4) return true;

  box_t r  = BOARD[address + R ];
  box_t ru = BOARD[address + RU];
  box_t rd = BOARD[address + RD];

  if ((b == SLASH      || b == OR ) && (ru == SLASH      || ru == AND) && reachr(address + RU)) return true;
  if ((b == SLASH      || b == OR ) && (r  == BACK_SLASH || r  == OR ) && reachr(address + R )) return true;
  if ((b == BACK_SLASH || b == AND) && (rd == BACK_SLASH || rd == OR ) && reachr(address + RD)) return true;
  if ((b == BACK_SLASH || b == AND) && (r  == SLASH      || r  == AND) && reachr(address + R )) return true;
  return false;
}

bool reachl(int address) {
  box_t b = BOARD[address];
  if (b == EMPTY) return false;
  if (file(address) == 0) return true;

  box_t l  = BOARD[address + L ];
  box_t lu = BOARD[address + LU];
  box_t ld = BOARD[address + LD];

  if ((b == BACK_SLASH || b == OR ) && (lu == BACK_SLASH || lu == AND) && reachl(address + LU)) return true;
  if ((b == BACK_SLASH || b == OR ) && (l  == SLASH      || l  == OR ) && reachl(address + L )) return true;
  if ((b == SLASH      || b == AND) && (ld == SLASH      || ld == OR ) && reachl(address + LD)) return true;
  if ((b == SLASH      || b == AND) && (l  == BACK_SLASH || l  == AND) && reachl(address + L )) return true;
  return false;
}

void drawBox(int file, int rank, uint8_t* bitmap) {
  arduboy.drawBitmap(12 * (10-rank) + 3, 12 * file + 1, bitmap, 13, 13);  
}

void drawCursor(int file, int rank) {
  arduboy.drawBitmap(12 * (9-rank) + 3, 12 * file + 1, CURSOR_BITMAP, 25, 13, INVERT);
}

int cur_file = 2;
int cur_rank = 5;
int delta_file = 0;
int delta_rank = 0;
int frame_counter = 128;

void setup() {
  arduboy.begin();
  arduboy.initRandomSeed();
  arduboy.setFrameRate(30);
}

void loop() {
  if (!arduboy.nextFrame()) return;
  arduboy.pollButtons();

  frame_counter++;

  uint8_t erases[60];
  uint8_t* erase = erases;
  for (int a = 0; a < 60; a++) {
    if (reachl(a) && reachr(a)) *erase++ = a;
  }
  *erase = 0;
  for (erase = erases; *erase != 0; erase++) {
    BOARD[*erase] = EMPTY;
  }

  if (frame_counter % 256 == 0) {
    box_t b[] = {EMPTY, EMPTY, EMPTY, EMPTY, SLASH, BACK_SLASH, AND, OR};
    for (int a = 0; a < 59; a++) BOARD[a] = BOARD[a + 1];
    for (int f = 0; f < 5;  f++) BOARD[address(f, 0)] = EMPTY;
    for (int f = 0; f < 5;  f++) BOARD[address(f, 10)] = b[random(8)];
    cur_rank = max(cur_rank - 1, 1);
  }

  if (arduboy.justPressed(A_BUTTON)) {
    box_t b = BOARD[address(cur_file, cur_rank)];
    BOARD[address(cur_file, cur_rank)] = BOARD[address(cur_file, cur_rank + 1)];
    BOARD[address(cur_file, cur_rank + 1)] = b;
  }

  if (arduboy.pressed(RIGHT_BUTTON)) delta_rank--;
  if (arduboy.pressed(LEFT_BUTTON))  delta_rank++;
  if (arduboy.pressed(UP_BUTTON))    delta_file--;
  if (arduboy.pressed(DOWN_BUTTON))  delta_file++;

  if (arduboy.everyXFrames(4)) {
    cur_file = cur_file + min(max(delta_file, -1), 1);
    cur_rank = cur_rank + min(max(delta_rank, -1), 1);
    cur_file = min(max(cur_file, 0), 4);
    cur_rank = min(max(cur_rank, 1), 9);
    delta_file = delta_rank = 0;
  }

  arduboy.clear();
  for (int f = 0; f < 5; f++) {
    for (int r = 1; r < 11; r++) {
      drawBox(f, r, &BOX_BITMAP[26 * BOARD[address(f, r)]]);
    }
  }
  drawCursor(cur_file, cur_rank);
  arduboy.display();
}
