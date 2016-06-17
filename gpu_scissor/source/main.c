/**
* Hello Triangle example, made by Lectem
*
* Draws a white triangle using the 3DS GPU.
* This example should give you enough hints and links on how to use the GPU for basic non-textured rendering.
* Another version of this example will be made with colors.
*
* Thanks to smea, fincs, neobrain, xerpi and all those who helped me understand how the 3DS GPU works
*/


#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gpuframework.h"

#define BG_COLOR_U8 {0xFF,0xFF,0xFF,0xFF}

static const vertex_pos_col test_mesh1[] =
{
    // Green square
    {{0.0f, 0.0f , 0.5f}, {0x00, 0xFF, 0x00, 0xFF}, {-0.5f,-0.0f}},
    {{400.0f, 0.0f , 0.5f}, {0x00, 0xFF, 0x00, 0xFF}, {1.0f,-0.0f}},
    {{400.0f, 240.0f, 0.5f}, {0x00, 0xFF, 0x00, 0xFF}, {1.0f,2.5f}},

    {{400.0f, 240.0f, 0.5f}, {0x00, 0xFF, 0x00, 0xFF}, {-0.5f,2.5f}},
    {{0.0f, 240.0f, 0.5f}, {0x00, 0xFF, 0x00, 0xFF}, {1.0f,2.5f}},
    {{0.0f, 0.0f, 0.5f}, {0x00, 0xFF, 0x00, 0xFF}, {-0.5f,-0.0f}},
};

static void* test_data1 = NULL;
static void* test_data2 = NULL;
static void* test_data3 = NULL;

static u32* test_texture=NULL;
static const u16 test_texture_w=256;
static const u16 test_texture_h=256;

extern const struct {
    u32 width;
    u32 height;
    u32 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
    u8 pixel_data[256 * 256 * 4 + 1];
} texture_data;

int main(int argc, char** argv)
{
    srvInit();
    aptInit();
    hidInit(NULL);
    sdmcInit();

    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);


    gpuUIInit();

    test_data1 = linearAlloc(sizeof(test_mesh1));     //allocate our vbo on the linear heap
    memcpy(test_data1, test_mesh1, sizeof(test_mesh1)); //Copy our data

    //Allocate a RGBA8 texture with dimensions of 1x1
    test_texture = linearMemAlign(test_texture_w*test_texture_h*sizeof(u32),0x80);

    printf("Testing scissor test (Registers 0x65-0x67), press START to go to the next test.\n");

    int i;	
    copyTextureAndTile((u8*)test_texture, texture_data.pixel_data, texture_data.width, texture_data.height);

    if (!test_texture)
    printf("couldn't allocate test_texture\n");

    printf("Testing Exclude mode\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if (keys & KEY_START)
            break; // Stop the current test when Start is pressed

        gpuStartFrame();

        GPU_SetTexEnv(
            0,
            GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
            GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
            GPU_TEVOPERANDS(0, 0, 0),
            GPU_TEVOPERANDS(0, 0, 0),
            GPU_REPLACE, GPU_REPLACE,
            0
        );

        // Setup the buffers data
        GPU_SetAttributeBuffers(
            3, // number of attributes
            (u32*) osConvertVirtToPhys((u32)test_data1),
            GPU_ATTRIBFMT(0, 3, GPU_FLOAT) | GPU_ATTRIBFMT(1, 4, GPU_UNSIGNED_BYTE) | GPU_ATTRIBFMT(2, 2, GPU_FLOAT),
            0xFFF8, // Attribute mask, in our case 0b1110 since we use only the first one
            0x210, // Attribute permutations (here it is the identity)
            1, // number of buffers
            (u32[]) {0x0}, // buffer offsets (placeholders)
            (u64[]) {0x210}, // attribute permutations for each buffer (identity again)
            (u8[]) {3} // number of attributes for each buffer
        );

        // Set the scissor box
        GPU_SetScissorTest(1, 0, 0, 240, 400);

        // Draw the first triangle of the first quad
        GPU_DrawArray(GPU_TRIANGLES, 0, 6);

        gpuEndFrame();
    } while(aptMainLoop());

    printf("Testing Include mode\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if (keys & KEY_START)
            break; // Stop the current test when Start is pressed

        gpuStartFrame();

        GPU_SetTexEnv(
            0,
            GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
            GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
            GPU_TEVOPERANDS(0, 0, 0),
            GPU_TEVOPERANDS(0, 0, 0),
            GPU_REPLACE, GPU_REPLACE,
            0
        );

        // Setup the buffers data
        GPU_SetAttributeBuffers(
            3, // number of attributes
            (u32*) osConvertVirtToPhys((u32)test_data1),
            GPU_ATTRIBFMT(0, 3, GPU_FLOAT) | GPU_ATTRIBFMT(1, 4, GPU_UNSIGNED_BYTE) | GPU_ATTRIBFMT(2, 2, GPU_FLOAT),
            0xFFF8, // Attribute mask, in our case 0b1110 since we use only the first one
            0x210, // Attribute permutations (here it is the identity)
            1, // number of buffers
            (u32[]) {0x0}, // buffer offsets (placeholders)
            (u64[]) {0x210}, // attribute permutations for each buffer (identity again)
            (u8[]) {3} // number of attributes for each buffer
        );

        // Set the scissor box
        GPU_SetScissorTest(3, 0, 0, 240, 400);

        // Draw the first triangle of the first quad
        GPU_DrawArray(GPU_TRIANGLES, 0, 6);

        gpuEndFrame();
    } while(aptMainLoop());

    printf("Testing Include mode offset from the origin\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if (keys & KEY_START)
            break; // Stop the current test when Start is pressed

        gpuStartFrame();

        GPU_SetTexEnv(
            0,
            GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
            GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
            GPU_TEVOPERANDS(0, 0, 0),
            GPU_TEVOPERANDS(0, 0, 0),
            GPU_REPLACE, GPU_REPLACE,
            0
        );

        // Setup the buffers data
        GPU_SetAttributeBuffers(
            3, // number of attributes
            (u32*) osConvertVirtToPhys((u32)test_data1),
            GPU_ATTRIBFMT(0, 3, GPU_FLOAT) | GPU_ATTRIBFMT(1, 4, GPU_UNSIGNED_BYTE) | GPU_ATTRIBFMT(2, 2, GPU_FLOAT),
            0xFFF8, // Attribute mask, in our case 0b1110 since we use only the first one
            0x210, // Attribute permutations (here it is the identity)
            1, // number of buffers
            (u32[]) {0x0}, // buffer offsets (placeholders)
            (u64[]) {0x210}, // attribute permutations for each buffer (identity again)
            (u8[]) {3} // number of attributes for each buffer
        );

        // Set the scissor box
        GPU_SetScissorTest(3, 0, 200, 240, 300);

        // Draw the first triangle of the first quad
        GPU_DrawArray(GPU_TRIANGLES, 0, 6);

        gpuEndFrame();
    } while(aptMainLoop());

    if (test_data1)
        linearFree(test_data1);

    if (test_data2)
        linearFree(test_data2);
    
    if (test_data3)
        linearFree(test_data3);
    
    if (test_texture)
        linearFree(test_texture);

    gpuUIExit();


    gfxExit();
    sdmcExit();
    hidExit();
    aptExit();
    srvExit();

    return 0;
}
