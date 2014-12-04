#include <3ds.h>

int main()
{
	// Initialize services
	srvInit();
	aptInit();
	hidInit(NULL);
	gfxInit();
    ptmInit();
    
    Handle timer;
    svcCreateTimer(&timer, 2);
    svcSetTimer(timer, 3000000000, 3000000000);
    bool synched = false;
	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

        if (kDown & KEY_B)
            synched = false;

		// Please note that the 3DS screens are sideways (thus 240x400 and 240x320)
		u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
		memset(fb, 0, 240*400*3);
		fb[3*(10+10*240)] = 0xFF;
        Result res = svcWaitSynchronization(timer, 1);
        if (res == 0)
            synched = true;
        if (synched)
        {
            fb[3*(10+10*240)+1] = 0xFF;
            fb[3*(10+10*240)+2] = 0xFF;
        }
        else
        {
            fb[3*(10+10*240)+1] = 0;
            fb[3*(10+10*240)+2] = 0;
        }

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

    svcCancelTimer(timer);
    
	// Exit services
    ptmExit();
	gfxExit();
	hidExit();
	aptExit();
	srvExit();
	return 0;
}
