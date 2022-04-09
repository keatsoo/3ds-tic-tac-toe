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

static C2D_SpriteSheet spriteSheet;
static Sprite sprites[MAX_SPRITES];
static size_t numSprites = MAX_SPRITES/2;

time_t start = time(0);
double checkTime();
int spriteNbrIndex(0);

static void initImages();
static int T3_DrawSprite(int type);

static touchPosition touch;

int main(int argc, char** argv[])
{
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	consoleInit(GFX_TOP, NULL);

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
		hidScanInput();
		hidTouchRead(&touch);

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START) break; // break in order to return to hbmenu

		// Checks time, clears the console then outputs the time that has passed
		int timePassed = round(checkTime());
		consoleClear();
		std::cout << timePassed;

		spriteNbrIndex = timePassed % 3;

		if (spriteNbrIndex == 0) spriteNbrIndex++; //Doesnt let spriteNbrIndex be equal to 0 (0 is the grid)

		Sprite* sprite = &sprites[spriteNbrIndex];
		C2D_SpriteSetPos(&sprite->spr, touch.px, touch.py);
		
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, C2D_Color32f(0.0f, 0.5f, 0.0f, 1.0f));
		C2D_SceneBegin(top);
		//----------- BEGIN DRAWING -------------
		T3_DrawSprite(0); // Draws the grid 
		if (touch.px != 0 && touch.py != 0) T3_DrawSprite(spriteNbrIndex); // Draws eiher an X or an O
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

	for (size_t i = 1; i < MAX_SPRITES; i++)
	{
		Sprite* sprite = &sprites[i];

		C2D_SpriteFromSheet(&sprite->spr, spriteSheet, i);
		C2D_SpriteSetCenter(&sprite->spr, 0.5f, 0.5f);
	}

	Sprite* sprite = &sprites[0];

	C2D_SpriteFromSheet(&sprite->spr, spriteSheet, 0);
	C2D_SpriteSetCenter(&sprite->spr, 0.0f, 0.0f);
}

static int T3_DrawSprite(int type){
	C2D_DrawSprite(&sprites[type].spr);
	return 0;
}