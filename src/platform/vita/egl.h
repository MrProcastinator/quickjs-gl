#ifndef VITA_EGL_H_
#define VITA_EGL_H_

#include <inttypes.h>
#include <stdbool.h>

#include <vitasdk.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define EGL_VITA_MULTISAMPLE_MODE   0x4000
#define EGL_VITA_VMEM   0x4001
#define EGL_VITA_LEGACY_POOL   0x4002

typedef struct VitaEGLDisplay {
    bool active;
} VitaEGLDisplay;

typedef struct VitaEGLSurface {
    bool active;
} VitaEGLSurface;

typedef struct VitaEGLNativeWindowType {
    bool native;
} VitaEGLNativeWindowType;

typedef struct VitaEGLContext {
    bool active;
    EGLint client_version;
} VitaEGLContext;

typedef struct VitaEGLConfig {
    EGLint red;
    EGLint green;
    EGLint blue;
    EGLint alpha;
    EGLint depth;
    EGLint stencil;
} VitaEGLConfig;

typedef struct VitaEGLSurfaceAttributes {
    EGLint width;
    EGLint height;
    SceGxmMultisampleMode multisample_mode;
    int vmem;
    int legacy_pool;
} VitaEGLSurfaceAttributes;

#endif /* VITA_EGL_H_ */