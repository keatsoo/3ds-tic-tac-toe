#include <3ds.h>
#include <citro2d.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <time.h>

//Needs to be the right ammount of sprite, otherwise crash on the 3ds :(
#define MAX_SPRITES 4
#define MAX_MENU_SPRITES 6
//Sets the width and height of the screen
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

//Sprite class basicly
typedef struct {
	C2D_Sprite spr;
	char type[]; 
} Sprite;

int state = 0; // 0 = Main Menu, 1 = Game

//Local instance of Sprite and C2D_SpriteSheet
static C2D_SpriteSheet spriteSheet;
static Sprite sprites[MAX_SPRITES];

static C2D_SpriteSheet mainMenuSheet;
static Sprite menuSprites[MAX_MENU_SPRITES];
//static size_t numSprites = MAX_SPRITES/2;

//Timer
time_t start = time(0);
double checkTime();
int IndexEachTick(0);

//Has to initialize the functions
static void initImages();
static int T3_DrawSprite(int type);
static int T3_DRAWARROW(int x, int y);

static touchPosition touch;

//Starts from 0->2 (not 1->3 like i thought)
int gridCoor[3][3] = {0}; // 3x3 array of ints

int gameRound = 0; // What round are we in
int turn; // Whose turn it is, the only values are 1 (X) and 2 (O)


bool checkRange(int value, int lowest, int highest);
int hasWon();
bool gameTie();

int arrowPosX;
int arrowPosY;

//Main method
int main(int argc, char**)
{
	//Wrapper for \ref romfsMountSelf with the default "romfs" device name.
	romfsInit();
	//Initializes the LCD framebuffers with default parameters This is equivalent to calling: gfxInit(GSP_BGR8_OES,GSP_BGR8_OES,false);
	gfxInitDefault();
	//Initializes citro3D lib
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	//Initialize citro2d and sets max objects
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	//Prepares the GPU for rendering 2D content
	C2D_Prepare();
	//Initialise the console, what screen should be used (our case its the top one) and a pointer to the default console (null)
	consoleInit(GFX_TOP, NULL);

	//Array of our names, const since it wont change
	const char *credits[3] = {"kitsou", "pvpb0t", "Speedyrogue"};

	// Create screens
	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Load game graphics
	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spriteSheet) svcBreak(USERBREAK_PANIC);

	// Load main menu graphics
	mainMenuSheet = C2D_SpriteSheetLoad("romfs:/gfx/mainMenuSprites.t3x");
	if (!mainMenuSheet) svcBreak(USERBREAK_PANIC);

	// Initialize game sprites
	initImages();

	bool hasBeenPressed = false;
	int whosPressed = 0;
	bool online = false;
	//int animationStep = 0;

	//Init all homemenu Sprites by reassigning them to the same variable and setting their pos
	Sprite* menuSprite = &menuSprites[0];
	C2D_SpriteFromSheet(&menuSprite->spr, mainMenuSheet, 0);
	C2D_SpriteSetCenter(&menuSprite->spr, 0.5f, 0.0f);
	C2D_SpriteSetPos(&menuSprite->spr, SCREEN_WIDTH/2, 0.0f);

	menuSprite = &menuSprites[1];
	C2D_SpriteFromSheet(&menuSprite->spr, mainMenuSheet, 1);
	C2D_SpriteSetCenter(&menuSprite->spr, 0.5f, 0.0f);
	C2D_SpriteSetPos(&menuSprite->spr, SCREEN_WIDTH/2, 112.0f);

	menuSprite = &menuSprites[2];
	C2D_SpriteFromSheet(&menuSprite->spr, mainMenuSheet, 2);
	C2D_SpriteSetCenter(&menuSprite->spr, 0.5f, 0.0f);
	C2D_SpriteSetPos(&menuSprite->spr, SCREEN_WIDTH/2, 174.0f);

	menuSprite = &menuSprites[3];
	C2D_SpriteFromSheet(&menuSprite->spr, mainMenuSheet, 3);
	C2D_SpriteSetCenter(&menuSprite->spr, 0.5f, 0.0f);
	C2D_SpriteSetPos(&menuSprite->spr, SCREEN_WIDTH/2, 0.0f);

	menuSprite = &menuSprites[4];
	C2D_SpriteFromSheet(&menuSprite->spr, mainMenuSheet, 4);
	C2D_SpriteSetCenter(&menuSprite->spr, 0.5f, 0.5f);
	C2D_SpriteSetPos(&menuSprite->spr, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

	menuSprite = &menuSprites[5];
	C2D_SpriteFromSheet(&menuSprite->spr, mainMenuSheet, 5);
	C2D_SpriteSetCenter(&menuSprite->spr, 0.5f, 0.5f);
	C2D_SpriteSetPos(&menuSprite->spr, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

	int timePassed; 
	int timesHappened = 0;
	int timeStartLoading = 0;
	int mainButtonSelect = 0;
	int mainSelected = 0;

	// Main loop
	while (aptMainLoop()) {
		//If in main Menu
		if (state == 0) {
//=============================================================================================================
//================================================= MAIN MENU =================================================
//=============================================================================================================
			hidScanInput();
			hidTouchRead(&touch);

			u32 kDown = hidKeysDown();

			if (kDown & KEY_START) break;

			consoleClear();
			std::cout << "Press A to start the game\nPress START to quit.";
			std::cout << "\nTouch coordinates are " << touch.px << " ; " << touch.py;

			//Check what button be pressed
			if ((((SCREEN_WIDTH / 2) + ( (257 - 63) / 2 )) > touch.px && ((SCREEN_WIDTH / 2) - ( (257 - 63) / 2 )) < touch.px) && (147 > touch.py && 112 < touch.py)){  // ((SCREEN_WIDTH / 2) + ( (257 - 63) / 2 )) > touch.px && ((SCREEN_WIDTH / 2) - ( (257 - 63) / 2 )) < touch.px   
				hasBeenPressed = true;
				whosPressed = 1;
			} else if ((((SCREEN_WIDTH / 2) + ( (257 - 63) / 2 )) > touch.px && ((SCREEN_WIDTH / 2) - ( (257 - 63) / 2 )) < touch.px) && (209 > touch.py && 174 < touch.py)){ // (checkRange(touch.px, (SCREEN_WIDTH/2) - (257 - 63/2), (SCREEN_WIDTH/2) + (257 - 63/2)) && checkRange(touch.py, 174 + (147 - 113/2), 174 - (147 - 113/2)))
				hasBeenPressed = true;
				whosPressed = 2;
			}

			//Advance stage when clicking on singleplayer
			if (whosPressed == 1) state++;

			//Check time
			timePassed = round(checkTime());

			//If you pressed the button
			if (hasBeenPressed) {
				std::cout << "\nButton number " << whosPressed << " has been pressed.";
				//If it was online button
				if (whosPressed == 2) online = true;
				//waits 3 second
				if (timePassed % 5 > 3) {
					hasBeenPressed = false;
					online = false;
				}
			}

			//If press key up
			if (kDown & KEY_UP) {
				if (mainButtonSelect == 0) {mainButtonSelect++;} else {mainButtonSelect--;}
				//if down
			} else if (kDown & KEY_DOWN) {
				mainButtonSelect++;
			}

			//Check if mainButtonSelect is even
			int mainSelected = mainButtonSelect % 2;

			//If press a
			if (kDown & KEY_A) {
				hasBeenPressed = true;
				whosPressed = mainSelected + 1;
			}

			std::cout << "\nmainButtonSelect = " << mainButtonSelect << " and mainSelected is " << mainSelected;

			//Sprite pointer called sprite that points towards the memory adress of index 3 of the array sprites
			Sprite* arrowSprite = &sprites[3];

			//If even, move arrow to singleplayer
			if (mainSelected == 0) {
				C2D_SpriteSetPos(&arrowSprite->spr, 40, 130);
				//If odd, move arrow to online
			} else if (mainSelected == 1) {
				C2D_SpriteSetPos(&arrowSprite->spr, 40, 192);
			}

			//Move sprite (absolute), derefrensing the sprite and using the ptr in the Sprite struct as the pointer to the sprite, sets x and y pos
			// C2D_SpriteSetPos(&sprite->spr, ((SCREEN_WIDTH / 3) * x) + 5, ((SCREEN_WIDTH / 3.8) * y) + 35);
			

			// start frame
			C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
			C2D_TargetClear(top, C2D_Color32f(0.0f, 0.5f, 0.0f, 1.0f));
			C2D_SceneBegin(top); 

			//Draws the sprites
			C2D_DrawSprite(&menuSprites[0].spr);
			C2D_DrawSprite(&menuSprites[1].spr);
			C2D_DrawSprite(&menuSprites[2].spr);

			//If online is true, draws the online assets
			if (online) {
				C2D_DrawSprite(&menuSprites[3].spr);
				C2D_DrawSprite(&menuSprites[4].spr);
			}
			
			//Draws sprite 3
			T3_DrawSprite(3);
		
			C3D_FrameEnd(0);
			// end frame
				
			//Wait for VBlank
			gspWaitForVBlank();
		
		//If in loading
		} else if (state == 1) {
//=============================================================================================================
//=================================================== LOADING =================================================
//=============================================================================================================
			if (timesHappened == 0) {
				//Get time when boot in loading screen
				timeStartLoading = round(checkTime());
				timesHappened++;
			}
			//Checks how much time be passed after it was first loaded
			timePassed = round(checkTime());
			int timeLoading = timePassed - timeStartLoading;
			
			consoleClear();
			std::cout << "Loading game... (actually not but its just so \nu have time to remove ur pen)";
			//If time is 3 seconds, it goes to next game state
			if (timeLoading == 3) state++;

			C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
			C2D_TargetClear(top, C2D_Color32f(0.0f, 0.5f, 0.0f, 1.0f));
			C2D_SceneBegin(top); 
			//draws the loading screen
			C2D_DrawSprite(&menuSprites[5].spr);
		
			C3D_FrameEnd(0);

			//Wait for VBlank
			gspWaitForVBlank();
		} else if (state == 2) {

//=============================================================================================================
//=====================================================  GAME =================================================
//=============================================================================================================
			
			//Starts listening to inputs
			hidScanInput();
			hidTouchRead(&touch);

			//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
			u32 kDown = hidKeysDown();

			if (kDown & KEY_START) break; // break in order to return to hbmenu

			//If clicks select (resets game)
			if(kDown & KEY_SELECT){//Loops through all the squares
				for (int y = 0; y < 3; y++) {
					for (int x = 0; x < 3; x++) {
						//If they aint empty
						if(gridCoor[x][y] != 0){
							//Resets them
							gridCoor[x][y] = 0;
							gameRound = 0;
						}
					}
				}
			}
			


			// Checks time, clears the console then outputs the time that has passed
			timePassed = round(checkTime());
			//Either is 0 or 1, switches between each second
			IndexEachTick = timePassed % 3;

			//Clears the screen by using iprintf("
			consoleClear();
			//Prints to console the Time in seconds and Switching between index 0 and 1 in the credits array
			std::cout << "Time: " << timePassed << "\n"<< "Game by: " << credits[IndexEachTick] << " >:)";
			std::cout << "\nNavigate with the TouchPad, or move the \narrow with theDPad and A to confirm" << "\nPress Select to reset";

			// Saves in variable gridPos and prints the coordinates of the case where we're clicking
			int caseX = touch.px / (SCREEN_WIDTH / 3);
			int caseY = touch.py / (SCREEN_WIDTH / 3.7);
			std::cout << "\n\nYou are on the case " << caseX << " ; " << caseY;
			std::cout << "\nTouch coordinates are : " << touch.px << " ; " << touch.py;
			std::cout << "\nYou are on round " << gameRound << " and it is turn " << turn << ".";

			//If touch position is not 0 and the square the touched point is inside is empty (gridcoord[][] = 0)
			if (touch.px != 0 && touch.py != 0){
				if(gridCoor[caseX][caseY] == 0) {
					//Either is 1 or 2 depending on the turn
					gridCoor[caseX][caseY] = turn;
					//If the square is already clicked, write text to console
					gameRound++;
				} else {
					std::cout << "\nThis case is occupied.";
				}
			}

			//If button presses, changes the coords for the arrow.
			if(kDown & KEY_UP){
				if (arrowPosY != 0){arrowPosY--;}
			} else if (kDown & KEY_DOWN){
				if (arrowPosY != 2){arrowPosY++;}
			} else if(kDown & KEY_RIGHT){
				if (arrowPosX != 2){arrowPosX++;}
			} else if (kDown & KEY_LEFT){
				if (arrowPosX != 0){arrowPosX--;}
			}

			//If button A is pressed
			if(kDown & KEY_A){
				//If the arrow is on an empty square -> it will select it and mark it with the respective x/o for the turn being
				if (gridCoor[arrowPosX][arrowPosY] == 0){
					gridCoor[arrowPosX][arrowPosY] = turn;
					gameRound++;}
				} else {
					std::cout << "\nThis case is occupied.";
				}

			// Changes turn (alternates between 1 and 2)
			turn = (gameRound % 2) + 1;
	
			if(hasWon() != 0){
				if (hasWon() == 1) {
					std::cout << "\n\nX WON !";
				} else if (hasWon() == 2) {
					std::cout << "\n\nO WON !";
				}
			}

			if (gameTie()){
				std::cout << "\n\nTIE !";
			}

			
			// draw frame
			C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
			C2D_TargetClear(top, C2D_Color32f(0.0f, 0.5f, 0.0f, 1.0f));
			C2D_SceneBegin(top); 
			//----------- BEGIN DRAWING -------------
			T3_DrawSprite(0); // Draws the grid

			
			//Every frame:
			//Loops through all the squares in the 3x3 grid
			for (int y = 0; y < 3; y++) {
				for (int x = 0; x < 3; x++) {

					//Sets the x and y position
					int xPos = ((SCREEN_WIDTH / 3) * x) + 35;
					int yPos = ((SCREEN_HEIGHT / 3) * y) + 35;

					//Initializes a swich statement
					switch (gridCoor[x][y]) {
						//If is empty
						case 0:
							break;
						//If is a cross
						case 1: {
							//Draws the cross
							Sprite* sprite = &sprites[1];
							C2D_SpriteSetPos(&sprite->spr, xPos, yPos);
							T3_DrawSprite(1);
							break;
						}

						//If is a circle
						case 2: {
							//Draws the cricle
							Sprite* sprite = &sprites[2];
							C2D_SpriteSetPos(&sprite->spr, xPos, yPos);
							T3_DrawSprite(2);
							break;
						}
						
						//Else
						default:
							break;
					}
				}	
			}

			//Draws the arrow on grid 1,1
			T3_DRAWARROW(arrowPosX,arrowPosY);

			/* if (touch.px != 0 && touch.py != 0) T3_DrawSprite(spriteNbrIndex); // Draws eiher an X or an O */
			//------------ END DRAWING --------------
			C3D_FrameEnd(0);

			//Wait for VBlank
			gspWaitForVBlank();
		
		}
	}
	// Deinitialize sprites
		C2D_SpriteSheetFree(spriteSheet);

		// Deinitialize libraries
		C2D_Fini();
		C3D_Fini();
		gfxExit();
		romfsExit();
		return 0;
}
//Checks the time from 0 to now
double checkTime(){
	return difftime(time(0), start); 
}

//Function to draw the arrow
static int T3_DRAWARROW(int x, int y){

		//Sprite pointer called sprite that points towards the memory adress of index 3 of the array sprites
		Sprite* sprite = &sprites[3];
		//Move sprite (absolute), derefrensing the sprite and using the ptr in the Sprite struct as the pointer to the sprite, sets x and y pos
		C2D_SpriteSetPos(&sprite->spr, ((SCREEN_WIDTH / 3) * x) + 5, ((SCREEN_WIDTH / 3.8) * y) + 35);
		//draws sprite
		T3_DrawSprite(3);

	//Ends function
	return 0;
}

static void initImages(){
	
	// size_t numImages = C2D_SpriteSheetCount(spriteSheet);

	// Puts the center of the X and the O to the middle of the sprite
	//using size_t instead of int since it works like int and we dont have cast it to a size_t for the parameter when drawing the sprites
	for (size_t i = 1; i < MAX_SPRITES; i++)
	{
		//Local sprite = number i in the sprite array
		Sprite* sprite = &sprites[i];

		
		C2D_SpriteFromSheet(&sprite->spr, spriteSheet, i);
		C2D_SpriteSetCenter(&sprite->spr, 0.5f, 0.5f);
	}

	// Puts the center of the grid to the top left corner so it takes the whole bottom screen.
	Sprite* sprite = &sprites[0];
	C2D_SpriteFromSheet(&sprite->spr, spriteSheet, 0);
	C2D_SpriteSetCenter(&sprite->spr, 0.0f, 0.0f);
}

static int T3_DrawSprite(int type){
	C2D_DrawSprite(&sprites[type].spr);
	return 0;
}

bool checkRange(int value, int lowest, int highest) {
	return (value <= highest && value >= lowest);
}

int hasWon(){
	//	Horizontal
	for (int i = 0; i < 3; i++)
	{
		if (1 == gridCoor[i][i] && 1 == gridCoor[i][i+1] && 1 == gridCoor[i][i+2]){return gridCoor[i][i];}
	}

	// Vertical
	for (int i = 0; i < 3; i++)
	{
		if (1 == gridCoor[i][i] && 1 == gridCoor[i+1][i] && 1 == gridCoor[i+2][i]){return gridCoor[i][i];}
	}

	//Diagonal
	if (gridCoor[0][0] == 1 && gridCoor[1][1] == 1 && gridCoor[2][2] == 1){return 1;}
	if (gridCoor[2][0] == 1 && gridCoor[1][1] == 1 && gridCoor[0][2] == 1){return 1;}
	
	return 0;
}

bool gameTie() {
	int numberOfFullCases = 0;
	for (int y = 0; y < 3; y++){
		for (int x = 0; x < 3; x++){
			if (gridCoor[x][y] != 0) {
				numberOfFullCases++;
			}
		}
	}

	if (numberOfFullCases >= 9){
		if(hasWon() == 0) {
			return true;
		}
	}
	
	return false;
}