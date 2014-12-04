#include <3ds.h>
#include <string.h>
#include <sstream>
#include <cstdio>
#define NUM_THREADS 8

u8 threadstack[NUM_THREADS][0x40] __attribute__((aligned(8)));
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
    
    Handle thread[NUM_THREADS];
    Result thread_results[NUM_THREADS];
    
    // Create 8 threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_results[i] = svcCreateThread(&thread[i], cmd_thread_func, 0x0,
                                            (u32*)(threadstack[i]+0x40),
                                            0x20, 0xFFFFFFFE);
        svcSignalEvent(Forever);
        svcCloseHandle(thread_results[i]);
    }
    
    u32 id[NUM_THREADS];
    Result id_results[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i) {
        id_results[i] = svcGetThreadId(&id[i], thread[i]);
    }
    
    
    // Use an invalid thread id
    u32 id3;
    Result r3 = svcGetThreadId(&id3, 0);
    
    FILE* f = fopen("sdmc:/thread_output.txt", "w");
    if (f)
    {
        std::stringstream ss;
        for (int i = 0; i < NUM_THREADS; ++i) {
            ss << "Thread handle 1: " << std::hex << (u32)thread[i] << " Creation result: " << (u32)thread_results[i] << " Id: " << id[i] << " Result: " << (u32)id_results[i] << "\n";
        }
        
        ss << "Thread handle Invalid: 0 Id: " << id3 << " Result: " << (u32)r3 << "\n";
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
        if (thread_results[0] == 0 && thread_results[1] == 0 && (Handle)id[0] == thread[0] && (Handle)id[1] == thread[1])
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

    svcSignalEvent(Forever);
    svcClearEvent(Forever);
    
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
