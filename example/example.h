#ifndef EXAMPLE_H
#define EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

void createNativeDisplay(int width, int height);
void destroyNativeDisplay();
void* getNativeWindow();
void handleNativeEventLoop();

#ifdef __cplusplus
}
#endif

#define create_native_display createNativeDisplay
#define destroy_native_display destroyNativeDisplay
#define get_native_window getNativeWindow
#define handle_loop_native_display handleNativeEventLoop

#endif // EXAMPLE_H