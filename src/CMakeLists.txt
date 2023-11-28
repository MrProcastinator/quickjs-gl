cmake_minimum_required(VERSION 3.0)

set(SOURCES
    bindings.cpp
    procs.cpp
    webgl.cpp
)

if(VITA)
    set(PLATFORM_SOURCES
        "platform/vita/egl.c"
    )
else()
    set(PLATFORM_SOURCES "")
endif()

set(JS_SOURCES
    "quickjs/gl/webgl-program.js"
    "quickjs/gl/webgl-framebuffer.js"
    "quickjs/gl/webgl-vertex-attribute.js"
    "quickjs/gl/bit-twiddle.js"
    "quickjs/gl/webgl-texture.js"
    "quickjs/gl/quickjs-index.js"
    "quickjs/gl/webgl-context-attributes.js"
    "quickjs/gl/native-gl.js"
    "quickjs/gl/linkable.js"
    "quickjs/gl/webgl-shader.js"
    "quickjs/gl/webgl-buffer.js"
    "quickjs/gl/webgl-texture-unit.js"
    "quickjs/gl/glsl-tokenizer/lib/operators.js"
    "quickjs/gl/glsl-tokenizer/lib/literals.js"
    "quickjs/gl/glsl-tokenizer/lib/literals-300es.js"
    "quickjs/gl/glsl-tokenizer/lib/builtins.js"
    "quickjs/gl/glsl-tokenizer/lib/builtins-300es.js"
    "quickjs/gl/glsl-tokenizer/string.js"
    "quickjs/gl/glsl-tokenizer/index.js"
    "quickjs/gl/webgl-active-info.js"
    "quickjs/gl/webgl-shader-precision-format.js"
    "quickjs/gl/webgl-uniform-location.js"
    "quickjs/gl/utils.js"
    "quickjs/gl/webgl-rendering-context.js"
    "quickjs/gl/extensions/ext-texture-filter-anisotropic.js"
    "quickjs/gl/extensions/oes-vertex-array-object.js"
    "quickjs/gl/extensions/oes-element-index-unit.js"
    "quickjs/gl/extensions/oes-texture-float-linear.js"
    "quickjs/gl/extensions/stackgl-destroy-context.js"
    "quickjs/gl/extensions/oes-standard-derivatives.js"
    "quickjs/gl/extensions/webgl-draw-buffers.js"
    "quickjs/gl/extensions/stackgl-resize-drawing-buffer.js"
    "quickjs/gl/extensions/angle-instanced-arrays.js"
    "quickjs/gl/extensions/ext-blend-minmax.js"
    "quickjs/gl/extensions/oes-texture-float.js"
    "quickjs/gl/webgl-drawing-buffer-wrapper.js"
    "quickjs/gl/webgl-renderbuffer.js"
    "quickjs/gl.js"
)

set(JS_IMPORTS
    "quickjs/gl/bindings.js"
)

set(JS_TARGET
    quickjs-gl-bc.c
)

set(JS_STATIC_TARGET
    quickjs-gl
)

add_quickjs_library(
    ${JS_TARGET}
    SOURCES ${JS_SOURCES}
    IMPORTS ${JS_IMPORTS}
    STATIC ${JS_STATIC_TARGET}
    MODULE
)

add_library(quickjs-gl-bindings STATIC
  ${SOURCES}
  ${PLATFORM_SOURCES}
)

add_library(quickjs-gl STATIC
  ${JS_TARGET}
)

install(TARGETS quickjs-gl DESTINATION "lib/quickjs")
install(TARGETS quickjs-gl-bindings DESTINATION "lib/quickjs") 