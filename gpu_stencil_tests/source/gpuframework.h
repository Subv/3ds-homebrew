/**
 *@file gpuframework.h
 *@author Lectem
 *@date 17/05/2015
 */
#pragma once


#include <3ds.h>
#include <stdio.h>



/**
* Crappy assert stuff that lets you go back to hbmenu by pressing start.
*/
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define my_assert(e) ((e) ? (void)0 : _my_assert("assert failed at " __FILE__ ":" LINE_STRING " (" #e ")\n"))
void _my_assert(char * text);

#define RGBA8(r,g,b,a) (((r)&0xFF) | (((g)&0xFF)<<8) | (((b)&0xFF)<<16) | (((a)&0xFF)<<24))

u32*gpuColorBuffer;
u32*gpuDBuffer;
u32* gpuCmd;

typedef struct {
    float x, y;
} vector_2f;

typedef struct {
    float x, y, z;
} vector_3f;

typedef struct {
    float r, g, b, a;
} vector_4f;

typedef struct {
    u8 r, g, b, a;
} vector_4u8;

typedef struct __attribute__((__packed__)){
    vector_3f position;
    vector_4u8 color;
    vector_2f texpos;
} vertex_pos_col;


void gpuUIInit();
void gpuUIExit();
void gpuStartFrame();
void gpuEndFrame();
void GPU_SetDummyTexEnv(u8 num);
void copyTextureAndTile(u8* dst,u8 * src,int w ,int h);
