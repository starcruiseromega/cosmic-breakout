#ifndef __DRAWING_H__
#define __DRAWING_H__
#include <stdint.h>

/* Basic screen functions */
void lcdSpi(unsigned char x);
void lcdCd(char isData);
void lcdCs(char selected);
void lcdCmdOneByte(unsigned char cmd);
void lcdCmdTwoByte(unsigned char cmd, unsigned char second);
void lcdDataByte(unsigned char val);
void lcdData(const unsigned char *data, unsigned short len);
void lcdOn(void);
void lcdClear(void);
void lcdScreenInvert(void);
void lcdScreenReset(void);
void lcdClearColumns(uint8_t row, uint8_t startCol, uint8_t numCols);
void lcdPixels(uint8_t row, uint8_t col, uint8_t vals);
void lcdManyPixels(uint8_t row, uint8_t col, const uint8_t* data, uint8_t num);

/* This thing that we will charitably call a "font" is capital,
 * vaguely-serifed letters that are two 8-pixel pages tall and
 * around ten pixels wide
 * N.B. I only implemented the ones I needed
 */
void writeCosmic(uint8_t row, uint8_t col);
void writeBreakout(uint8_t row, uint8_t col);
void drawBigA(uint8_t row, uint8_t col);
void drawBigB(uint8_t row, uint8_t col);
void drawBigC(uint8_t row, uint8_t col);
void drawBigE(uint8_t row, uint8_t col);
void drawBigI(uint8_t row, uint8_t col);
void drawBigK(uint8_t row, uint8_t col);
void drawBigM(uint8_t row, uint8_t col);
void drawBigO(uint8_t row, uint8_t col);
void drawBigR(uint8_t row, uint8_t col);
void drawBigS(uint8_t row, uint8_t col);
void drawBigT(uint8_t row, uint8_t col);
void drawBigU(uint8_t row, uint8_t col);

/* This one is a small (one page tall) font with upper case, lower
 * case, numbers and an apostrophe.
 */
void printSmallString(uint8_t row, uint8_t col, const char* str, uint8_t length);
/* This returns the number of columns a given character takes up */
uint8_t charWidth(char character);
/* The score is a set of small, monospace numbers */
void printScore(uint8_t row, uint8_t col, uint32_t score);
/* The board is 8 tiles tall by 12 tiles wide with 3px border on the width
 * There's two bricks on a tile. If top or bottom are positive, we put a brick there */
void putBrickTile(uint8_t x, uint8_t y, uint8_t top, uint8_t bottom);
void putBrickTileWithPowerup(uint8_t x, uint8_t y, uint8_t top, uint8_t bottom, uint8_t puRow, uint8_t puX);
void putBrickTileWithBall(uint8_t x, uint8_t y, uint8_t top, uint8_t bottom, uint8_t ballRow, uint8_t ballCol, uint8_t puRow, uint8_t puX, uint8_t powerup);
/* Lives are little filled balls. They live in the lower right corner. */
void putLives(uint8_t lives);
/* The paddle lives on the penultimate page and just moves back and forth */
void putPaddle(uint8_t col, uint8_t ballRow, uint8_t ballCol, uint8_t puRow, uint8_t puX, uint8_t size);
/* This is an empty ball. bricks is the last row with bricks in it */
void putBall(uint8_t row, uint8_t col, uint8_t bricks);
#endif  /* __DRAWING_H__ */