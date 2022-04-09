#include <3ds.h>
#include <citro2d.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <time.h>
#include <cmath>

#define MAX_SPRITES 5
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240


typedef struct {
	C2D_Sprite spr;
	char type[]; 
} Sprite;

typedef struct {
	touchPosition x;
	touchPosition y;
} GridCoor;

static GridCoor checkCase(touchPosition x, touchPosition y);

static C2D_SpriteSheet spriteSheet;
static Sprite sprites[MAX_SPRITES];
static size_t numSprites = MAX_SPRITES/2;

time_t start = time(0);
double checkTime();
int creditsIndex(0);

static GridCoor gridPos;

static void initImages();
static int T3_DrawSprite(int type);

static touchPosition touch;

int gridCoor[3][3] = {0}; // 3x3 array of ints

int main(int argc, char** argv[])
{
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	consoleInit(GFX_TOP, NULL);

	const char *credits[2] = { "kitsou", "pvpb0t"};

	// Create screens
	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Load graphics
	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spriteSheet) svcBreak(USERBREAK_PANIC);

	// Initialize sprites
	initImages();

	// Main loop
	while (aptMainLoop())
	{
		//Starts listening to inputs
		hidScanInput();
		hidTouchRead(&touch);

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START) break; // break in order to return to hbmenu

		// Checks time, clears the console then outputs the time that has passed
		int timePassed = round(checkTime());
		creditsIndex = timePassed % 2;


		consoleClear();
		std::cout << "Time: " << timePassed << "\n"<< "Game by: " << credits[creditsIndex] << "\n:)";
	
		/*
		// Sets the position of the X/O
		Sprite* sprite = &sprites[spriteNbrIndex];
		C2D_SpriteSetPos(&sprite->spr, touch.px, touch.py);
		*/
		
		// draw frame
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, C2D_Color32f(0.0f, 0.5f, 0.0f, 1.0f));
		C2D_SceneBegin(top);
		//----------- BEGIN DRAWING -------------
				if (touch.px != 0 && touch.py != 0) T3_DrawSprite(IndexEachTick+1); // Draws eiher an X or an O
		T3_DrawSprite(0); // Draws the grid

		for (int y = 0; y < 3; y++) {
			for (int x = 0; x < 3; x++) {

				int xPos = ((SCREEN_WIDTH / 3) * x) + 35;
				int yPos = ((SCREEN_HEIGHT / 3) * y) + 35;

				switch (gridCoor[x][y]) {
					case 0:
						break;
					
					case 1: {
						Sprite* sprite = &sprites[1];
						C2D_SpriteSetPos(&sprite->spr, xPos, yPos);
						T3_DrawSprite(1);
						break;
					}

					case 2: {
						Sprite* sprite = &sprites[2];
						C2D_SpriteSetPos(&sprite->spr, xPos, yPos);
						T3_DrawSprite(2);
						break;
					}
					
					default:
						break;
				}
			}	
		}
		

		/* if (touch.px != 0 && touch.py != 0) T3_DrawSprite(spriteNbrIndex); // Draws eiher an X or an O */
		//------------ END DRAWING --------------
		C3D_FrameEnd(0);
		
		//Wait for VBlank
		gspWaitForVBlank();
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

double checkTime(){
	return difftime(time(0), start); 
}

static void initImages(){
	
	// size_t numImages = C2D_SpriteSheetCount(spriteSheet);

	// what

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

static GridCoor checkCase(touchPosition x, touchPosition y) {
	gridPos.x = x / (SCREEN_WIDTH / 3);
	gridPos.y = y / (SCREEN_HEIGHT / 3);
	return gridPos;
}