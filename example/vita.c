#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <vitasdk.h>

void createNativeDisplay(int width, int height) {
    /* Do nothing, let quickjs-gl do it for us */
}

void destroyNativeDisplay() {
    /* Do nothing, let quickjs-gl do it for us */
}

void* getNativeWindow() {
    /* This forces current vita EGL implementation */
    return NULL;
}

void handleNativeEventLoop() {
    SceCtrlData pad;

    while (1) {
        sceCtrlPeekBufferPositive(0, &pad, 1);

        if (pad.buttons != 0) {
            break;
        }

        sceKernelDelayThread(10000);
    }
}