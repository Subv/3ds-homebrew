#include <3ds.h>
#include <citro3d.h>
#include <string.h>
#include <stdio.h>
#include "vshader_shbin.h"
#include "vshader_2_shbin.h"

#define CLEAR_COLOR 0x68B0D8FF

#define DISPLAY_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

typedef struct { float x, y, z; } vertex;

static const vertex vertex_list[] =
{
    { 200.0f, 200.0f, 0.5f },
    { 100.0f, 40.0f, 0.5f },
    { 300.0f, 40.0f, 0.5f },
};

#define vertex_list_count (sizeof(vertex_list)/sizeof(vertex_list[0]))

static DVLB_s* vshader_dvlb1;
static shaderProgram_s program1;
static int uLoc_projection1;
static C3D_Mtx projection1;

static void* vbo_data1;

static void sceneInit(void)
{
    // Load the vertex shader, create a shader program and bind it
    vshader_dvlb1 = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program1);
    shaderProgramSetVsh(&program1, &vshader_dvlb1->DVLE[0]);
    C3D_BindProgram(&program1);

    // Get the location of the uniforms
    uLoc_projection1 = shaderInstanceGetUniformLocation(program1.vertexShader, "projection");

    // Configure attributes for use with the vertex shader
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddFixed(attrInfo, 1); // v1=color

    // Set the fixed attribute (color) to solid red
    C3D_FixedAttribSet(1, 1.0, 0.0, 0.0, 1.0);

    // Compute the projection matrix
    Mtx_OrthoTilt(&projection1, 0.0, 400.0, 0.0, 240.0, 0.0, 1.0, true);

    // Create the VBO (vertex buffer object)
    vbo_data1 = linearAlloc(sizeof(vertex_list));
    memcpy(vbo_data1, vertex_list, sizeof(vertex_list));

    // Configure buffers
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, vbo_data1, sizeof(vertex), 1, 0x0);

    // Configure the first fragment shading substage to just pass through the vertex color
    // See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, 0, 0);
    C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
}

static void sceneRender1(void)
{
    // Update the uniforms
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection1, &projection1);

    // Draw the VBO
    C3D_DrawArrays(GPU_TRIANGLES, 0, vertex_list_count);
}

static DVLB_s* vshader_dvlb2;
static shaderProgram_s program2;
static int uLoc_projection2;
static C3D_Mtx projection2;

static void* vbo_data2;

static void sceneInit2(void)
{
    // Load the vertex shader, create a shader program and bind it
    vshader_dvlb2 = DVLB_ParseFile((u32*)vshader_2_shbin, vshader_2_shbin_size);
    shaderProgramInit(&program2);
    shaderProgramSetVsh(&program2, &vshader_dvlb2->DVLE[0]);
    C3D_BindProgram(&program2);

    // Get the location of the uniforms
    uLoc_projection2 = shaderInstanceGetUniformLocation(program2.vertexShader, "projection");

    // Configure attributes for use with the vertex shader
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddFixed(attrInfo, 1); // v1=color

    // Set the fixed attribute (color) to solid red
    C3D_FixedAttribSet(1, 1.0, 0.0, 0.0, 1.0);

    // Compute the projection matrix
    Mtx_OrthoTilt(&projection2, 0.0, 400.0, 0.0, 240.0, 0.0, 1.0, true);

    // Create the VBO (vertex buffer object)
    vbo_data2 = linearAlloc(sizeof(vertex_list));
    memcpy(vbo_data2, vertex_list, sizeof(vertex_list));

    // Configure buffers
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, vbo_data2, sizeof(vertex), 1, 0x0);

    // Configure the first fragment shading substage to just pass through the vertex color
    // See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, 0, 0);
    C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
}

static void sceneRender2(void)
{
    // Update the uniforms
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection2, &projection2);

    // Draw the VBO
    C3D_DrawArrays(GPU_TRIANGLES, 0, vertex_list_count);
}

static void sceneExit(void)
{
    // Free the VBO
    linearFree(vbo_data1);
    linearFree(vbo_data2);

    // Free the shader program
    shaderProgramFree(&program1);
    DVLB_Free(vshader_dvlb1);
    shaderProgramFree(&program2);
    DVLB_Free(vshader_dvlb2);
}

int main()
{
    // Initialize graphics
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    consoleInit(GFX_BOTTOM, NULL);

    // Initialize the render target
    C3D_RenderTarget* target = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetClear(target, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
    C3D_RenderTargetSetOutput(target, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

    // Initialize the scene
    sceneInit();

    // Main loop
    while (aptMainLoop())
    {
        hidScanInput();

        // Respond to user input
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
            break; // break in order to return to hbmenu

        consoleClear();
        printf("Press START to go to the next draw");

        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            C3D_FrameDrawOn(target);
            sceneRender1();
        C3D_FrameEnd(0);
    }

    sceneInit2();

    while (aptMainLoop())
    {
        hidScanInput();

        // Respond to user input
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
            break; // break in order to return to hbmenu

        consoleClear();
        printf("Press START to exit");

        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            C3D_FrameDrawOn(target);
            sceneRender2();
        C3D_FrameEnd(0);
    }

    // Deinitialize the scene
    sceneExit();

    // Deinitialize graphics
    C3D_Fini();
    gfxExit();
    return 0;
}