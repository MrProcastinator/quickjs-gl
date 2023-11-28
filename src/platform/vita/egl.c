#include "egl.h"

#include <stdio.h>

GLboolean vglInitExtended(int legacy_pool_size, int width, int height, int ram_threshold, SceGxmMultisampleMode msaa);
void vglEnd(void);

static FILE* log_file;

// Only one display at a time
static VitaEGLDisplay current_display;
static VitaEGLConfig current_config;
static VitaEGLContext current_context;
static VitaEGLSurfaceAttributes current_surface_attributes = {
    .width = 960,
    .height = 544,
    .multisample_mode = SCE_GXM_MULTISAMPLE_NONE,
    .vmem = 0x1800000,
    .legacy_pool = 0
};
static const EGLint vita_num_configs = 1;

static SceGxmMultisampleMode eglVitaMultisamplingModeFor(EGLint value) {
    switch(value) {
        case 0:
            return SCE_GXM_MULTISAMPLE_NONE;
        case 2:
            return SCE_GXM_MULTISAMPLE_2X;
        case 4:
            return SCE_GXM_MULTISAMPLE_4X;
    }
    return SCE_GXM_MULTISAMPLE_NONE;
}

static
EGLAPI 
EGLSurface eglCreateVitaSurface (EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
    if(current_display.active && current_context.active) {
        while(*attrib_list != EGL_NONE) {
            EGLint attrib_value = *(attrib_list + 1);
            switch(attrib_value) {
                case EGL_WIDTH:
                    current_surface_attributes.width = attrib_value;
                    break;
                case EGL_HEIGHT:
                    current_surface_attributes.height = attrib_value;
                    break;
                case EGL_VITA_MULTISAMPLE_MODE:
                    current_surface_attributes.multisample_mode = eglVitaMultisamplingModeFor(attrib_value);
                    break;
                case EGL_VITA_VMEM:
                    current_surface_attributes.vmem = eglVitaMultisamplingModeFor(attrib_value);
                    break;
                case EGL_VITA_LEGACY_POOL:
                    current_surface_attributes.legacy_pool = eglVitaMultisamplingModeFor(attrib_value);
                    break;
            }
            attrib_list += 2;
        }
        fprintf(log_file, "vglInitExtended(%d, %d, %d, %d, %d)\n", current_surface_attributes.legacy_pool, current_surface_attributes.width, current_surface_attributes.height, current_surface_attributes.vmem, current_surface_attributes.multisample_mode);
        GLboolean result = vglInitExtended(current_surface_attributes.legacy_pool, current_surface_attributes.width, current_surface_attributes.height, current_surface_attributes.vmem, current_surface_attributes.multisample_mode);
        fprintf(log_file, "vglInitExtended result: %d\n", result);
        return (EGLSurface)EGL_TRUE;
    } 
    return (EGLSurface)EGL_FALSE;
}

/* EGL bindings for vitaGL, may not be 100% compliant just for use with this module */
EGLAPI 
EGLBoolean eglInitialize (EGLDisplay dpy, EGLint *major, EGLint *minor) {
    current_display.active = true;
    log_file = fopen("ux0:data/quickjs-gl/egl.txt", "w");
    return EGL_TRUE;
}

EGLAPI 
EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
    if(current_display.active) {
        while(*attrib_list != EGL_NONE) {
            EGLint attrib_value = *(attrib_list + 1);
            switch(attrib_value) {
                case EGL_RED_SIZE:
                    current_config.red = attrib_value;
                    break;
                case EGL_GREEN_SIZE:
                    current_config.green = attrib_value;
                    break;
                case EGL_BLUE_SIZE:
                    current_config.blue = attrib_value;
                    break;
                case EGL_ALPHA_SIZE:
                    current_config.alpha = attrib_value;
                    break;
                case EGL_DEPTH_SIZE:
                    current_config.depth = attrib_value;
                    break;
                case EGL_STENCIL_SIZE:
                    current_config.stencil = attrib_value;
                    break;
            }
            attrib_list += 2;
        }
        *((VitaEGLConfig*)configs) = current_config;
        *num_config = vita_num_configs;
        return EGL_TRUE;
    } 
    return EGL_FALSE;
}

EGLAPI 
EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) {
    while(*attrib_list != EGL_NONE) {
        EGLint attrib_value = *(attrib_list + 1);
        switch(attrib_value) {
            case EGL_CONTEXT_CLIENT_VERSION:
                current_context.client_version = attrib_value;
                break;
        }
        attrib_list += 2;
    }
    current_context.active = true;
    return &current_context;
}

EGLAPI 
EGLSurface eglCreatePbufferSurface (EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
    return eglCreateVitaSurface(dpy, config, attrib_list);
}

EGLAPI 
EGLSurface eglCreateWindowSurface (EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list) {
    /* Assume it was all configured by SDL or other external library */
    return (EGLSurface)EGL_TRUE;
}

EGLAPI 
EGLBoolean eglDestroyContext (EGLDisplay dpy, EGLContext ctx) {
    return EGL_TRUE;
}

EGLAPI 
EGLBoolean eglDestroySurface (EGLDisplay dpy, EGLSurface surface) {
    // Do nothing
    return EGL_TRUE;
}

EGLAPI 
EGLBoolean eglMakeCurrent (EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    return EGL_TRUE;
}

EGLAPI 
EGLBoolean eglTerminate (EGLDisplay dpy) {
    current_display.active = false;
    vglEnd();
    fclose(log_file);
    return EGL_TRUE;
}