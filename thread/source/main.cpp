#include <3ds.h>
#include <string.h>
#include <sstream>
#include <cstdio>

u8 threadstack[0x4000] __attribute__((aligned(8)));
u8 threadstack2[0x4000] __attribute__((aligned(8)));
Handle Forever;

void cmd_thread_func(u32) {
    svcWaitSynchronization(Forever, U64_MAX);
    svcClearEvent(Forever);
    svcExitThread();
}

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
    
    svcCreateEvent(&Forever, 0); 
    
    Handle thread;
    Handle thread2;
    
    // Create 2 threads
    Result tr1 = svcCreateThread(&thread, cmd_thread_func, 0x0,
                    (u32*)(threadstack+0x4000),
                    0x18, 1);
                    
    Result tr2 = svcCreateThread(&thread2, cmd_thread_func, 0x0,
                    (u32*)(threadstack2+0x4000),
                    0x18, 1);
    
    u32 id, id2, id3;
    Result r = svcGetThreadId(&id, thread);
    Result r2 = svcGetThreadId(&id2, thread2);
    
    // Use an invalid thread id
    Result r3 = svcGetThreadId(&id3, 0);
    
    FILE* f = fopen("sdmc:/thread_output.txt", "w");
    if (f)
    {
        std::stringstream ss;
        ss << "Thread handle 1: " << (u32)thread << " Creation result: " << (u32)tr1 << " Id: " << id << " Result: " << (u32)r << "\n";
        ss << "Thread handle 2: " << (u32)thread2 << " Creation result: " << (u32)tr2 << " Id: " << id2 << " Result: " << (u32)r2 << "\n";
        ss << "Thread handle 3: 0 Id: " << id3 << " Result: " << (u32)r3 << "\n";
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
        // Check if both thread ids match the thread handle. Result: They don't
        if (r == 0 && r2 == 0 && (Handle)id == thread && (Handle)id2 == thread2)
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
