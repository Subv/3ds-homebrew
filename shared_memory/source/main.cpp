#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <3ds.h>

u32 SHARED_MEM_ADDRESS_1 = 0;
u32 SHARED_MEM_ADDRESS_2 = 0;
const u32 SHARED_MEM_ADDRESS_3 = 0x10000000;
const u32 SHARED_MEM_SIZE = 0x1000;
const MemPerm SHARED_MEM_PERMISSIONS = (MemPerm)(MEMPERM_READ | MEMPERM_WRITE);

Result GetSharedFont(Handle& mem_handle, u32& addr) {
    Handle apt_handle;
    srvGetServiceHandle(&apt_handle, "APT:U");
    u32* cmdbuf = getThreadCommandBuffer();

    // Call APT::GetSharedFont function to load font into memory...

    cmdbuf[0] = 0x00440000;
    Result res = svcSendSyncRequest(apt_handle);

    mem_handle = cmdbuf[4];
    addr = cmdbuf[2];

    // Close APT handle...

    svcCloseHandle(apt_handle);

    return res;
}

int main() {
    gfxInitDefault();
    aptInit();
    consoleInit(GFX_TOP, NULL);

    printf("Starting tests\n\n");

    SHARED_MEM_ADDRESS_1 = (u32)memalign(0x1000, SHARED_MEM_SIZE);
    SHARED_MEM_ADDRESS_2 = (u32)memalign(0x1000, SHARED_MEM_SIZE);

    printf("Address: %08X\n\n", SHARED_MEM_ADDRESS_1);

    Handle mem_block;
    Handle shared_mem_block;
    u32 shared_mem_addr;
    Result res = GetSharedFont(shared_mem_block, shared_mem_addr);

    if (res != 0) {
        printf("Unable to get shared mem memblock %08X\n\n", (u32)res);
        goto loop;
    }

    res = svcCreateMemoryBlock(&mem_block, SHARED_MEM_ADDRESS_1, SHARED_MEM_SIZE, SHARED_MEM_PERMISSIONS, SHARED_MEM_PERMISSIONS);

    if (res != 0) {
        printf("Unable to create memblock %08X\n\n", (u32)res);
        goto loop;
    }

    printf("Unmapping\n\n");

    res = svcUnmapMemoryBlock(mem_block, SHARED_MEM_ADDRESS_1);

    if (res != 0) {
        printf("Unable to unmap memblock %08X\n\n", (u32)res);
        goto loop;
    }

    printf("Unmapped\n\n");

    res = svcMapMemoryBlock(shared_mem_block, 0, MEMPERM_READ, MEMPERM_DONTCARE);

    if (res != 0) {
        printf("Unable to map memblock %08X\n\n", (u32)res);
        goto loop;
    }

    printf("Mapped\n\n");

    res = svcUnmapMemoryBlock(shared_mem_block, shared_mem_addr);

    if (res != 0) {
        printf("Unable to unmap memblock 2 %08X\n\n", (u32)res);
        goto loop;
    }

    printf("Finished\n\n");

loop:
    while (aptMainLoop())
    {
        gspWaitForVBlank();
        hidScanInput();
        auto keys = hidKeysHeld();

        if (keys & KEY_START)
            break;

        gfxFlushBuffers();
        gfxSwapBuffers();
    }

    svcCloseHandle(mem_block);
    svcCloseHandle(shared_mem_block);
    gfxExit();
    return 0;
}