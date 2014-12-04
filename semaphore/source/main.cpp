#include <3ds.h>
#include <string.h>
#include <sstream>
#include <cstdio>

int main()
{
	// Initialize services
	srvInit();
	aptInit();
	hidInit(NULL);
	gfxInit();
    ptmInit();
    fsInit();
    sdmcInit();
    
    Handle semaphore;
    Handle invalidSemaphore;
    Result result = svcCreateSemaphore(&semaphore, 0, 2);
    s32 count, count2, count3;
    Result releaseR1 = svcReleaseSemaphore(&count, semaphore, 1);
    Result releaseR2 = svcReleaseSemaphore(&count2, semaphore, 1);
    Result releaseR3 = svcReleaseSemaphore(&count3, semaphore, 10);
    
    Result waitRes = svcWaitSynchronization(semaphore, 3000000000);
    
    // Now create an invalid semaphore
    s32 countInvalid;
    Result invalid = svcCreateSemaphore(&invalidSemaphore, 1, 0);
    Result releaseInvalid = svcReleaseSemaphore(&countInvalid, invalidSemaphore, 1);
    
    FILE* f = fopen("sdmc:/semaphore_output.txt", "w");
    if (f)
    {
        std::stringstream ss;
        ss << std::hex << "Semaphore creation result: " << (u32)result << " First wait result: " << (u32)waitRes << "\n";
        ss << "Result of the invalid semaphore: " << (u32)invalid << "\n";
        ss << "Result of release 1: " << (u32)releaseR1 << " Result of release 2: " << (u32)releaseR2 << " Result of release 3: " << (u32)releaseR3 << " Result of release invalid: " << (u32)releaseInvalid << "\n";
        ss << "Release count 1: " << count << " Release count 2: " << count2 << " Release count 3: " << count3 << " Release count invalid: " << countInvalid << "\n";
        fwrite(ss.str().c_str(), ss.str().size(), 1, f);
        fclose(f);
    }
    
	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
        memset(fb, 0, 240*400*3);
        
		fb[3*(10+10*240)] = 0xFF;
        if (1)
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

	// Exit services
    sdmcExit();
    fsExit();
    ptmExit();
	gfxExit();
	hidExit();
	aptExit();
	srvExit();
	return 0;
}
