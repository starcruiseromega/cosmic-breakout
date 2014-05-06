#include <stdint.h>
#include "common.h"
#include "drawing.h"
#define LEVEL_ROW(b1,b2,b3,b4,b5,b6,b7,b8,b9,bA,bB,bC)\
	(b1 << 5)|(b2 << 2)|(b3 >> 1),\
	(b3 << 7)|(b4 << 4)|(b5 << 1)|(b6 >> 2),\
	(b6 << 6)|(b7 << 3)|b8,\
	(b9 << 5)|(bA << 2)|(bB >> 1),\
	(bB << 7)|(bC << 4)

#include "level.h"

// Here's the pins that stuff is attached to:
// LED = C0
// Buttons = A0, A1, A2
// LCD:
//  n_reset - C6
//  A0 / CD - C7
//  n_CS    - C2
//  DATA    - C5
//  CLK     - C3

#define MAX_CHECKS 10 	// Size of circular debounce queue
volatile char db_left_q[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile char db_center_q[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile char db_right_q[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char db_left = 0;
char db_center = 0;
char db_right = 0;
uint8_t db_index = 0; 	// Current circular queue index

// This level of brick doesn't decrement. If you want to add a 5-hit brick, you can just bump this number up.
#define UNBREAKABLE 5

/* States for the game */
#define MAIN_MENU 0
#define LEVEL_SCREEN 1
#define GAME_SCREEN 2
#define GAME_OVER 3
#define HIGH_SCORES 4
#define ENTER_NAME 5
#define PAUSE_MENU 6
#define LOSE_LIFE 7

/* Types of power-ups */
#define EXTRA_LIFE  0
#define GROW_PADDLE 1
#define EXTRA_POINTS 2
#define NUM_POWERUPS 3

/* Paddle sizes */
#define PADDLE_STANDARD 8
#define PADDLE_LARGE 16

uint8_t State = MAIN_MENU;
/* Variables to draw the main menu screen */
#define MENU_PLAY 0
#define MENU_SCORES 1
uint8_t Menu_Selected = MENU_PLAY;

/* Variables to define the level screen */
uint8_t Level = 1;
int Level_Counter = 0;
volatile uint8_t Level_Redraw = 0;

/* Variables to define the game over screen */
int Game_Over_Counter = 0;
volatile uint8_t Game_Won = 0;

/* Variables to define the game screen */
volatile uint8_t Ball_Location_Row = 48;
volatile uint8_t Ball_Location_Col = 52;
uint8_t Old_Ball_Row = 47;
uint8_t Old_Ball_Col = 51;
int Ball_Counter = 0;
// Velocity is just added to the row/col, so the ball goes right for a
// positive X velocity and down for a positive Y velocity
int Ball_Velocity_Y = -1;
int Ball_Velocity_X = 1;
uint8_t Ball_Period = 40;
uint8_t Paddle_Col = 51; // Left edge of paddle
// Track if we're currently moving
uint8_t Paddle_Left = 0;
uint8_t Paddle_Right = 0;
int Paddle_Counter = 0;
uint8_t Paddle_Period = 20;
volatile uint8_t Paddle_Size = PADDLE_STANDARD;
uint32_t Old_Score = 10;
volatile uint32_t Game_Score = 0;
volatile uint8_t Game_Lives = 3;
uint8_t Screen_Pause = 0;
// These control the powerups. If the speed is 0, there's no powerup on the screen.
volatile uint8_t Powerup_Speed = 0;
uint8_t Powerup_Period = 60;
volatile uint8_t Powerup_Row = 0;
uint8_t OLD_Powerup_Row = 10;
volatile uint8_t Powerup_X = 0; // Just keep track of which tile it's in
volatile uint8_t Powerup_Type = 0;
volatile uint8_t Powerup_Counter = 0;

/* Variables to deal with name entry */
volatile char Entered_Name[] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
char Working_Letter = 'A';
uint8_t Entered_Name_Length = 0;
uint32_t Entered_Score = 0;
char Name_Finished = 0;
uint8_t Entered_Name_Width = 0;
uint8_t Working_Letter_Width = 5; // Magic number! :( This is the width of 'A'

/* Variables for the pause screen */
#define PAUSE_RESUME 0
#define PAUSE_QUIT 1
uint8_t Pause_Selected = PAUSE_RESUME;
int Life_Counter = 0;


uint32_t High_Score_Values[] = {350000, 250000, 200000, 142000, 114400, 57000, 14000};
char name1[] = {'D', 'i', 'm', 'a', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
char name2[] = {'R', 'y', 'a', 'n', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
char name3[] = {'J', 'o', 'h', 'n', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
char name4[] = {'A', 'd', 'r', 'i', 'a', 'n', ' ', ' ', ' ', ' ', ' '};
char name5[] = {'A', 'd', 'i', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
char name6[] = {'K', 's', 'h', 'i', 't', 'i', 'j', ' ', ' ', ' ', ' '};
char name7[] = {'A', 'a', 'r', 'o', 'n', ' ', ' ', ' ', ' ', ' ', ' '};
volatile char* High_Score_Names[] = {name1, name2, name3, name4, name5, name6, name7};
uint8_t High_Score_Lengths[] = {4, 4, 4, 6, 3, 7, 5};

char row0[12];
char row1[12];
char row2[12];
char row3[12];
char row4[12];
char row5[12];
char row6[12];
char row7[12];
// These last two sets of bricks are getting pretty close to the paddle, but counting them as "bricks" lets us handle the redraw better
char row8[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char row9[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char row10[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char row11[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile char* Bricks[] = {row0, row1, row2, row3, row4, row5, row6, row7, row8, row9, row10, row11};
uint8_t Total_Bricks = 60;

/*char row0[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
char row1[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char row2[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char row3[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char row4[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char row5[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char row6[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char row7[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// These last two sets of bricks are getting pretty close to the paddle, but counting them as "bricks" lets us handle the redraw better
char row8[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char row9[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char row10[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char row11[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile char* Bricks[] = {row0, row1, row2, row3, row4, row5, row6, row7, row8, row9, row10, row11};
uint8_t Total_Bricks = 1;*/

// Kind of a mini screen buffer
char drow0[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char drow1[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char drow2[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char drow3[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char drow4[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char drow5[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char drow6[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char drow7[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char drow8[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char drow9[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char drow10[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char drow11[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile char* Drawn_Bricks[] = {drow0, drow1, drow2, drow3, drow4, drow5, drow6, drow7, drow8, drow9, drow10, drow11};

// Signal to redraw the screen
volatile uint8_t Redraw = 1;

static void led(uint8_t bri){
    static const uint16_t gamma[] =
	{
		0x0000, 0x0001, 0x0002, 0x0004, 0x0007, 0x000B, 0x000F, 0x0015,
		0x001C, 0x0023, 0x002C, 0x0036, 0x0041, 0x004D, 0x005B, 0x0069,
		0x0079, 0x008A, 0x009D, 0x00B1, 0x00C6, 0x00DD, 0x00F5, 0x010E,
		0x0129, 0x0145, 0x0163, 0x0182, 0x01A3, 0x01C5, 0x01E9, 0x020E,
		0x0235, 0x025E, 0x0288, 0x02B4, 0x02E1, 0x0310, 0x0341, 0x0373,
		0x03A7, 0x03DD, 0x0415, 0x044E, 0x0489, 0x04C6, 0x0504, 0x0545,
		0x0587, 0x05CB, 0x0610, 0x0658, 0x06A1, 0x06EC, 0x073A, 0x0788,
		0x07D9, 0x082C, 0x0881, 0x08D7, 0x0930, 0x098A, 0x09E7, 0x0A45,
		0x0AA5, 0x0B07, 0x0B6C, 0x0BD2, 0x0C3A, 0x0CA4, 0x0D11, 0x0D7F,
		0x0DEF, 0x0E62, 0x0ED6, 0x0F4D, 0x0FC5, 0x1040, 0x10BD, 0x113C,
		0x11BD, 0x1240, 0x12C5, 0x134D, 0x13D6, 0x1462, 0x14F0, 0x1580,
		0x1612, 0x16A6, 0x173D, 0x17D6, 0x1870, 0x190E, 0x19AD, 0x1A4F,
		0x1AF3, 0x1B99, 0x1C41, 0x1CEC, 0x1D99, 0x1E48, 0x1EF9, 0x1FAD,
		0x2063, 0x211B, 0x21D6, 0x2293, 0x2352, 0x2414, 0x24D8, 0x259E,
		0x2667, 0x2732, 0x27FF, 0x28CF, 0x29A1, 0x2A76, 0x2B4D, 0x2C26,
		0x2D02, 0x2DE0, 0x2EC0, 0x2FA3, 0x3089, 0x3171, 0x325B, 0x3348,
		0x3437, 0x3528, 0x361C, 0x3713, 0x380C, 0x3908, 0x3A06, 0x3B06,
		0x3C09, 0x3D0F, 0x3E17, 0x3F21, 0x402E, 0x413E, 0x4250, 0x4365,
		0x447C, 0x4596, 0x46B2, 0x47D1, 0x48F3, 0x4A17, 0x4B3D, 0x4C66,
		0x4D92, 0x4EC1, 0x4FF2, 0x5125, 0x525C, 0x5394, 0x54D0, 0x560E,
		0x574F, 0x5892, 0x59D8, 0x5B21, 0x5C6C, 0x5DBA, 0x5F0B, 0x605E,
		0x61B4, 0x630D, 0x6469, 0x65C7, 0x6727, 0x688B, 0x69F1, 0x6B5A,
		0x6CC6, 0x6E34, 0x6FA5, 0x7119, 0x728F, 0x7409, 0x7585, 0x7703,
		0x7885, 0x7A09, 0x7B90, 0x7D1A, 0x7EA7, 0x8036, 0x81C8, 0x835D,
		0x84F5, 0x8690, 0x882D, 0x89CD, 0x8B70, 0x8D16, 0x8EBE, 0x906A,
		0x9218, 0x93C9, 0x957D, 0x9734, 0x98EE, 0x9AAA, 0x9C6A, 0x9E2C,
		0x9FF1, 0xA1B9, 0xA384, 0xA551, 0xA722, 0xA8F5, 0xAACC, 0xACA5,
		0xAE81, 0xB060, 0xB242, 0xB427, 0xB60F, 0xB7FA, 0xB9E7, 0xBBD8,
		0xBDCB, 0xBFC2, 0xC1BB, 0xC3B8, 0xC5B7, 0xC7B9, 0xC9BE, 0xCBC7,
		0xCDD2, 0xCFE0, 0xD1F1, 0xD405, 0xD61C, 0xD836, 0xDA53, 0xDC73,
		0xDE96, 0xE0BC, 0xE2E5, 0xE511, 0xE740, 0xE973, 0xEBA8, 0xEDE0,
		0xF01B, 0xF259, 0xF49B, 0xF6DF, 0xF926, 0xFB71, 0xFDBE, 0xFFFF
	};

	while(PSMC1CONbits.PSMC1LD); //wait for previous update to finish if it has not yet
    PSMC1DC = gamma[bri];
	PSMC1CONbits.PSMC1LD = 1; //latch
}

uint32_t PseudoRand = 12345678;
uint8_t pseudoRandom(uint8_t modulo){
    PseudoRand = (1103515245 * PseudoRand + 12345) % 2147483648;
	return PseudoRand % modulo; 
}

char btnRead(uint8_t which){
	// read one of the 3 buttons: btn0, btn1, or btn2. return 1 if pressed
	return !((PORTA >> which) & 1);
}

uint8_t readEeprom(uint8_t address){
	EEADRL = address;
	EEPGD = 0; // Set the selector to EEPROM (as opposed to flash)
	CFGS = 0; // Select EEPROM/flash (as opposed to some configuration registers)
	RD = 1; // Select read
	return EEDATL;
}

void writeEeprom(uint8_t address, uint8_t data){
	EEADRL = address;
	EEDATL = data;
	EEPGD = 0; // Select EEPROM
	CFGS = 0;
	WREN = 1; // Enable write
	INTCON |= 0b10000000; // This clears the GIE bit to turn off interrupts while we do the write
	// This is magic defined in the data sheet
	EECON2 = 0x55;
	EECON2 = 0xaa;
	WR = 1;
	INTCON &= 0b01111111; // Set the GIE bit again to turn the interrupts back on
	WREN = 0;
	while(WR); // Wait for the WR bit to be cleared by hardware
}

// Each high score is saved as 16 bytes in the EEPROM
// The first byte is the name length (it's set last during write and set to 0xff before write begins for integrity)
// The next 11 bytes are the characters in the name (spaces at the end like the in-memory array)
// The last four bytes represent the score with most significant bits first
void readHighScore(uint8_t index){
	uint8_t address = index * 16;
	uint8_t length = readEeprom(address);
	uint32_t score = 0;
	uint8_t i;
	if(length == 0xff){
		// Don't load any data that might be inconsistent
		// TODO: do we want to do anything in this case?
		return;
	}
	High_Score_Lengths[index] = length;
	for(i = 0; i < 11; i++){
		High_Score_Names[index][i] = readEeprom(address + 1 + i);
	}
	for(i = 0; i < 4; i++){
		score |= readEeprom(address + 12 + i);
		if(i != 3){
			// Don't need to shift the last time
			score = score << 8;
		}
	}
	High_Score_Values[index] = score;
}

void writeHighScore(uint8_t index){
	uint8_t address = index * 16;
	uint8_t scoreBits = 0x00;
	uint32_t tempScore = High_Score_Values[index];
	uint8_t i;
	// Start by writing 0xff to the length byte. If the write process is interrupted, we'll know the data is possibly inconsistent
	// Note that the length is always 11 or less anyway so 0xff is never a valid value (chose this instead of zero because that's how the memory is cleared)
	writeEeprom(address, 0xff);
	for(i = 0; i < 11; i++){
		writeEeprom(address + 1 + i, High_Score_Names[index][i]);
	}
	for(i = 0; i < 4; i++){
		scoreBits |= tempScore;
		writeEeprom(address + 15 - i, scoreBits);
		scoreBits = 0x00;
		tempScore = tempScore >> 8;
	}
	writeEeprom(address, High_Score_Lengths[index]);
}

uint8_t brickTileY(uint8_t row){
	if(row <= 3){
		return 0;
	}
	if(row >= 48){
		return 11; // XXX: yuck, this works because we don't put anything in the last row
	}
	uint8_t y = row >> 2;
	return y;
}

uint8_t brickTileX(uint8_t col){
	uint8_t x = (col - 3) >> 3;
	return x;
}

uint16_t hitBrick(uint8_t x, uint8_t y){
	uint16_t score = 0;
	if(Bricks[y][x] < UNBREAKABLE){
		Bricks[y][x]--;
		score += 100;
	}
	if(Bricks[y][x] <= 0){
		Total_Bricks--;
		score += 50; // extra bonus for finishing off a brick
		// Start a power-up randomly
		if(Powerup_Speed == 0 &&  pseudoRandom(10) == 1){
			Powerup_X = x;
			Powerup_Row = y * 4;
			Powerup_Speed = 1;
			Powerup_Type = pseudoRandom(NUM_POWERUPS);
		}
	}
	return score;
}

uint16_t hitClosestBrick(void){
	uint16_t score = 0;
	uint8_t left = Ball_Location_Col;
	uint8_t right = Ball_Location_Col + 4;
	uint8_t top = Ball_Location_Row;
	uint8_t bottom = Ball_Location_Row + 4;

	uint8_t leftTile = brickTileX(left);
	uint8_t rightTile = brickTileX(right);
	uint8_t topTile = brickTileY(top);
	uint8_t bottomTile = brickTileY(bottom);

	int ballX = Ball_Velocity_X;
	int ballY = Ball_Velocity_Y;

	if(top == 0){
		// We're hitting the top of the screen, so just reflect off
		Ball_Velocity_Y = 0 - ballY;
	} else if(bottom >= 48){
		// Do nothing in this case (the calling method takes care of it)
		// We'll still need to check the X values below though
	} else if((ballY < 0) && (Bricks[brickTileY(top + ballY)][rightTile] || Bricks[brickTileY(top + ballY)][leftTile])){
		// There's a brick right above; hit one or more bricks and reflect
		Ball_Velocity_Y = 0 - ballY;
		if(Bricks[brickTileY(top + ballY)][rightTile]){
			score += hitBrick(rightTile, brickTileY(top + ballY));
		}
		if(rightTile != leftTile && Bricks[brickTileY(top + ballY)][leftTile]){
			score += hitBrick(leftTile, brickTileY(top + ballY));
		}
	} else if((ballY > 0) && (Bricks[brickTileY(bottom + ballY)][rightTile] || Bricks[brickTileY(bottom + ballY)][leftTile])){
		// There's a brick right below; hit one or more bricks and reflect
		Ball_Velocity_Y = 0 - ballY;
		if(Bricks[brickTileY(bottom + ballY)][rightTile]){
			score += hitBrick(rightTile, brickTileY(bottom + ballY));
		}
		if(rightTile != leftTile && Bricks[brickTileY(bottom + ballY)][leftTile]){
			score += hitBrick(leftTile, brickTileY(bottom + ballY));
		}
	}

	if(left + ballX <= 2 || right + ballX >= 98){
		// We're at an edge of the screen so just reflect back
		Ball_Velocity_X = 0 - ballX;
	} else if((ballX < 0) && (Bricks[topTile][brickTileX(left + ballX)] || Bricks[bottomTile][brickTileX(left + ballX)])){
		// There's a brick directly left
		Ball_Velocity_X = 0 - ballX;
		if(Bricks[topTile][brickTileX(left + ballX)]){
			score += hitBrick(brickTileX(left + ballX), topTile);
		}
		if(topTile != bottomTile && Bricks[bottomTile][brickTileX(left + ballX)]){
			score += hitBrick(brickTileX(left + ballX), bottomTile);
		}
	} else if((ballX > 0) && (Bricks[topTile][brickTileX(right + ballX)] || Bricks[bottomTile][brickTileX(right + ballX)])){
		// There's a brick directly right
		Ball_Velocity_X = 0 - ballX;
		if(Bricks[topTile][brickTileX(right + ballX)]){
			score += hitBrick(brickTileX(right + ballX), topTile);
		}
		if(topTile != bottomTile && Bricks[bottomTile][brickTileX(right + ballX)]){
			score += hitBrick(brickTileX(right + ballX), bottomTile);
		}
	}
	return score;
}

void resetBricks(void){
	uint8_t i;
	uint8_t j;
	Total_Bricks = 0;

	//erase the entire field
	for(i = 0; i < 12; i++)
		for(j = 0; j < 12; j++)
			Bricks[i][j] = 0;

	//decompress
	for(i = 0; i < 8; i++)	{//load the row
		const uint8_t *rb = LEVEL_PATTERNS[Level - 1][i];

		Bricks[i][0] = rb[0] >> 5;
		Bricks[i][1] = rb[0] >> 2;
		Bricks[i][2] = (rb[0] << 1) | (rb[1] >> 7);
		Bricks[i][3] = rb[1] >> 4;
		Bricks[i][4] = rb[1] >> 1;
		Bricks[i][5] = (rb[1] << 2) | (rb[2] >> 6);
		Bricks[i][6] = rb[2] >> 3;
		Bricks[i][7] = rb[2];
		Bricks[i][8] = rb[3] >> 5;
		Bricks[i][9] = rb[3] >> 2;
		Bricks[i][10] = (rb[3] << 1) | (rb[4] >> 7);
		Bricks[i][11] = rb[4] >> 4;
	}

	//mask values to be valid and count breakable bricks
	for(i = 0; i < 12; i++) {
		for(j = 0; j < 12; j++) {
			Bricks[i][j] &= 7;
			if (Bricks[i][j] && Bricks[i][j] != UNBREAKABLE)
				Total_Bricks++;
		}
	}
}

void resetBall(void){
	Ball_Velocity_X = 1;
	Ball_Velocity_Y = -1;
	Ball_Location_Row = 48;
	// Reset the ball right over the paddle. Note that in the resetLevelState
	// method we reset the paddle first.
	Ball_Location_Col = Paddle_Col + 1;
}

void resetPowerup(void){
	Powerup_Speed = 0;
	Powerup_Row = 0;
	Powerup_X = 0;
	Powerup_Counter = 0;
}

void resetLevelState(void){
	Paddle_Col = 51;
	Paddle_Right = 0;
	Paddle_Left = 0;
	Paddle_Size = PADDLE_STANDARD;
	resetPowerup();
	resetBall();
	resetBricks();
}

void resetGameState(void){
	Game_Lives = 3;
	Game_Score = 0;
	Old_Score = 10;
	Level = 1;
	resetLevelState();
	resetBricks();
}

void gameOver(void){
	if(Game_Score >= High_Score_Values[6]){
		State = ENTER_NAME;		Entered_Score = Game_Score;		Name_Finished = 0;	} else {
		State = GAME_OVER;
	}
	resetGameState();
	lcdClear();
}

uint8_t ballOverPaddle(void){
	uint8_t position = 0; // This is a number 1-8 indicating where the ball is hitting the paddle (0 means it's not)
	if((Ball_Location_Col >= Paddle_Col && Ball_Location_Col <= Paddle_Col + Paddle_Size) ||
		(Ball_Location_Col + 5 >= Paddle_Col && Ball_Location_Col + 5 <= Paddle_Col + Paddle_Size)){
		if(Ball_Location_Col + 2 <= Paddle_Col){
			position = 1;
		} else if (Ball_Location_Col + 2 > Paddle_Col + Paddle_Size){
			position = Paddle_Size;
		} else {
			position = Ball_Location_Col + 2 - Paddle_Col;
		}
	}
	return position;
}

uint8_t powerupOverPaddle(void){
	uint8_t puCol = Powerup_X * 8;
	if((puCol >= Paddle_Col && puCol <= Paddle_Col + Paddle_Size) ||
		(puCol + 5 >= Paddle_Col && puCol + 5 <= Paddle_Col + Paddle_Size)){
		return 1;
	}
	return 0;
}

void interrupt isr(void){
	if(TMR1IF){
		uint8_t i;
		char left = 0;
		char center = 0;
		char right = 0;

		char leftButtonRE = 0;
		char leftButtonFE = 0;
		char centerButtonRE = 0;
		char centerButtonFE = 0;
		char rightButtonRE = 0;
		char rightButtonFE = 0;

		/* Put new values into the queues */
		db_left_q[db_index]= btnRead(0);
		db_center_q[db_index]= btnRead(1);
		db_right_q[db_index]= btnRead(2);
		db_index++;
 		if(db_index >= MAX_CHECKS){
			db_index = 0;
		}

		/* Calculate the debounced switch values */
		for(i = 0; i < MAX_CHECKS; i++){
			left = left + db_left_q[i];
			center = center + db_center_q[i];
			right = right + db_right_q[i];
		}

		// at this point, ==0 means low
		//                ==MAX_CHECKS means high
		//                else means bouncing currently

		/* Don't care about the falling edges of button presses for the right or left buttons,
		 * but the center button needs to do something on press and release sometimes
		 */
		if (!left || left == MAX_CHECKS){
			if(!db_left && left){
				leftButtonRE = 1;
			} else if(db_left && !left){
				leftButtonFE = 1;
			}
			db_left = !!left;
		}

		if(!right || right == MAX_CHECKS){
			if(!db_right && right){
				rightButtonRE = 1;
			} else if(db_right && !right){
				rightButtonFE = 1;
			}
			db_right = !!right;
		}

		if (!center || center == MAX_CHECKS){
			if(!db_center && center){
				centerButtonRE = 1;
			} else if(db_center && !center){
				centerButtonFE = 1;
			}
			db_center = !!center;
		}

		/* Update any screen parameters as necessary */
		if(State == MAIN_MENU){
			if(leftButtonRE && (Menu_Selected == MENU_SCORES)) {
				Menu_Selected = MENU_PLAY;
			} else if(rightButtonRE && (Menu_Selected == MENU_PLAY)) {
				Menu_Selected = MENU_SCORES;
			} else if(centerButtonRE) {
				lcdScreenInvert();
				Screen_Pause = 1;
			} else if(centerButtonFE) {
				lcdScreenReset();
				if(Menu_Selected == MENU_PLAY){
					State = LEVEL_SCREEN;
				} else if(Menu_Selected == MENU_SCORES){
					State = HIGH_SCORES;
				}
				lcdClear();
				Screen_Pause = 0;
			}
		} else if(State == LEVEL_SCREEN){
			/* Stay in this state for about a second before transitioning to the next */
			if(Level_Counter > 800){
				State = GAME_SCREEN;
				Level_Counter = 0;
				lcdClear();
				Redraw = 1;
			} else {
				Level_Counter++;
			}
		} else if(State == GAME_SCREEN){
			if(leftButtonRE) {
				Paddle_Left = 1;
			} else if (leftButtonFE) {
				Paddle_Left = 0;
			}
			if(rightButtonRE) {
				Paddle_Right = 1;
			} else if (rightButtonFE) {
				Paddle_Right = 0;
			}
			if(Paddle_Left && Paddle_Right) {
				// This isn't super likely but it makes sense to do nothing in this case
			} else if(Paddle_Left && (Paddle_Col >= 4) && (Paddle_Counter >= Paddle_Period)) {
				Paddle_Col -= 1;
				Paddle_Counter = 0;
			} else if(Paddle_Right && (Paddle_Col <= 97 - Paddle_Size) && (Paddle_Counter >= Paddle_Period)) {
				Paddle_Col += 1;
				Paddle_Counter = 0;
			} else if(centerButtonRE) {
				lcdScreenInvert();
				Screen_Pause = 1;
			} else if(centerButtonFE) {
				lcdScreenReset();
				Screen_Pause = 0;
				State = PAUSE_MENU;
			}
			Paddle_Counter++;
			if(Ball_Counter >= Ball_Period){
				uint16_t amountHit = hitClosestBrick();
				Game_Score += amountHit;
			 	if(Ball_Location_Row >= 48){
					uint8_t overPaddle = ballOverPaddle();
					if(overPaddle > 0 && overPaddle < 2){
						Ball_Velocity_X = -2;
						Ball_Velocity_Y = -1;
						Ball_Period = 70;
					} else if(overPaddle >= 2 && overPaddle <= Paddle_Size - 1){
						if(Ball_Velocity_X < 0){
							Ball_Velocity_X = -1;
						} else {
							Ball_Velocity_X = 1;
						}
						Ball_Velocity_Y = -1;
						Ball_Period = 40;
					} else if(overPaddle > Paddle_Size - 1){
						Ball_Velocity_X = 2;
						Ball_Velocity_Y = -1;
						Ball_Period = 70;
					} else {
						Game_Lives--;
						State = LOSE_LIFE;
						Paddle_Right = 0;
						Paddle_Left = 0;
						lcdScreenInvert();
					}
				}
				Ball_Location_Row += Ball_Velocity_Y;
				Ball_Location_Col += Ball_Velocity_X;
				Ball_Counter = 0;
			}
			Ball_Counter++;
			if(Powerup_Speed){
				if(Powerup_Counter >= Powerup_Period){
				 	if(Powerup_Row >= 48){
						if(powerupOverPaddle()){
							if(Powerup_Type == EXTRA_LIFE){
								Game_Lives += 1;
							} else if(Powerup_Type == GROW_PADDLE){
								Paddle_Size = PADDLE_LARGE;
							} else if(Powerup_Type == EXTRA_POINTS){
								Game_Score += 300;
							}
						}
						resetPowerup();
					} else {
						Powerup_Row += Powerup_Speed;
					}
					Powerup_Counter = 0;
				} else {
					Powerup_Counter++;
				}
			}
			if(Total_Bricks == 0) {
				if(Level >= LAST_LEVEL) {
					Game_Won = 1;
					gameOver();
				} else {
					Screen_Pause = 1;
					State = LEVEL_SCREEN;
					Level++;
					lcdClear();
					Level_Counter = 0;
					Level_Redraw = 1;
					resetLevelState();
					Screen_Pause = 0;
				}
			}
		} else if(State == LOSE_LIFE) {
			/* Stay in this state for half a second before transitioning to the next */
			if(Life_Counter > 400){
				lcdScreenReset();
				if(Game_Lives == 0){
					gameOver();
				} else {
					State = GAME_SCREEN;
					Life_Counter = 0;
					Paddle_Size = PADDLE_STANDARD; // If the paddle was powered up, reset it
					resetBall();
				}
			} else {
				Life_Counter++;
			}
		} else if(State == PAUSE_MENU) {
			if(rightButtonRE && (Pause_Selected == PAUSE_RESUME)) {
				Pause_Selected = PAUSE_QUIT;
			} else if(leftButtonRE && (Pause_Selected == PAUSE_QUIT)) {
				Pause_Selected = PAUSE_RESUME;
			} else if(centerButtonRE){
				lcdScreenInvert();
				Screen_Pause = 1;
			} else if(centerButtonFE && (Pause_Selected == PAUSE_QUIT)) {
				if(Game_Score >= High_Score_Values[6]){
					State = ENTER_NAME;					Entered_Score = Game_Score;					Name_Finished = 0;				} else {
					State = GAME_OVER;
				}
				resetGameState();
				lcdScreenReset();
				lcdClear();
				Screen_Pause = 0;
			} else if(centerButtonFE && (Pause_Selected == PAUSE_RESUME)) {
				Old_Score = 0; // Force the score to redraw
				Redraw = 1;
				State = GAME_SCREEN;
				lcdScreenReset();
				lcdClear();
				Screen_Pause = 0;
			}
		} else if(State == GAME_OVER){
			/* Stay in this state for about a second before transitioning to the next */
			if(Game_Over_Counter > 800){
				State = MAIN_MENU;
				Game_Over_Counter = 0;
				lcdClear();
				Screen_Pause = 0;
			} else {
				Game_Over_Counter++;
				Screen_Pause = 1;
			}
		} else if(State == ENTER_NAME){
			if(leftButtonRE) {
				if(Working_Letter == 'a'){
					Working_Letter = '.';
				} else if(Working_Letter == '.'){
					Working_Letter = 'Z';
				} else if(Working_Letter == 'A'){
					Working_Letter = '9';
				} else if(Working_Letter == '0'){
					Working_Letter = ' ';
				} else if(Working_Letter == ' '){
					Working_Letter = 'z';
				} else {
					Working_Letter--;
				}
				Working_Letter_Width = charWidth(Working_Letter);
			} else if(rightButtonRE) {
				if(Working_Letter == 'z'){
					Working_Letter = ' ';
				} else if(Working_Letter == ' '){
					Working_Letter = '0';
				} else if(Working_Letter == '9'){
					Working_Letter = 'A';
				} else if(Working_Letter == 'Z'){
					Working_Letter = '.';
				} else if(Working_Letter == '.'){
					Working_Letter = 'a';
				} else {
					Working_Letter++;
				}
				Working_Letter_Width = charWidth(Working_Letter);
			} else if(centerButtonRE) {
				if(Entered_Name_Length <= 10 && Working_Letter != '.') {
					Entered_Name[Entered_Name_Length] = Working_Letter;
					Entered_Name_Length++;
					Entered_Name_Width += (Working_Letter_Width + 1);
					if(Entered_Name_Length == 11 || Entered_Name_Width >= 59){
						Working_Letter = ' ';
						Working_Letter_Width = charWidth(' ');
					} else if(Entered_Name[Entered_Name_Length - 1] == ' '){
						Working_Letter = 'A';
						Working_Letter_Width = charWidth('A');
					} else {
						Working_Letter = 'a';
						Working_Letter_Width = charWidth('a');
					}
				}
				if(Entered_Name_Length == 11 || Working_Letter == '.' || Entered_Name_Width >= 59){
					Screen_Pause = 1;
					Name_Finished = 1;
				}
			} else if(centerButtonFE && Name_Finished) {
				int i;
				int j;
				High_Score_Values[6] = Entered_Score;
				for(i = 0; i < 11; i++){
					High_Score_Names[6][i] = Entered_Name[i];
				}
				High_Score_Lengths[6] = Entered_Name_Length;
				for(i = 5; i >= 0; i--){
					if(High_Score_Values[i] >= Entered_Score){
						break;
					} else {
						High_Score_Values[i + 1] = High_Score_Values[i];
						High_Score_Values[i] = Entered_Score;
						for(j = 0; j < 11; j++){
							High_Score_Names[i + 1][j] = High_Score_Names[i][j];
							High_Score_Names[i][j] = Entered_Name[j];

						}
						High_Score_Lengths[i + 1] = High_Score_Lengths[i];
						High_Score_Lengths[i] = Entered_Name_Length;
					}
				}
				// Persist all the scores (waiting until the temporary ones are all sorted)
				for(i = 0; i < 7; i++){
					writeHighScore(i);
				}
				for(i = 0; i < 11; i++){
					Entered_Name[i] = ' ';
				}
				Entered_Name_Length = 0;
				Entered_Score = 0;
				Working_Letter = 'A';
				Name_Finished = 0;
				Entered_Name_Width = 0;
				Working_Letter_Width = 5;

				State = GAME_OVER;
				Screen_Pause = 0;
				lcdClear();
			}
		} else if(State == HIGH_SCORES){
			if(centerButtonRE) {
				lcdScreenInvert();
				Screen_Pause = 1;
			} else if(centerButtonFE){
				lcdScreenReset();
				State = MAIN_MENU;
				lcdClear();
				Screen_Pause = 0;
			}
		}
		TMR1IF = 0; // Signal that the timer interrupt was handled
	}
}

extern init(void){
	// set clock speed
	OSCCON = 0b11111000;
	#ifdef OVERCLOCK
		OSCTUNE = 0b00011111; // max speed about 33MHz
	#endif

	// option reg
	OPTION_REG &=~ 0x80; // pullups available

	// configure analog
	ANSELA = 0;	// all pins digital
	ANSELB = 0;
	ANSELC = 0;

	// configure ports
	TRISC  = 0;	 		// all of portC is output
	LATC = (1 << 2);	// CS is high, all else is low

	TRISA = 0x07;	// all of porta0..porta2
	WPUA = 0x07;	// pullups on A0..A2

	TMR1H = 0; // Clear timer registers before setting interrupts
	TMR1L = 0; // Other half of timer registers
	PIR1 = 0; // Clear the interrupt bits
	PIR2 = 0;
	PIR4 = 0;
	INTCON = 0b11000000; // GIE and PEIE set to turn timer 1 interrupts on
	PIE1 = 0b00000001; // TMR1IE on
	T1CON = 0b01000101; // System clock, no prescale, oscillator off, don't use asynchronous mode, timer 1 on


	//init LED as per spec
	PSMC1PR = 0xFFFF;
    PSMC1DC = 0; /* start atr= 0 brightness */
    PSMC1PH = 0;
    PSMC1CLK = 0x01; //64MHz clock
    PSMC1STR0bits.P1STRA = 1;
    PSMC1POLbits.P1POLA = 0;
    PSMC1OENbits.P1OEA = 1;
    PSMC1PRSbits.P1PRST = 1;
    PSMC1PHSbits.P1PHST = 1;
    PSMC1DCSbits.P1DCST = 1;
    PSMC1CON = 0b11000000;
}

void main(void){
	init();
	lcdOn();
	
	uint8_t index;
	for(index = 0; index < 7; index++){
		readHighScore(index);
	}
	{
		static const uint8_t triangleSelect[] = {0x7c, 0x38, 0x10};
		static const uint8_t zeroes[] = {0x00, 0x00, 0x00};
		char levelString[] = {'L', 'e', 'v', 'e', 'l', ' ', ' ', ' '};
		lcdClear();

		resetBricks();
		while(1){
			if(Screen_Pause){
				continue;
			} else if(State == MAIN_MENU){
				printSmallString(0, 3, "Star Cruiser Omega's", 20);
				writeCosmic(8, 20);
				writeBreakout(24, 10);
				printSmallString(48, 13, "Play", 4);
				printSmallString(48, 44, "High Scores", 11);

				if(Menu_Selected == MENU_PLAY){
					lcdManyPixels(48, 8, triangleSelect, 3);
					lcdManyPixels(48, 39, zeroes, 3);
				} else {
					lcdManyPixels(48, 8, zeroes, 3);
					lcdManyPixels(48, 39, triangleSelect, 3);
				}
			} else if(State == LEVEL_SCREEN){
				if(Level_Redraw){
					lcdClear();
					Level_Redraw = 0;
				}
				if(0 < Level && Level < 10){
					levelString[6] = '0' + Level;
					printSmallString(32, 32, levelString, 7);
				} else if(10 <= Level && Level <= 99){
					levelString[6] = '0' + Level / 10;
					levelString[7] = '0' + Level % 10;
					printSmallString(32, 30, levelString, 8);
				} else if(Level <= 0){
					printSmallString(0, 0, "Oh noes Level is non positive", 29);
				} else {
					printSmallString(0, 0, "Level exceeded 99", 17);
				}
			} else if(State == GAME_SCREEN){
				// Write score
				uint32_t score;
				do {
					score = Game_Score;
				} while(Game_Score != score);
				if(score != Old_Score){
					printScore(56, 4, score);
					Old_Score = score;
				}
				// Draw lives
				lcdClearColumns(56, 44, 54 - 6 * Game_Lives);
				putLives(Game_Lives);
				// Read ball location
				uint8_t ballCol;
				do {
					ballCol = Ball_Location_Col;
				} while(Ball_Location_Col != ballCol);
				uint8_t ballRow;
				do {
					ballRow = Ball_Location_Row;
				} while(Ball_Location_Row != ballRow);
				uint8_t puX;
				do {
					puX = Powerup_X;
				} while(Powerup_X != puX);
				uint8_t puRow;
				do {
					puRow = Powerup_Row;
				} while(Powerup_Row != puRow);
				uint8_t puSpeed;
				do {
					puSpeed = Powerup_Speed;
				} while(Powerup_Speed != puSpeed);
				// Draw the paddle
				putPaddle(Paddle_Col, ballRow, ballCol, puRow, puX, Paddle_Size);
				// Draw the bricks (plus lower empty rows)
				uint8_t i, j;
				for(i = 0; i < 12; i++){
					for(j = 0; j < 6; j++){
						if(Bricks[j * 2][i] != Drawn_Bricks[j * 2][i] || Bricks[j * 2 + 1][i] != Drawn_Bricks[j * 2 + 1][i] || Redraw){
							putBrickTile(i, j, Bricks[j * 2][i], Bricks[j * 2 + 1][i]);
							Drawn_Bricks[j * 2][i] = Bricks[j * 2][i];
							Drawn_Bricks[j * 2 + 1][i] = Bricks[j * 2 + 1][i];
						}
					}
				}
				uint8_t puY = puRow / 8;
				if(OLD_Powerup_Row < 48 && puY != OLD_Powerup_Row / 8){
					putBrickTile(puX, OLD_Powerup_Row / 8, Bricks[OLD_Powerup_Row / 8 * 2][puX], Bricks[OLD_Powerup_Row / 8 * 2 + 1][puX]);
				}
				if(puSpeed){
					// Draw the top and bottom of the powerup as long as it's above the paddle row
					if(puY < 6){
						putBrickTileWithPowerup(puX, puY, Bricks[puY * 2][puX], Bricks[puY * 2 + 1][puX], puRow, puX);
					}
					if(puY < 5){
						putBrickTileWithPowerup(puX, puY + 1, Bricks[(puY + 1) * 2][puX], Bricks[(puY + 1) * 2 + 1][puX], puRow, puX);
					}
					OLD_Powerup_Row = puRow;
				}

				// Figure out which tiles to redraw
				uint8_t ballTopY = ballRow / 8;
				uint8_t ballBottomY = (ballRow + 5) / 8;
				uint8_t ballLeftX = (ballCol - 3) / 8;
				uint8_t ballRightX = (ballCol + 2) / 8;
				if(ballTopY == ballBottomY){
					if(ballLeftX == ballRightX){
						// Everything is equal and we redraw one tile
						//  _
						// |_|
						putBrickTileWithBall(ballLeftX, ballTopY, Bricks[ballTopY * 2][ballLeftX], Bricks[ballTopY * 2 + 1][ballLeftX], ballRow, ballCol, puRow, puX, Powerup_Speed);
					} else {
						// Redraw two tiles side-by-side
						//  _ _
						// |_|_|
						putBrickTileWithBall(ballLeftX, ballTopY, Bricks[ballTopY * 2][ballLeftX], Bricks[ballTopY * 2 + 1][ballLeftX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						putBrickTileWithBall(ballRightX, ballTopY, Bricks[ballTopY * 2][ballRightX], Bricks[ballTopY * 2 + 1][ballRightX], ballRow, ballCol, puRow, puX, Powerup_Speed);
					}
				} else {
					if(ballLeftX == ballRightX){
						// Redraw two tiles top and bottom
						//  _
						// |_|
						// |_|
						putBrickTileWithBall(ballLeftX, ballTopY, Bricks[ballTopY * 2][ballLeftX], Bricks[ballTopY * 2 + 1][ballLeftX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						putBrickTileWithBall(ballLeftX, ballBottomY, Bricks[ballBottomY * 2][ballLeftX], Bricks[ballBottomY * 2 + 1][ballLeftX], ballRow, ballCol, puRow, puX, Powerup_Speed);
					} else {
						// Redraw all four tiles
						//  _ _
						// |_|_|
						// |_|_|
						putBrickTileWithBall(ballLeftX, ballTopY, Bricks[ballTopY * 2][ballLeftX], Bricks[ballTopY * 2 + 1][ballLeftX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						putBrickTileWithBall(ballRightX, ballTopY, Bricks[ballTopY * 2][ballRightX], Bricks[ballTopY * 2 + 1][ballRightX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						putBrickTileWithBall(ballLeftX, ballBottomY, Bricks[ballBottomY * 2][ballLeftX], Bricks[ballBottomY * 2 + 1][ballLeftX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						putBrickTileWithBall(ballRightX, ballBottomY, Bricks[ballBottomY * 2][ballRightX], Bricks[ballBottomY * 2 + 1][ballRightX], ballRow, ballCol, puRow, puX, Powerup_Speed);				
					}
				}

				// We might have some other tiles to redraw
				uint8_t oldBallTopY = Old_Ball_Row / 8;
				uint8_t oldBallBottomY = (Old_Ball_Row + 5) / 8;
				uint8_t oldBallLeftX = (Old_Ball_Col - 3) / 8;
				uint8_t oldBallRightX = (Old_Ball_Col + 2) / 8;
				if(oldBallTopY == oldBallBottomY){
					if(oldBallLeftX == oldBallRightX){
						// Everything is equal and we redraw up to one tile
						//  _
						// |_|
						if(oldBallLeftX != ballLeftX || oldBallTopY != ballTopY){
							putBrickTileWithBall(oldBallLeftX, oldBallTopY, Bricks[oldBallTopY * 2][oldBallLeftX], Bricks[oldBallTopY * 2 + 1][oldBallLeftX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						}
					} else {
						// Redraw up to two tiles side-by-side
						//  _ _
						// |_|_|
						if(oldBallLeftX != ballLeftX || oldBallTopY != ballTopY){
							putBrickTileWithBall(oldBallLeftX, oldBallTopY, Bricks[oldBallTopY * 2][oldBallLeftX], Bricks[oldBallTopY * 2 + 1][oldBallLeftX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						}
						if(oldBallRightX != ballRightX || oldBallTopY != ballTopY){
							putBrickTileWithBall(oldBallRightX, oldBallTopY, Bricks[oldBallTopY * 2][oldBallRightX], Bricks[oldBallTopY * 2 + 1][oldBallRightX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						}
					}
				} else {
					if(oldBallLeftX == oldBallRightX){
						// Redraw up to two tiles top and bottom
						//  _
						// |_|
						// |_|
						if(oldBallLeftX != ballLeftX || oldBallTopY != ballTopY){
							putBrickTileWithBall(oldBallLeftX, oldBallTopY, Bricks[oldBallTopY * 2][oldBallLeftX], Bricks[oldBallTopY * 2 + 1][oldBallLeftX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						}
						if(oldBallLeftX != ballLeftX || oldBallBottomY != ballBottomY){
							putBrickTileWithBall(oldBallLeftX, oldBallBottomY, Bricks[oldBallBottomY * 2][oldBallLeftX], Bricks[oldBallBottomY * 2 + 1][oldBallLeftX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						}
					} else {
						// Redraw up to four tiles
						//  _ _
						// |_|_|
						// |_|_|
						if(oldBallLeftX != ballLeftX || oldBallTopY != ballTopY){
							putBrickTileWithBall(oldBallLeftX, oldBallTopY, Bricks[oldBallTopY * 2][oldBallLeftX], Bricks[oldBallTopY * 2 + 1][oldBallLeftX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						}
						if(oldBallRightX != ballRightX || oldBallTopY != ballTopY){
							putBrickTileWithBall(oldBallRightX, oldBallTopY, Bricks[oldBallTopY * 2][oldBallRightX], Bricks[oldBallTopY * 2 + 1][oldBallRightX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						}
						if(oldBallLeftX != ballLeftX || oldBallBottomY != ballBottomY){
							putBrickTileWithBall(oldBallLeftX, oldBallBottomY, Bricks[oldBallBottomY * 2][oldBallLeftX], Bricks[oldBallBottomY * 2 + 1][oldBallLeftX], ballRow, ballCol, puRow, puX, Powerup_Speed);
						}
						if(oldBallRightX != ballRightX || oldBallBottomY != ballBottomY){
							putBrickTileWithBall(oldBallRightX, oldBallBottomY, Bricks[oldBallBottomY * 2][oldBallRightX], Bricks[oldBallBottomY * 2 + 1][oldBallRightX], ballRow, ballCol, puRow, puX, Powerup_Speed);				
						}
					}
				}

				// Draw side borders
				if(Redraw){
					for(j = 0; j < 8; j++){
						lcdPixels(j * 8, 2, 0xff);
						lcdPixels(j * 8, 98, 0xff); 
					}
				}
				Old_Ball_Row = ballRow;
				Old_Ball_Col = ballCol;
				Redraw = 0;
			} else if(State == PAUSE_MENU) {
				printSmallString(24, 30, "Paused", 6);
				printSmallString(40, 17, "Resume", 6);
				printSmallString(40, 66, "Quit", 4);

				if(Pause_Selected == PAUSE_RESUME){
					lcdManyPixels(40, 13, triangleSelect, 3);
					lcdManyPixels(40, 62, zeroes, 3);
				} else {
					lcdManyPixels(40, 13, zeroes, 3);
					lcdManyPixels(40, 62, triangleSelect, 3);
				}
			} else if(State == GAME_OVER){
				if(Game_Won){
					printSmallString(16, 27, "Cool, you won!", 14);
					Game_Won = 0;
				}
				printSmallString(32, 29, "Game Over", 9);
			} else if(State == HIGH_SCORES){
				int i;
				for(i = 0; i < 7; i++){
					char currentName[11];
					int j;
					for(j = 0; j < 11; j++){
						currentName[j] = High_Score_Names[i][j];
					}
					printScore(i * 8, 0, High_Score_Values[i]);
					printSmallString(i * 8, 43, currentName, High_Score_Lengths[i]);
				}
				printSmallString(56, 7, "Go Back", 7);
				lcdManyPixels(56, 2, triangleSelect, 3);
			} else if(State == ENTER_NAME) {
				printSmallString(16, 0, "You got a high score!", 21);
				printSmallString(24, 0, "Enter Name:", 11);
				lcdClearColumns(32, Entered_Name_Width + Working_Letter_Width, 102);
				char enteredName[11];
				int i;
				for(i = 0; i < Entered_Name_Length; i++){
					enteredName[i] = Entered_Name[i];
				}
				enteredName[Entered_Name_Length] = Working_Letter;
				printSmallString(32, 0, enteredName, Entered_Name_Length + 1);
			}
		}
	}
	while(1);
}