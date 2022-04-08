#include <3ds.h>
#include <citro2d.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

typedef struct
{
	C2D_Sprite spr;
	char type[]; 
} GameImage;

time_t start = time(0);
double checkTime();

int main(int argc, char** argv[])
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START) break; // break in order to return to hbmenu

		consoleClear();
		printf(checkTime());

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();

		//Wait for VBlank
		gspWaitForVBlank();
	}

	gfxExit();
	return 0;
}

double checkTime(){
	return difftime(time(0), start); 
}