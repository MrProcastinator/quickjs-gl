#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <EGL/egl.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    Window window;
    Display* display;
} NativeDisplay;

static NativeDisplay currentDisplay;

// Function to create an X11 window
static Window createX11Window(Display *display, int width, int height) {
    Window root = DefaultRootWindow(display);
    XVisualInfo *visualInfo;
    XSetWindowAttributes windowAttributes;

    static int attributeList[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        None
    };

    visualInfo = glXChooseVisual(display, 0, attributeList);
    if (visualInfo == NULL) {
        fprintf(stderr, "Error: Couldn't get an RGB, Double-buffered visual.\n");
        exit(1);
    }

    windowAttributes.colormap = XCreateColormap(display, root, visualInfo->visual, AllocNone);
    windowAttributes.background_pixmap = None;
    windowAttributes.border_pixel = 0;
    windowAttributes.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;

    Window window = XCreateWindow(
        display, root,
        0, 0, width, height,
        0, visualInfo->depth, InputOutput,
        visualInfo->visual,
        CWBorderPixel | CWColormap | CWEventMask,
        &windowAttributes
    );

    XMapWindow(display, window);
    XStoreName(display, window, "OpenGL Window");

    return window;
}

static void destroyX11Window(Display* display, Window window) {
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

static Display* createDisplay() {
    // Open a connection to the X server
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Error: Could not open X display\n");
        exit(1);
    }
    return display;
}

void createNativeDisplay(int width, int height) {
    Display* display = createX11Display();
    Window window = createX11Window(display, width, height);

    currentDisplay.window = window;
    currentDisplay.display = display;
}

void destroyNativeDisplay() {
    destroyX11Window(currentDisplay.display, currentDisplay.window);
}

void* getNativeWindow() {
    return &currentDisplay.window;
}

void handleNativeEventLoop() {

}