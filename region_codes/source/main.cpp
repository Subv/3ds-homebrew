#include <3ds.h>
#include <unordered_map>
#include <sstream>
#include <cstdio>
#include <cstring>

int main()
{
	// Initialize services
	srvInit();
	aptInit();
	hidInit(NULL);
	gfxInit();
    initCfgu();
    fsInit();
    sdmcInit();
	//gfxSet3D(true); // uncomment if using stereoscopic 3D

    std::unordered_map<u16, u16> codes;
    // Get all the codes
    for (u16 i = 0; i < 0xBB; ++i)
    {
        u16 string;
        Result ret = CFGU_GetCountryCodeString(i, &string);
        if (ret == (Result)0)
            codes[i] = string;
    }
    
    // Check the result code for an out of bounds query, result: 3640722426
    u16 dummy;
    Result outOfBoundsResult = CFGU_GetCountryCodeString(0xBB, &dummy);
    // Check the result for an invalid country code query, result: 3640722426
    Result invalidQueryResult = CFGU_GetCountryCodeString(0, &dummy);
    
    // Check the result code for an invalid country string, result: 3640722426
    Result invalidCountryResult = CFGU_GetCountryCodeID(0, &dummy);
    
    FILE* f = fopen("sdmc:/region_codes.txt", "w");
    bool correct = false;
    if (f)
    {
        std::stringstream ss;
        ss << "Result of an out of bounds query: " << (u32)outOfBoundsResult << "\n";
        ss << "Result of an invalid query: " << (u32)invalidQueryResult << "\n";
        ss << "Result of an invalid country string: " << (u32)invalidCountryResult << "\n";
        fwrite(ss.str().c_str(), ss.str().size(), 1, f);
        
        correct = true;
        for (auto itr : codes)
        {
            std::stringstream stream;
            stream << "Code: " << itr.first << " String: " << itr.second << "\n";
            fwrite(stream.str().c_str(), stream.str().size(), 1, f);
        }
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

		// Example rendering code that displays a white pixel
		// Please note that the 3DS screens are sideways (thus 240x400 and 240x320)
		u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
		memset(fb, 0, 240*400*3);
		fb[3*(10+10*240)] = 0xFF;
        // Draw a red pixel if something went wrong
        if (correct)
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
    exitCfgu();
	gfxExit();
	hidExit();
	aptExit();
	srvExit();
	return 0;
}
