#include <3ds.h>
#include <string.h>
#include <sstream>
#include <cstdio>
#define NUM_THREADS 8

u8 threadstack[NUM_THREADS][0x4000] __attribute__((aligned(8)));
Handle Forever;
Handle Forever2;
Handle mutex;
FILE* f = nullptr;
u8 thread_data[NUM_THREADS] = {};

void Log(std::string const& log, u32 d) {
    std::stringstream ss;
    ss << log << d << "\n";
    fwrite(ss.str().c_str(), ss.str().size(), 1, f);
}

void cmd_thread_func(u32 tr) {
    
    //Log("Waiting to get mutex from thread ", tr);
    
    svcWaitSynchronization(mutex, U64_MAX);

    //Log("Mutex acquired from thread ", tr);
    
    thread_data[tr] = 1;
    svcSignalEvent(Forever);
    svcSleepThread(0); // Yield
    
    
    //svcReleaseMutex(mutex);
    
    //Log("Mutex released from thread ", tr);
   
   svcWaitSynchronization(Forever2, U64_MAX);
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
    svcCreateEvent(&Forever2, 0);
    svcCreateMutex(&mutex, true);
    
    Handle thread[NUM_THREADS];
    Result thread_results[NUM_THREADS];

    f = fopen("sdmc:/mutex_output.txt", "w");
    
    // Create 8 threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_results[i] = svcCreateThread(&thread[i], cmd_thread_func, i,
                                            (u32*)(threadstack[i]+0x4000),
                                            0x20, 0xFFFFFFFE);
    }
    
    svcReleaseMutex(mutex);
    
    svcWaitSynchronization(Forever, U64_MAX);
    svcClearEvent(Forever);

    std::stringstream ss;
    for (int i = 0; i < NUM_THREADS; ++i) {
        ss << "Thread " << i << " data: " << +thread_data[i] << "\n";
    }
    
    fwrite(ss.str().c_str(), ss.str().size(), 1, f);
    fclose(f);
    
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
