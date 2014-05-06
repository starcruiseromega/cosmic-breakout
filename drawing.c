#include "common.h"
#include "drawing.h"
#include "fonts.h"
#include "sprites.h"

void lcdSpi(uint8_t x){

	while(!(SSPSTAT & 0x01));
	SSPBUF = x;
}

void lcdCd(char isData){
	if (isData){
		LATC |= (1 << 7);
	} else {
		LATC &=~ (1 << 7);
	}
}

void lcdCs(char selected){
	if (selected){
		LATC &=~ (1 << 2);
	} else {
		LATC |= (1 << 2);
	}
}

void lcdCmdOneByte(unsigned char cmd){
	lcdCd(0);	// is cmd
	lcdCs(1);	// selected
	lcdSpi(cmd);
	lcdCs(0);	// deselected
}

void lcdCmdTwoByte(unsigned char cmd, unsigned char second){
	lcdCd(0);	// is cmd
	lcdCs(1);	// selected
	lcdSpi(cmd);
	lcdSpi(second);
	lcdCs(0);	// deselected
}

void lcdDataByte(unsigned char val){
	lcdCd(1);	// is not cmd
	lcdCs(1);	// selected
	lcdSpi(val);
	lcdCs(0);	// deselected
}

void lcdData(const unsigned char *data, unsigned short len){
	lcdCd(1);	// is not cmd
	lcdCs(1);	// selected
	while(len--){
		lcdSpi(*data++);
	}
	lcdCs(0);	// deselected
}

void lcdOn(void){
	SSPSTAT = 0b11000000;
	SSPCON2 = 0b00000000;
	SSPCON3 = 0b00000000;
	SSPCON1 = 0b00100000;
	SSPBUF = 0; //start one

	LATC |= (1 << 6);
	__delay_ms(100);
	lcdCmdOneByte(0x40);
	lcdCmdOneByte(0xA0); //diff from docs: reverse seg direction (mirror screen)
 	lcdCmdOneByte(0xC8); //diff from docs: reverse com direction (mirror screen)
	lcdCmdOneByte(0xA4);
	lcdCmdOneByte(0xA6); // not inverse
	lcdCmdOneByte(0xA2);
	lcdCmdOneByte(0x2F);
	lcdCmdOneByte(0x27);
	lcdCmdTwoByte(0x81, 0x06); //contrast
	lcdCmdTwoByte(0xFA, 0x90);
	lcdCmdOneByte(0xAF);
}

void lcdClear(void){
	uint8_t pg;
	uint8_t ct;

	for(pg = 0; pg < 8; pg++){
		lcdCmdOneByte(0xB0 | pg);	// set page addr
		lcdCmdTwoByte(0x00, 0x10);	// set columd addr to 0
		lcdCd(1);	// is not cmd
		lcdCs(1);	// selected
		for(ct = 0; ct < 132; ct++) lcdSpi(0);
		lcdCs(0);	// deselected
	}
}

void lcdClearColumns(uint8_t row, uint8_t startCol, uint8_t numCols){
	// This is basically doing lcdManyPixels with all the columns as nothing
	uint8_t page = row >> 3;

	startCol += (132 - 102);	//for inverted only)

	lcdCmdOneByte(0xB0 | page);	// set page
	lcdCmdTwoByte(0x00 | (startCol & 0x0F), 0x10 | (startCol >> 4));
	lcdCd(1);	// is not cmd
	lcdCs(1);	// selected
	while(numCols--){
		lcdSpi(0x00);
	}
	lcdCs(0);	// deselected
}

void lcdManyPixels(uint8_t row, uint8_t col, const uint8_t* data, uint8_t num){
	// write 8 vertical pixels, LSB on top, at a given coordinate, where row MUST be multiple of 8
	uint8_t page = row >> 3;

	col += (132 - 102);	//for inverted only)

	lcdCmdOneByte(0xB0 | page);	// set page
	lcdCmdTwoByte(0x00 | (col & 0x0F), 0x10 | (col >> 4));
	lcdData(data, num);
}

void lcdScreenInvert(){
	lcdCmdOneByte(0xA7);	
}

void lcdScreenReset(){
	lcdCmdOneByte(0xA6);	
}

void lcdPixels(uint8_t row, uint8_t col, uint8_t vals){
	lcdManyPixels(row, col, &vals, 1);
}

void writeCosmic(uint8_t row, uint8_t col){
	drawBigC(row, col);
	drawBigO(row, col + 10);
	drawBigS(row, col + 20);
	drawBigM(row, col + 30);
	drawBigI(row, col + 45);
	drawBigC(row, col + 50);
}

void writeBreakout(uint8_t row, uint8_t col){
	drawBigB(row, col);
	drawBigR(row, col + 10);
	drawBigE(row, col + 20);
	drawBigA(row, col + 30);
	drawBigK(row, col + 40);
	drawBigO(row, col + 50);
	drawBigU(row, col + 60);
	drawBigT(row, col + 70);
}

void drawBigA(uint8_t row, uint8_t col){
	// 9 pixels wide
	static const uint8_t bigA_top[] = {0xc0, 0xf0, 0xbc, 0x8e, 0x82, 0x8e, 0xbc, 0xf0, 0xc0};
	static const uint8_t bigA_bottom[] = {0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff};
	lcdManyPixels(row, col, bigA_top, sizeof(bigA_top));
	lcdManyPixels(row + 8, col, bigA_bottom, sizeof(bigA_bottom));
}
void drawBigB(uint8_t row, uint8_t col){
	// 9 pixels wide
	static const uint8_t bigB_top[] = {0x02, 0x02, 0xfe, 0xfe, 0x02, 0x02, 0x86, 0xfc, 0x78};
	static const uint8_t bigB_bottom[] = {0x80, 0x80, 0xff, 0xff, 0x81, 0x81, 0xc3, 0x7e, 0x3c};
	lcdManyPixels(row, col, bigB_top, sizeof(bigB_top));
	lcdManyPixels(row + 8, col, bigB_bottom, sizeof(bigB_bottom));
}
void drawBigC(uint8_t row, uint8_t col){
	// 9 pixels wide
	static const uint8_t bigC_top[] = {0xf0, 0xf8, 0x0c, 0x06, 0x02, 0x02, 0x06, 0x0c, 0x18};
	static const uint8_t bigC_bottom[] = {0x1f, 0x3f, 0x60, 0xc0, 0x80, 0x80, 0xc0, 0x60, 0x30};
	lcdManyPixels(row, col, bigC_top, sizeof(bigC_top));
	lcdManyPixels(row + 8, col, bigC_bottom, sizeof(bigC_bottom));
}
void drawBigE(uint8_t row, uint8_t col){
	// 9 pixels wide
	static const uint8_t bigE_top[] = {0x02, 0x02, 0xfe, 0xfe, 0x02, 0x02, 0x02, 0x82, 0x06};
	static const uint8_t bigE_bottom[] = {0x80, 0x80, 0xff, 0xff, 0x81, 0x81, 0x81, 0x83, 0xc0};
	lcdManyPixels(row, col, bigE_top, sizeof(bigE_top));
	lcdManyPixels(row + 8, col, bigE_bottom, sizeof(bigE_bottom));
}
void drawBigI(uint8_t row, uint8_t col){
	// 4 pixels wide
	static const uint8_t bigI_top[] = {0x06, 0xfe, 0xfe, 0x06};
	static const uint8_t bigI_bottom[] = {0xc0, 0xff, 0xff, 0xc0};
	lcdManyPixels(row, col, bigI_top, sizeof(bigI_top));
	lcdManyPixels(row + 8, col, bigI_bottom, sizeof(bigI_bottom));
}
void drawBigK(uint8_t row, uint8_t col){
	// 10 pixels wide
	static const uint8_t bigK_top[] = {0x02, 0xfe, 0xfe, 0x82, 0x80, 0xc0, 0xf2, 0x3e, 0x0e, 0x02};
	static const uint8_t bigK_bottom[] = {0x80, 0xff, 0xff, 0x83, 0x03, 0x06, 0x9e, 0xb8, 0xe0, 0x80};
	lcdManyPixels(row, col, bigK_top, sizeof(bigK_top));
	lcdManyPixels(row + 8, col, bigK_bottom, sizeof(bigK_bottom));
}
void drawBigM(uint8_t row, uint8_t col){
	// 14 pixels wide
	static const uint8_t bigM_top[] = {0x02, 0xfe, 0xfe, 0x0e, 0x18, 0x30, 0xe0, 0xe0, 0x30, 0x18, 0x0e, 0xfe, 0xfe, 0x02};
	static const uint8_t bigM_bottom[] = {0xc0, 0xff, 0xff, 0xc0};
	lcdManyPixels(row, col, bigM_top, sizeof(bigM_top));
	lcdManyPixels(row + 8, col, bigM_bottom, sizeof(bigM_bottom));
	lcdManyPixels(row + 8, col + 10, bigM_bottom, sizeof(bigM_bottom));
}
void drawBigO(uint8_t row, uint8_t col){
	// 10 pixels wide
	static const uint8_t bigO_top[] = {0xf0, 0xf8, 0x0c, 0x06, 0x02, 0x02, 0x06, 0x0c, 0xf8, 0xf0};
	static const uint8_t bigO_bottom[] = {0x1f, 0x3f, 0x60, 0xc0, 0x80, 0x80, 0xc0, 0x60, 0x3f, 0x1f};
	lcdManyPixels(row, col, bigO_top, sizeof(bigO_top));
	lcdManyPixels(row + 8, col, bigO_bottom, sizeof(bigO_bottom));
}
void drawBigR(uint8_t row, uint8_t col){
	// 9 pixels wide
	static const uint8_t bigR_top[] = {0x02, 0xfe, 0xfe, 0x02, 0x02, 0x86, 0xfc, 0x78};
	static const uint8_t bigR_bottom[] = {0x80, 0xff, 0xff, 0x01, 0x03, 0x87, 0xfe, 0xfc, 0x80};
	lcdManyPixels(row, col, bigR_top, sizeof(bigR_top));
	lcdManyPixels(row + 8, col, bigR_bottom, sizeof(bigR_bottom));
}
void drawBigS(uint8_t row, uint8_t col){
	// 8 pixels wide
	static const uint8_t bigS_top[] = {0x00, 0x78, 0xfc, 0x86, 0x02, 0x02, 0x06, 0x1c, 0x18};
	static const uint8_t bigS_bottom[] = {0x00, 0x30, 0x70, 0xc1, 0x81, 0x81, 0xc3, 0x7e, 0x3c};
	lcdManyPixels(row, col, bigS_top, sizeof(bigS_top));
	lcdManyPixels(row + 8, col, bigS_bottom, sizeof(bigS_bottom));
}
void drawBigT(uint8_t row, uint8_t col){
	// 10 pixels wide
	static const uint8_t bigT_top[] = {0x0e, 0x06, 0x06, 0x06, 0xfe, 0xfe, 0x06, 0x06, 0x06, 0x0e};
	static const uint8_t bigT_bottom[] = {0x80, 0x80, 0xff, 0xff, 0x80, 0x80};
	lcdManyPixels(row, col, bigT_top, sizeof(bigT_top));
	lcdManyPixels(row + 8, col + 2, bigT_bottom, sizeof(bigT_bottom));
}
void drawBigU(uint8_t row, uint8_t col){
	// 10 pixels wide
	static const uint8_t bigU_top[] = {0x02, 0xfe, 0xfe, 0x02, 0x00, 0x00, 0x02, 0xfe, 0xfe, 0x02};
	static const uint8_t bigU_bottom[] = {0x3f, 0x7f, 0xe0, 0xc0, 0xc0, 0xe0, 0x7f, 0x3f};
	lcdManyPixels(row, col, bigU_top, sizeof(bigU_top));
	lcdManyPixels(row + 8, col + 1, bigU_bottom, sizeof(bigU_bottom));
}

static const uint8_t apostrophe[] = {0x0a, 0x06};
static const uint8_t littleNumbers[10][3] = {
{0x7c, 0x44, 0x7c}, // 0
{0x00, 0x7c, 0x00}, // 1
{0x74, 0x54, 0x5c}, // 2
{0x54, 0x54, 0x7c}, // 3
{0x1c, 0x10, 0x7c}, // 4
{0x5c, 0x54, 0x74}, // 5
{0x7c, 0x54, 0x74}, // 6
{0x04, 0x04, 0x7c}, // 7
{0x7c, 0x54, 0x7c}, // 8
{0x1c, 0x14, 0x7c}, // 9
};
void printSmallString(uint8_t row, uint8_t col, const char* str, uint8_t length){
	int i;
	const uint8_t* characterMap;
	uint8_t characterWidth;
	uint8_t currentCol = col; // This is the column at the left edge of the current character
	for(i = 0; i < length; i++){
		if('A' <= str[i] && str[i] <= 'Z'){
			characterMap = upperCase[str[i] - 'A'];
			characterWidth = upperCaseWidths[str[i] - 'A'];
			lcdManyPixels(row, currentCol, characterMap, characterWidth);
			currentCol += characterWidth + 1;
		} else if('a' <= str[i] && str[i] <= 'z'){
			characterMap = lowerCase[str[i] - 'a'];
			characterWidth = lowerCaseWidths[str[i] - 'a'];
			lcdManyPixels(row, currentCol, characterMap, characterWidth);
			currentCol += characterWidth + 1;
		} else if('0' <= str[i] && str[i] <= '9'){
			characterMap = numbers[str[i] - '0'];
			characterWidth = numbersWidths[str[i] - '0'];
			lcdManyPixels(row, currentCol, characterMap, characterWidth);
			currentCol += characterWidth + 1;
		} else if(str[i] == '\''){
			lcdManyPixels(row, currentCol, apostrophe, 2);
			currentCol += 2;
		} else if(str[i] == '!'){
			lcdPixels(row, currentCol, 0x5f);
			currentCol += 2;
		} else if(str[i] == ':'){
			lcdPixels(row, currentCol, 0x28);
			currentCol += 2;
		} else if(str[i] == ' '){
			lcdPixels(row, currentCol, 0x00);
			currentCol += 2;
		} else if(str[i] == '.'){
			lcdPixels(row, currentCol, 0x40);
			currentCol += 2;
		}
	}
}

uint8_t charWidth(char character){
		if('A' <= character && character <= 'Z'){
			return upperCaseWidths[character - 'A'];
		} else if('a' <= character && character <= 'z'){
			return lowerCaseWidths[character - 'a'];
		} else if('0' <= character && character <= '9'){
			return numbersWidths[character - '0'];
		} else if(character == '\'' || character == '!' || character == ':' || character == ' ' || character == '.'){
			return 1;
		}
	return 0;
}

void printScore(uint8_t row, uint8_t col, uint32_t score){
	uint8_t i;
	uint8_t remainder = 0;
	uint32_t quotient = score;
	uint8_t length = 0;
	uint8_t parsed[10]; // uint32_t goes up to 4294967295, so we won't have one bigger than this!
	for(i = 0; i < 10; i++){
		parsed[i] = 0;
	}
	while(length < 10){
		length++;
		remainder = quotient % 10;
		quotient = quotient / 10;
		parsed[10 - length] = remainder;
		if(quotient <= 0){
			break;
		}
	}
	uint8_t currentCol = col; // This is the column at the left edge of the current character
	for(i = 10 - length; i < 10; i++){
		lcdManyPixels(row, currentCol, littleNumbers[parsed[i]], 3);
		currentCol += 4;
	}
}

static const uint8_t topBrick1[] = {0x05, 0x08, 0x01, 0x08, 0x01, 0x08, 0x01, 0x0a};
static const uint8_t bottomBrick1[] = {0x50, 0x80, 0x10, 0x80, 0x10, 0x80, 0x10, 0xa0};

static const uint8_t topBrick2[] = {0x0f, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0f};
static const uint8_t bottomBrick2[] = {0xf0, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0xf0};

static const uint8_t topBrick3[] = {0x0f, 0x0d, 0x0b, 0x0d, 0x0b, 0x0d, 0x0b, 0x0f};
static const uint8_t bottomBrick3[] = {0xf0, 0xd0, 0xb0, 0xd0, 0xb0, 0xd0, 0xb0, 0xf0};

static const uint8_t topBrick4[] = {0x0f, 0x09, 0x0f, 0x09, 0x0f, 0x09, 0x0f, 0x0f};
static const uint8_t bottomBrick4[] = {0xf0, 0x90, 0xf0, 0x90, 0xf0, 0x90, 0xf0, 0xf0};

static const uint8_t topBrick5[] = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f};
static const uint8_t bottomBrick5[] = {0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0};

static const uint8_t* topBrick[] = {topBrick1, topBrick2, topBrick3, topBrick4, topBrick5};
static const uint8_t* bottomBrick[] = {bottomBrick1, bottomBrick2, bottomBrick3, bottomBrick4, bottomBrick5};
void putBrickTile(uint8_t x, uint8_t y, uint8_t top, uint8_t bottom){
	uint8_t row = y * 8;
	uint8_t col = x * 8 + 3;
	uint8_t pixelArray[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	int i;
	if(top){
		if(top > 5){
			top = 5;
		}
		for(i = 0; i < 8; i++){
			pixelArray[i] |= topBrick[top - 1][i];
		}
	}
	if(bottom){
		if(bottom > 5){
			bottom = 5;
		}
		for(i = 0; i < 8; i++){
			pixelArray[i] |= bottomBrick[bottom - 1][i];
		}	
	}
	lcdManyPixels(row, col, pixelArray, (x == 11 ? 7 : 8)); // On the eleventh column there's a border on the edge
}

void putBrickTileWithPowerup(uint8_t x, uint8_t y, uint8_t top, uint8_t bottom, uint8_t puRow, uint8_t puX){
	if(y >= 6){
		// Don't do anything if this is the paddle row or lower
		return;
	}
	uint8_t row = y * 8;
	uint8_t col = x * 8 + 3;
	uint8_t pixelArray[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	int i;
	if(top){
		if(top > 5){
			top = 5;
		}
		for(i = 0; i < 8; i++){
			pixelArray[i] |= topBrick[top - 1][i];
		}
	}
	if(bottom){
		if(bottom > 5){
			bottom = 5;
		}
		for(i = 0; i < 8; i++){
			pixelArray[i] |= bottomBrick[bottom - 1][i];
		}	
	}
	// Add in the power up or its bottom if necessary
	if(puX == x && puRow / 8 == y){
		for(i = 0; i < 5; i++){
			pixelArray[i] |= powerUpSprites[puRow % 8][i];
		} 
	}
	if(puX == x && puRow / 8 + 1 == y && puRow % 8 >= 3){
		for(i = 0; i < 5; i++){
			pixelArray[i] |= powerUpBottoms[puRow % 8][i];
		} 
	}
	lcdManyPixels(row, col, pixelArray, (x == 11 ? 7 : 8)); // On the eleventh column there's a border on the edge
}

void putBrickTileWithBall(uint8_t x, uint8_t y, uint8_t top, uint8_t bottom, uint8_t ballRow, uint8_t ballCol, uint8_t puRow, uint8_t puX, uint8_t powerup){
	if(y >= 6){
		// Don't do anything if this is the paddle row or lower
		return;
	}
	uint8_t row = y * 8;
	uint8_t col = x * 8 + 3;
	uint8_t pixelArray[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	int i;
	if(top){
		if(top > 5){
			top = 5;
		}
		for(i = 0; i < 8; i++){
			pixelArray[i] |= topBrick[top - 1][i];
		}
	}
	if(bottom){
		if(bottom > 5){
			bottom = 5;
		}
		for(i = 0; i < 8; i++){
			pixelArray[i] |= bottomBrick[bottom - 1][i];
		}	
	}
	// Add in the power up or its bottom if necessary
	if(puX == x && puRow / 8 == y && powerup){
		for(i = 0; i < 5; i++){
			pixelArray[i] |= powerUpSprites[puRow % 8][i];
		} 
	}
	if(puX == x && puRow / 8 + 1 == y && puRow % 8 >= 3 && powerup){
		for(i = 0; i < 5; i++){
			pixelArray[i] |= powerUpBottoms[puRow % 8][i];
		} 
	}
	if(ballRow >= row && ballRow < row + 8 && (ballCol >= col && ballCol < col + 8 || ballCol + 5 >= col && ballCol + 5 < col + 8)){
		// The top of the ball coincides with this tile
		for(i = col; i < col + 8; i++){
			if(x == 11 && i == col + 7){
				// If this is the eleventh column, then this is the border and we don't need to do anything
				continue;
			}
			if(i >= ballCol && i < ballCol + 5){
				pixelArray[i - col] |= ballSprites[ballRow % 8][i - ballCol];
			}
		} 
	} else if(ballRow + 5 >= row && ballRow + 5 < row + 8 && (ballCol >= col && ballCol < col + 8 || ballCol + 5 >= col && ballCol + 5 < col + 8)){
		// The bottom of the ball coincides with this tile
		for(i = col; i < col + 8; i++){
			if(x == 11 && i == col + 7){
				// If this is the eleventh column, then this is the border and we don't need to do anything
				continue;
			}
			if(i >= ballCol && i < ballCol + 5){
				pixelArray[i - col] |= ballBottoms[ballRow % 8][i - ballCol];
			}
		}
	}
	lcdManyPixels(row, col, pixelArray, (x == 11 ? 7 : 8)); // On the eleventh column there's a border on the edge
}

static const uint8_t life[] = {0x70, 0xf8, 0xf8, 0xf8, 0x70};
void putLives(uint8_t lives){
	int i;
	uint8_t col = 92; // With some padding from border
	for(i = 0; i < lives; i++){
		lcdManyPixels(56, col, life, 5);
		col -= 6;
	}
}

void putPaddle(uint8_t col, uint8_t ballRow, uint8_t ballCol, uint8_t puRow, uint8_t puX, uint8_t size){
	int i;
	int j = 0;
	int k = 0;
	uint8_t pixelCol = 0x00;
	for(i = 3; i < 98; i++){
		if(i >= col && i < col + size){
			if(i == col || i == col + size - 1){
				pixelCol = 0xe0;
			} else {
				pixelCol = 0xa0;
			}
		} else {
			pixelCol = 0x00;
		}
		if(ballRow >= 43 && i >= ballCol && i < ballCol + 5){
			pixelCol |= ballBottoms[ballRow % 8][j];
			j++;
		}
		if(puRow >= 43 && i >= puX * 8 + 3 && i < puX * 8 + 8){
			pixelCol |= powerUpBottoms[puRow % 8][k];
			k++;
		}
		lcdPixels(48, i, pixelCol);
	}
}

void putBall(uint8_t row, uint8_t col, uint8_t bricks){
	int i;
	uint8_t lastBrickRow = (bricks / 8) * 8;
	if(row >= lastBrickRow + 8){ // If the ball is in the last row of bricks, the brick tile draw method will draw the top
		for(i = 3; i < col; i++){
			lcdPixels(row, i, 0x00);
		}
		lcdManyPixels(row, col, ballSprites[row % 8], BALL_WIDTH);
		for(i = col + 5; i < 98; i++){
			lcdPixels(row, i, 0x00);
		}
	}
	if(row % 8 > 3 && row < 40){ // The paddle draw method will draw the ball bottom too and the 0-3th ball positions don't overlap
		for(i = 3; i < col; i++){
			lcdPixels(row + 8, i, 0x00);
		}
		lcdManyPixels(row + 8, col, ballBottoms[row % 8], BALL_WIDTH);
		for(i = col + 5; i < 98; i++){
			lcdPixels(row, i, 0x00);
		}
	}
}