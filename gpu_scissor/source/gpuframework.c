/**
 *@file gpuframework.cpp
 *@author Lectem
 *@date 17/05/2015
 */
#include "gpuframework.h"
#include <3ds.h>

#include "shader_vsh_shbin.h"
#include "3dutils.h"
#include "mmath.h"

void _my_assert(char * text)
{
    printf("%s\n",text);
    do{
        hidScanInput();
        if(keysDown()&KEY_START)break;
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }while(aptMainLoop());
    //should stop the program and clean up our mess
}



#define GPU_CMD_SIZE 0x40000

//GPU framebuffer address
u32*gpuColorBuffer =NULL;
//GPU depth buffer address
u32* gpuDBuffer =NULL;

//GPU command buffers
u32* gpuCmd = NULL;

//shader structure
DVLB_s* shader_dvlb;    //the header
shaderProgram_s shader; //the program


Result projUniformRegister      =-1;
Result modelviewUniformRegister =-1;


//The color used to clear the screen
u32 clearColor=0;//RGBA8(0xFF, 0x00, 0x80, 0xFF);

//The projection matrix
static float ortho_matrix[4*4];


void gpuDisableEverything()
{

    GPU_SetFaceCulling(GPU_CULL_NONE);
    //No stencil test
    GPU_SetStencilTest(false, GPU_ALWAYS, 0x00, 0xFF, 0x00);
    GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP);
    //No blending color
    GPU_SetBlendingColor(0,0,0,0);
    //Fake disable AB. We just ignore the Blending part
    GPU_SetAlphaBlending(
            GPU_BLEND_ADD,
            GPU_BLEND_ADD,
            GPU_ONE, GPU_ZERO,
            GPU_ONE, GPU_ZERO
    );

    GPU_SetAlphaTest(false, GPU_ALWAYS, 0x00);

    GPU_SetDepthTestAndWriteMask(true, GPU_ALWAYS, GPU_WRITE_ALL);
    GPUCMD_AddMaskedWrite(GPUREG_0062, 0x1, 0);
    GPUCMD_AddWrite(GPUREG_0118, 0);

    GPU_SetDummyTexEnv(0);
    GPU_SetDummyTexEnv(1);
    GPU_SetDummyTexEnv(2);
    GPU_SetDummyTexEnv(3);
    GPU_SetDummyTexEnv(4);
    GPU_SetDummyTexEnv(5);
}


void gpuUIInit()
{

    //Allocate the GPU render buffers
    gpuColorBuffer = vramMemAlign(400*240*8, 0x100);
    gpuDBuffer = vramMemAlign(400*240*8, 0x100);

    GPU_Init(NULL);//initialize GPU

    gfxSet3D(false);//We will not be using the 3D mode in this example
    Result res=0;

    /**
    * Load our vertex shader and its uniforms
    * Check http://3dbrew.org/wiki/SHBIN for more informations about the shader binaries
    */
    shader_dvlb = DVLB_ParseFile((u32 *)shader_vsh_shbin, shader_vsh_shbin_size);//load our vertex shader binary
    my_assert(shader_dvlb != NULL);
    shaderProgramInit(&shader);
    res = shaderProgramSetVsh(&shader, &shader_dvlb->DVLE[0]);
    my_assert(res >=0); // check for errors

    //In this example we are only rendering in "2D mode", so we don't need one command buffer per eye
    gpuCmd=(u32*)linearAlloc(GPU_CMD_SIZE * (sizeof *gpuCmd) ); //Don't forget that commands size is 4 (hence the sizeof)
    my_assert(gpuCmd != NULL);

    //Reset the gpu
    //This actually needs a command buffer to work, and will then use it as default
    GPU_Reset(NULL, gpuCmd, GPU_CMD_SIZE);

    projUniformRegister = shaderInstanceGetUniformLocation(shader.vertexShader, "projection");
    my_assert(projUniformRegister != -1); // make sure we did get the uniform


    shaderProgramUse(&shader); // Select the shader to use

    initOrthographicMatrix(ortho_matrix, 0.0f, 400.0f, 0.0f, 240.0f, 0.0f, 1.0f); // A basic projection for 2D drawings
    SetUniformMatrix(projUniformRegister, ortho_matrix); // Upload the matrix to the GPU

    GPU_DepthMap(-1.0f, 0.0f);  //Be careful, standard OpenGL clipping is [-1;1], but it is [-1;0] on the pica200
    // Note : this is corrected by our projection matrix !

    gpuDisableEverything();

    //Flush buffers and setup the environment for the next frame
    gpuEndFrame();

}

void gpuUIExit()
{
    //do things properly
    linearFree(gpuCmd);
    shaderProgramFree(&shader);
    DVLB_Free(shader_dvlb);
    GPU_Reset(NULL, gpuCmd, GPU_CMD_SIZE); // Not really needed, but safer for the next applications ?
    vramFree(gpuColorBuffer);
    vramFree(gpuDBuffer);
}

void gpuStartFrame()
{

    //Get ready to start a new frame
    GPUCMD_SetBufferOffset(0);

    //Viewport (http://3dbrew.org/wiki/GPU_Commands#Command_0x0041)
    GPU_SetViewport((u32 *)osConvertVirtToPhys((u32)gpuDBuffer),
                    (u32 *)osConvertVirtToPhys((u32) gpuColorBuffer),
                    0, 0,
            //Our screen is 400*240, but the GPU actually renders to 400*480 and then downscales it SetDisplayTransfer bit 24 is set
            //This is the case here (See http://3dbrew.org/wiki/GPU#0x1EF00C10 for more details)
                    240*2, 400);

    //Clear the screen
    GX_SetMemoryFill(NULL, gpuColorBuffer, clearColor, &gpuColorBuffer[0x2EE00],
                     0x201, gpuDBuffer, 0x00000000, &gpuDBuffer[0x2EE00], 0x201);
    gspWaitForPSC0();

}

void gpuEndFrame()
{
    //Ask the GPU to draw everything (execute the commands)
    GPU_FinishDrawing();
    GPUCMD_Finalize();
    GPUCMD_FlushAndRun(NULL);
    gspWaitForP3D();//Wait for the gpu 3d processing to be done
    //Copy the GPU output buffer to the screen framebuffer
    //See http://3dbrew.org/wiki/GPU#Transfer_Engine for more details about the transfer engine

    GX_SetDisplayTransfer(NULL, gpuColorBuffer, 0x019001E0, (u32*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 0x019001E0, 0x01001000);
    gspWaitForPPF();

    gfxSwapBuffersGpu();

    //Wait for the screen to be updated
    gspWaitForVBlank();
}





void GPU_SetDummyTexEnv(u8 num)
{
    //Don't touch the colors of the previous stages
    GPU_SetTexEnv(num,
                  GPU_TEVSOURCES(GPU_PREVIOUS, GPU_PREVIOUS, GPU_PREVIOUS),
                  GPU_TEVSOURCES(GPU_PREVIOUS, GPU_PREVIOUS, GPU_PREVIOUS),
                  GPU_TEVOPERANDS(0,0,0),
                  GPU_TEVOPERANDS(0,0,0),
                  GPU_REPLACE,
                  GPU_REPLACE,
                  0xFFFFFFFF);
}




// Grabbed from Citra Emulator (citra/src/video_core/utils.h)
static inline u32 morton_interleave(u32 x, u32 y)
{
	u32 i = (x & 7) | ((y & 7) << 8); // ---- -210
	i = (i ^ (i << 2)) & 0x1313;      // ---2 --10
	i = (i ^ (i << 1)) & 0x1515;      // ---2 -1-0
	i = (i | (i >> 7)) & 0x3F;
	return i;
}

//Grabbed from Citra Emulator (citra/src/video_core/utils.h)
static inline u32 get_morton_offset(u32 x, u32 y, u32 bytes_per_pixel)
{
    u32 i = morton_interleave(x, y);
    unsigned int offset = (x & ~7) * 8;
    return (i + offset) * bytes_per_pixel;
}


void copyTextureAndTile(u8* dst,u8 * src,int w ,int h)
{
	int i, j;
	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {

			u32 coarse_y = j & ~7;
			u32 dst_offset = get_morton_offset(i, j, 4) + coarse_y * w * 4;

			u32 v = ((u32 *)src)[i + (h - 1 - j)*w];
			*(u32 *)(dst + dst_offset) = __builtin_bswap32(v);
		}
	}
}
