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

// Small green square
static const vertex_pos_col test_mesh1[] =
{
	{{100.0f, 70.0f , 0.5f}, {0x00, 0xFF, 0x00, 0xFF}, {-0.5f,-0.0f}},
	{{200.0f, 70.0f , 0.5f}, {0x00, 0xFF, 0x00, 0xFF}, {1.0f,-0.0f}},
	{{100.0f, 100.0f, 0.5f}, {0x00, 0xFF, 0x00, 0xFF}, {1.0f,2.5f}},
	{{200.0f, 70.0f , 0.5f}, {0x00, 0xFF, 0x00, 0xFF}, {1.0f,2.5f}},
	{{100.0f, 100.0f, 0.5f}, {0x00, 0xFF, 0x00, 0xFF}, {-0.5f,2.5f}},
	{{200.0f, 100.0f, 0.5f}, {0x00, 0xFF, 0x00, 0xFF}, {-0.5f,-0.0f}}
};

// Big red square
static const vertex_pos_col test_mesh2[] =
{
	{{0.0f  , 0.0f  , 0.5f}, {0xFF, 0x00, 0x00, 0xFF}, {-0.5f,-0.0f}},
	{{400.0f, 0.0f  , 0.5f}, {0xFF, 0x00, 0x00, 0xFF}, {1.0f,-0.0f}},
	{{400.0f, 240.0f, 0.5f}, {0xFF, 0x00, 0x00, 0xFF}, {1.0f,2.5f}},
	{{400.0f, 240.0f, 0.5f}, {0xFF, 0x00, 0x00, 0xFF}, {1.0f,2.5f}},
	{{0.0f  , 240.0f, 0.5f}, {0xFF, 0x00, 0x00, 0xFF}, {-0.5f,2.5f}},
	{{0.0f  , 0.0f  , 0.5f}, {0xFF, 0x00, 0x00, 0xFF}, {-0.5f,-0.0f}}
};
		
static void* test_data1 = NULL;
static void* test_data2 = NULL;

static u32* test_texture=NULL;
static const u16 test_texture_w=256;
static const u16 test_texture_h=256;

extern const struct {
  u32  	 width;
  u32  	 height;
  u32  	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  u8 	 pixel_data[256 * 256 * 4 + 1];
} texture_data;

void DrawSmallSquare() {
	//Setup the buffers data
	GPU_SetAttributeBuffers(
			3, // number of attributes
			(u32 *) osConvertVirtToPhys((u32) test_data1),
			GPU_ATTRIBFMT(0, 3, GPU_FLOAT)|GPU_ATTRIBFMT(1, 4, GPU_UNSIGNED_BYTE)|GPU_ATTRIBFMT(2, 2, GPU_FLOAT),
			0xFFF8,//Attribute mask, in our case 0b1110 since we use only the first one
			0x210,//Attribute permutations (here it is the identity)
			1, //number of buffers
			(u32[]) {0x0}, // buffer offsets (placeholders)
			(u64[]) {0x210}, // attribute permutations for each buffer (identity again)
			(u8[]) {3} // number of attributes for each buffer
	);
	
	//Display the buffers data
	GPU_DrawArray(GPU_TRIANGLES, sizeof(test_mesh1) / sizeof(test_mesh1[0]));
}

void DrawBigSquare() {
	GPU_SetAttributeBuffers(
			3, // number of attributes
			(u32 *) osConvertVirtToPhys((u32) test_data2),
			GPU_ATTRIBFMT(0, 3, GPU_FLOAT)|GPU_ATTRIBFMT(1, 4, GPU_UNSIGNED_BYTE)|GPU_ATTRIBFMT(2, 2, GPU_FLOAT),
			0xFFF8,//Attribute mask, in our case 0b1110 since we use only the first one
			0x210,//Attribute permutations (here it is the identity)
			1, //number of buffers
			(u32[]) {0x0}, // buffer offsets (placeholders)
			(u64[]) {0x210}, // attribute permutations for each buffer (identity again)
			(u8[]) {3} // number of attributes for each buffer
	);
	
	GPU_DrawArray(GPU_TRIANGLES, sizeof(test_mesh2) / sizeof(test_mesh2[0]));
}

int main(int argc, char** argv)
{

    srvInit();
    aptInit();
    hidInit(NULL);
    sdmcInit();

    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);


    gpuUIInit();

    printf("Starting stencil tests.\n");
	printf("Press START to go to the next test.\n");
	printf("Press B to exit.\n");
    test_data1 = linearAlloc(sizeof(test_mesh1));     //allocate our vbo on the linear heap
    memcpy(test_data1, test_mesh1, sizeof(test_mesh1)); //Copy our data
	
	test_data2 = linearAlloc(sizeof(test_mesh2));     //allocate our vbo on the linear heap
    memcpy(test_data2, test_mesh2, sizeof(test_mesh2)); //Copy our data
    //Allocate a RGBA8 texture with dimensions of 1x1
    test_texture = linearMemAlign(test_texture_w*test_texture_h*sizeof(u32),0x80);

    int i;	
	copyTextureAndTile((u8*)test_texture, texture_data.pixel_data, texture_data.width, texture_data.height);
	
    if (!test_texture)
		printf("couldn't allocate test_texture\n");
	
	printf("Test stencil operation 2 (GL_REPLACE).\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test the stencil operation 2
		// Hyphothesis:     The stencil operation 2 is GL_REPLACE.
		// Result if true:  The red square is drawn everywhere except on the pixels where the green square was already drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 15 as reference value.
		GPU_SetStencilTest(true, GPU_ALWAYS, 15, 0xFF, 0xFF);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 15.
		// If the hypothesis is correct, the stencil buffer will have a value of 15 where the green square was drawn,
		// preventing the drawing of the red square in that section.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 15, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());
	
	printf("Test stencil operation 5 (GL_INVERT).\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test the stencil operation 5
		// Hyphothesis:     The stencil operation 5 is GL_INVERT.
		// Result if true:  The red square is drawn everywhere except on the pixels where the green square was already drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 6 as reference value.
		GPU_SetStencilTest(true, GPU_ALWAYS, 6, 0xFF, 0xFF);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();
		
		// Invert the value of the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 5); // GPU_INVERT
		
		// Redraw the green square to update the stencil buffer
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 0xF9.
		// If the hypothesis is correct, the stencil buffer will have a value of 0xF9 (the inverse of 6 as an u8 value)
		// where the green square was drawn, preventing the drawing of the red square in that section.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 0xF9, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());
	
	printf("Test stencil operation 3 (GL_INCR).\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test the stencil operation 3
		// Hyphothesis:     The stencil operation 3 is GL_INCR.
		// Result if true:  The red square is drawn everywhere except on the pixels where the green square was already drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 6 as reference value.
		GPU_SetStencilTest(true, GPU_ALWAYS, 6, 0xFF, 0xFF);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();
		
		// Increment the value of the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 3); // GPU_INCR
		
		// Redraw the green square to update the stencil buffer
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 7.
		// If the hypothesis is correct, the stencil buffer will have a value of 7 (6 + 1)
		// where the green square was drawn, preventing the drawing of the red square in that section.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 7, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());
	
	printf("Test stencil operation 3 (GL_INCR) saturation.\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test the stencil operation 3 saturation
		// Hyphothesis:     The stencil operation 3 is GL_INCR, which saturates the increment to 255.
		// Result if true:  The red square is drawn everywhere except on the pixels where the green square was already drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 255 as reference value.
		GPU_SetStencilTest(true, GPU_ALWAYS, 255, 0xFF, 0xFF);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();
		
		// Increment the value of the stencil buffer where the green square is drawn
		// This should not do anything, as 255 + 1 is 255 when doing a saturated increment
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 3); // GPU_INCR
		
		// Redraw the green square to update the stencil buffer
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 255.
		// If the hypothesis is correct, the stencil buffer will have a value of 255
		// where the green square was drawn, preventing the drawing of the red square in that section.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 255, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());
	
	printf("Test stencil operation 4 (GL_DECR).\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test the stencil operation 4
		// Hyphothesis:     The stencil operation 4 is GL_DECR.
		// Result if true:  The red square is drawn everywhere except on the pixels where the green square was already drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 6 as reference value.
		GPU_SetStencilTest(true, GPU_ALWAYS, 6, 0xFF, 0xFF);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();
		
		// Decrement the value of the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 4); // GPU_DECR
		
		// Redraw the green square to update the stencil buffer
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 5.
		// If the hypothesis is correct, the stencil buffer will have a value of 5 (6 - 1)
		// where the green square was drawn, preventing the drawing of the red square in that section.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 5, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());
	
	printf("Test stencil operation 4 (GL_DECR) saturation.\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test the stencil operation 4 saturation
		// Hyphothesis:     The stencil operation 4 is GL_DECR, which saturates the decrement to 0.
		// Result if true:  The red square is not drawn, only the green one is drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 0 as reference value.
		GPU_SetStencilTest(true, GPU_ALWAYS, 0, 0xFF, 0xFF);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();
		
		// Decrement the value of the stencil buffer where the green square is drawn
		// This should not do anything, as 0 - 1 is 0 when doing a saturated decrement
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 4); // GPU_DECR
		
		// Redraw the green square to update the stencil buffer
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 0.
		// If the hypothesis is correct, the stencil buffer will have a value of 0 everywhere, 
		// preventing the drawing of the red square.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 0, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());
	
	printf("Test stencil operation 1 (GL_ZERO).\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test the stencil operation 1
		// Hyphothesis:     The stencil operation 1 is GL_ZERO.
		// Result if true:  The red square is not drawn, only the green one is drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 15 as reference value.
		GPU_SetStencilTest(true, GPU_ALWAYS, 15, 0xFF, 0xFF);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();
		
		// Set the value of the stencil buffer to 0 where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 1); // GPU_ZERO
		
		// Redraw the green square to update the stencil buffer
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 0.
		// If the hypothesis is correct, the stencil buffer will have a value of 0 everywhere, 
		// preventing the drawing of the red square.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 0, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());
	
	printf("Test stencil operation 6 (GL_INCR_WRAP).\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test the stencil operation 6
		// Hyphothesis:     The stencil operation 6 is GL_INCR_WRAP.
		// Result if true:  The red square is not drawn where the green square was drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 30 as reference value.
		GPU_SetStencilTest(true, GPU_ALWAYS, 30, 0xFF, 0xFF);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();
		
		// Increment the value of the stencil buffer where the green square is drawn.
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 6); // GPU_INCR_WRAP
		
		// Redraw the green square to update the stencil buffer
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 31.
		// If the hypothesis is correct, the stencil buffer will have a value of 0 everywhere, except where the green square was drawn
		// preventing the drawing of the red square in that section.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 31, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());
	
	printf("Test stencil operation 6 (GL_INCR_WRAP) wrap.\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test the stencil operation 6
		// Hyphothesis:     The stencil operation 6 is GL_INCR_WRAP.
		// Result if true:  The red square is not drawn, only the green one is drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 255 as reference value.
		GPU_SetStencilTest(true, GPU_ALWAYS, 255, 0xFF, 0xFF);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();
		
		// Increment the value of the stencil buffer where the green square is drawn.
		// This will cause it to wrap around to 0 (255 + 1 = 0)
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 6); // GPU_INCR_WRAP
		
		// Redraw the green square to update the stencil buffer
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 0.
		// If the hypothesis is correct, the stencil buffer will have a value of 0 everywhere, 
		// preventing the drawing of the red square.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 0, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());
	
	printf("Test stencil operation 7 (GL_DECR_WRAP).\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test the stencil operation 7
		// Hyphothesis:     The stencil operation 7 is GL_DECR_WRAP.
		// Result if true:  The red square is not drawn where the green square was drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 30 as reference value.
		GPU_SetStencilTest(true, GPU_ALWAYS, 30, 0xFF, 0xFF);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();
		
		// Decrement the value of the stencil buffer where the green square is drawn.
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 7); // GPU_DECR_WRAP
		
		// Redraw the green square to update the stencil buffer
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 29.
		// If the hypothesis is correct, the stencil buffer will have a value of 0 everywhere, except where the green square was drawn
		// preventing the drawing of the red square in that section.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 29, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());
	
	printf("Test stencil operation 7 (GL_DECR_WRAP) wrap.\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test the stencil operation 7
		// Hyphothesis:     The stencil operation 7 is GL_DECR_WRAP.
		// Result if true:  The red square is not drawn where the green square was drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 0 as reference value.
		GPU_SetStencilTest(true, GPU_ALWAYS, 0, 0xFF, 0xFF);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();
		
		// Decrement the value of the stencil buffer where the green square is drawn.
		// This will cause it to wrap around to 255 (0 - 1 = 255)
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 7); // GPU_DECR_WRAP
		
		// Redraw the green square to update the stencil buffer
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 255.
		// If the hypothesis is correct, the stencil buffer will have a value of 0 everywhere, except where the green square was drawn
		// preventing the drawing of the red square in that section.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 255, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());
	
	printf("Test stencil write mask.\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test the stencil write mask
		// Hyphothesis:     The stencil write mask will mask writes to the stencil buffer
		// Result if true:  The red square is drawn everywhere except on the pixels where the green square was already drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 15 as reference value.
		// Use a write mask of 3, so that only the 2 least significant bits are written to the buffer
		GPU_SetStencilTest(true, GPU_ALWAYS, 15, 0xFF, 0x3);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 3.
		// If the hypothesis is correct, the stencil buffer will have a value of 3 where the green square was drawn,
		// preventing the drawing of the red square in that section.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 3, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());
	
	printf("Test stencil zpass behavior when the depth test is disabled.\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test stencil behavior when depth testing is disabled.
		// Hyphothesis:     The stencil zpass function is executed even if the depth testing is disabled.
		// Result if true:  The red square is drawn everywhere except on the pixels where the green square was already drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Disable the depth test
		GPU_SetDepthTestAndWriteMask(false, GPU_ALWAYS, GPU_WRITE_ALL);
		
		// Enable the stencil test
		GPU_SetStencilTest(true, GPU_ALWAYS, 1, 0xFF, 0xFF);
		
		// Write 1 to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 1.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 1, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());

	printf("Test whether the input mask applies when writing the stencil value or not.\n");
    do {
        hidScanInput();
        u32 keys = keysDown();
        if(keys & KEY_START) break; // Stop the current test when Start is pressed
		if(keys & KEY_B) goto exit; // Exit when B is pressed

		GPUCMD_AddWrite(GPUREG_DEPTHBUFFER_FORMAT, 3); // Depth + Stencil
		
        gpuStartFrame();

        int texenvnum=0;
        GPU_SetTexEnv(
			texenvnum,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xAABBCCDD
        );

		// Test whether the stencil input mask applies when writing the stencil value or not.
		// Hyphothesis:     The stencil input mask only applies to the comparison function and not when writing to the buffer.
		// Result if true:  The red square is drawn everywhere except on the pixels where the green square was already drawn.
		// Result if false: The red square is drawn on top of everything.
		
		// Enable the stencil test, use 15 = 0b1111 as reference value.
		// Set the input mask to test only the 3 least significant bits.
		GPU_SetStencilTest(true, GPU_ALWAYS, 15, 0x7, 0xFF);
		
		// Write the reference value to the stencil buffer where the green square is drawn
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, 2); // GPU_REPLACE
		
		// Draw the green square
        DrawSmallSquare();

		// Prepare the stencil test to not allow drawing where the stencil value is 15.
		// If the hypothesis is correct, the stencil buffer will have a value of 15, and not 3.
		// Disable writing to the stencil buffer.
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 15, 0xFF, 0);
		
		// Draw the red square
		DrawBigSquare();
		
        gpuEndFrame();
    } while(aptMainLoop());

exit:
    if(test_data1)
    {
        linearFree(test_data1);
    }
	if(test_data2)
    {
        linearFree(test_data2);
    }
    if(test_texture)
    {
        linearFree(test_texture);
    }

    gpuUIExit();


    gfxExit();
    sdmcExit();
    hidExit();
    aptExit();
    srvExit();

    return 0;
}
