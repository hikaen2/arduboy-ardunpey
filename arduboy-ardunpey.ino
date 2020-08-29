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


/*
 * global vars
 */

Arduboy2 arduboy;

/*
 * [0][0] [1][0] [2][0] [3][0] [4][0]
 * [0][1] [1][1] [2][1] [3][1] [4][1]
 * [0][2] [1][2] [2][2] [3][2] [4][2]
 * [0][3] [1][3] [2][3] [3][3] [4][3]
 * [0][4] [1][4] [2][4] [3][4] [4][4]
 * [0][5] [1][5] [2][5] [3][5] [4][5]
 * [0][6] [1][6] [2][6] [3][6] [4][6]
 * [0][7] [1][7] [2][7] [3][7] [4][7]
 * [0][8] [1][8] [2][8] [3][8] [4][8]
 * [0][9] [1][9] [2][9] [3][9] [4][9]
 */
panel_t BOARD[FILE_MAX][RANK_MAX];
bool REACHED[FILE_MAX][RANK_MAX];
bool SEEN[FILE_MAX][RANK_MAX];
uint8_t cur_file = 2;
uint8_t cur_rank = 4;
uint8_t key_wait = 0;
uint8_t scroll_wait = 64;
uint8_t erase_wait = 0;
uint16_t score = 0;


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

    erase_wait = max(erase_wait - 1, 0);
    if (erase_wait == 0) {
        for (int a = 0; a < FILE_MAX * RANK_MAX; a++) {
            if (*((panel_t*)REACHED + a)) {
                *((panel_t*)BOARD + a) = EMPTY;
                score++;
            }
        }
        memset(REACHED, 0, sizeof(REACHED));
        memset(SEEN, 0, sizeof(SEEN));
        bool reached = false;
        for (int r = 0; r < RANK_MAX; r++) reached |= search_r(0, r);
        if (reached) erase_wait = 31;
    }

    if (erase_wait == 0 && (--scroll_wait == 0 || arduboy.justPressed(B_BUTTON))) {
        panel_t b[] = {EMPTY, EMPTY, EMPTY, EMPTY, SLASH, BACK_SLASH, AND, OR};
        for (int f = 0; f < FILE_MAX; f++) BOARD[f][0] = EMPTY;
        for (int a = 0; a < FILE_MAX * RANK_MAX - 1; a++) *((panel_t*)BOARD + a) = *((panel_t*)BOARD + a + 1);
        for (int f = 0; f < FILE_MAX; f++) BOARD[f][9] = b[random(8)];
        cur_rank = max(cur_rank - 1, 0);
        scroll_wait = 255;
    }

    if (arduboy.justPressed(A_BUTTON) && !REACHED[cur_file][cur_rank] && !REACHED[cur_file][cur_rank + 1]) {
        panel_t p = BOARD[cur_file][cur_rank];
        BOARD[cur_file][cur_rank] = BOARD[cur_file][cur_rank + 1];
        BOARD[cur_file][cur_rank + 1] = p;
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
            if (REACHED[f][r] && erase_wait / 8 % 2 == 1) {
                draw_panel(f, r, &BOX_BITMAP[0]);
            } else {
                draw_panel(f, r, &BOX_BITMAP[24 * BOARD[f][r]]);
            }
        }
    }
    draw_cursor(cur_file, cur_rank);
    arduboy.drawBitmap(120, 48, &NUM_BITMAP[5 * (score / 1000 % 10)], 5, 3);
    arduboy.drawBitmap(120, 52, &NUM_BITMAP[5 * (score / 100 % 10)], 5, 3);
    arduboy.drawBitmap(120, 56, &NUM_BITMAP[5 * (score / 10 % 10)], 5, 3);
    arduboy.drawBitmap(120, 60, &NUM_BITMAP[5 * (score % 10)], 5, 3);
    arduboy.drawFastVLine(127, 0, 63 - scroll_wait / 4);
    arduboy.display();
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

bool is_index_valid(uint8_t f, uint8_t r) {
    return f < FILE_MAX && r < RANK_MAX;
}

/**
 * パネルの右端のノード（右上か右下）に接続されているエッジを探索する
 */
bool search_r(uint8_t f, uint8_t r) {
    if (SEEN[f][r] || REACHED[f][r]) return REACHED[f][r];
    SEEN[f][r] = true;
    REACHED[f][r] |= f == FILE_MAX - 1;
    REACHED[f][r] |= has_ru_node(BOARD[f][r]) && is_index_valid(f + 1, r - 1) && has_ld_node(BOARD[f + 1][r - 1]) && search_r(f + 1, r - 1); // パネルの右上のノードから右上に向かう
    REACHED[f][r] |= has_ru_node(BOARD[f][r]) && is_index_valid(f + 1, r - 0) && has_lu_node(BOARD[f + 1][r - 0]) && search_r(f + 1, r - 0); // パネルの右上のノードから右下に向かう
    REACHED[f][r] |= has_ru_node(BOARD[f][r]) && is_index_valid(f + 0, r - 1) && has_rd_node(BOARD[f + 0][r - 1]) && search_l(f + 0, r - 1); // パネルの右上のノードから左上に向かう
    REACHED[f][r] |= has_rd_node(BOARD[f][r]) && is_index_valid(f + 1, r + 1) && has_lu_node(BOARD[f + 1][r + 1]) && search_r(f + 1, r + 1); // パネルの右下のノードから右下に向かう
    REACHED[f][r] |= has_rd_node(BOARD[f][r]) && is_index_valid(f + 1, r + 0) && has_ld_node(BOARD[f + 1][r + 0]) && search_r(f + 1, r + 0); // パネルの右下のノードから右上に向かう
    REACHED[f][r] |= has_rd_node(BOARD[f][r]) && is_index_valid(f + 0, r + 1) && has_ru_node(BOARD[f + 0][r + 1]) && search_l(f + 0, r + 1); // パネルの右下のノードから左下に向かう
    SEEN[f][r] = false;
    return REACHED[f][r];
}

/**
 * パネルの左端のノード（左上か左下）に接続されているエッジを探索する
 */
bool search_l(uint8_t f, uint8_t r) {
    if (SEEN[f][r] || REACHED[f][r]) return REACHED[f][r];
    SEEN[f][r] = true;
    // REACHED[f][r] |= f == FILE_MAX - 1; // 左端探索のときは到達しない
    REACHED[f][r] |= has_lu_node(BOARD[f][r]) && is_index_valid(f - 1, r - 1) && has_rd_node(BOARD[f - 1][r - 1]) && search_l(f - 1, r - 1); // パネルの左上のノードから左上に向かう
    REACHED[f][r] |= has_lu_node(BOARD[f][r]) && is_index_valid(f - 1, r - 0) && has_ru_node(BOARD[f - 1][r - 0]) && search_l(f - 1, r - 0); // パネルの左上のノードから左下に向かう
    REACHED[f][r] |= has_lu_node(BOARD[f][r]) && is_index_valid(f - 0, r - 1) && has_ld_node(BOARD[f - 0][r - 1]) && search_r(f - 0, r - 1); // パネルの左上のノードから右上に向かう
    REACHED[f][r] |= has_ld_node(BOARD[f][r]) && is_index_valid(f - 1, r + 1) && has_ru_node(BOARD[f - 1][r + 1]) && search_l(f - 1, r + 1); // パネルの左下のノードから左下に向かう
    REACHED[f][r] |= has_ld_node(BOARD[f][r]) && is_index_valid(f - 1, r + 0) && has_rd_node(BOARD[f - 1][r + 0]) && search_l(f - 1, r + 0); // パネルの左下のノードから左上に向かう
    REACHED[f][r] |= has_ld_node(BOARD[f][r]) && is_index_valid(f - 0, r + 1) && has_lu_node(BOARD[f - 0][r + 1]) && search_r(f - 0, r + 1); // パネルの左下のノードから右下に向かう
    SEEN[f][r] = false;
    return REACHED[f][r];
}

void draw_panel(int file, int rank, const uint8_t* bitmap) {
    arduboy.drawBitmap(11 * (RANK_MAX - 1 - rank) + 3, 12 * file + 2, bitmap, 12, 13);
}

void draw_cursor(int file, int rank) {
    arduboy.drawBitmap(11 * (RANK_MAX - 2 - rank) + 3, 12 * file + 2, CURSOR_BITMAP, 23, 13, INVERT);
}
