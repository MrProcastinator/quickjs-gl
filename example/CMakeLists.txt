cmake_minimum_required(VERSION 3.0)

# Some platforms requiere absolute paths
function(eval_path file)
    if(VITA)
        set(prefix "ux0:data/quickjs-gl/example/")
    else()
        set(prefix "./")
    endif()

    set(${file} "${prefix}${${file}}" PARENT_SCOPE)
endfunction()

set(EXAMPLE_SOURCES 
    example.cpp
)

set(CORE_LIBRARIES pthread stdc++fs c m dl)
set(OTHER_LIBRARIES "")
set(NATIVE_LIBRARIES "")

if(USE_SDL)
    set(NATIVE_SOURCES "sdl.c")
    set(NATIVE_LIBRARIES EGL SDL2)
else()
    if(VITA)
        set(NATIVE_SOURCES "vita.c")
        set(NATIVE_LIBRARIES ${VITA_GL_LIB}) 
        set(CORE_LIBRARIES c m dl -Wl,--whole-archive ${VITA_PTHREAD_LIB} -Wl,--no-whole-archive) 
        set(OTHER_LIBRARIES mathneon ${VITA_VITASHARK_LIB} SceShaccCgExt taihen_stub SceShaccCg_stub SceKernelDmacMgr_stub SceCommonDialog_stub SceAppMgr_stub SceGxm_stub SceCtrl_stub SceSblSsMgr_stub SceLibKernel_stub SceDisplay_stub)
    endif()
endif()

if(VITA)
    set(EXAMPLE_WIDTH 960)
    set(EXAMPLE_HEIGHT 544)
    set(EXAMPLE_PLATFORM vita)
else()
    set(EXAMPLE_WIDTH 640)
    set(EXAMPLE_HEIGHT 480)
    set(EXAMPLE_PLATFORM linux)
endif()

link_directories("${BUILD_PREFIX}/lib/quickjs")

link_libraries(quickjs-gl quickjs-gl-bindings quickjs ${NATIVE_LIBRARIES} ${CORE_LIBRARIES} ${OTHER_LIBRARIES})

set(EVAL_FILE_1 "triangles/triangles.js")
eval_path(EVAL_FILE_1)

add_executable(quickjs-gl_01_triangles
    ${EXAMPLE_SOURCES}
    ${NATIVE_SOURCES}
)

target_compile_definitions(quickjs-gl_01_triangles PUBLIC 
    EVAL_FILE="${EVAL_FILE_1}"
    EXAMPLE_WIDTH=${EXAMPLE_WIDTH}
    EXAMPLE_HEIGHT=${EXAMPLE_HEIGHT}
    EXAMPLE_PLATFORM="${EXAMPLE_PLATFORM}"
)

set(EVAL_FILE_2 "fractals/mandelbrot.js")
eval_path(EVAL_FILE_2)

add_executable(quickjs-gl_02_fractals
    ${EXAMPLE_SOURCES}
    ${NATIVE_SOURCES}
)

target_compile_definitions(quickjs-gl_02_fractals PUBLIC 
    EVAL_FILE="${EVAL_FILE_2}"
    EXAMPLE_WIDTH=${EXAMPLE_WIDTH}
    EXAMPLE_HEIGHT=${EXAMPLE_HEIGHT}
    EXAMPLE_PLATFORM="${EXAMPLE_PLATFORM}"
)

## Create vpk for each example
if(VITA)
    set(VITA_VERSION  "01.00")

    set(VITA_MKSFOEX_FLAGS "")
    set(VITA_PACK_VPK_FLAGS "")

    vita_create_self(quickjs-gl_01_triangles.self quickjs-gl_01_triangles)
    vita_create_vpk(quickjs-gl_01_triangles.vpk QJSGL0001 quickjs-gl_01_triangles.self
    VERSION ${VITA_VERSION}
    NAME "QuickJS GL Triangles"
    FILE ../vita/sce_sys/icon0.png sce_sys/icon0.png
    FILE ../vita/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
    FILE ../vita/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
    FILE ../vita/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
    )

    # Hack to avoid duplicate title id
    set(VITA_MKSFOEX_FLAGS "")
    set(VITA_PACK_VPK_FLAGS "")

    vita_create_self(quickjs-gl_02_fractals.self quickjs-gl_02_fractals)
    vita_create_vpk(quickjs-gl_02_fractals.vpk QJSGL0002 quickjs-gl_02_fractals.self
    VERSION ${VITA_VERSION}
    NAME "QuickJS GL Fractals"
    FILE ../vita/sce_sys/icon0.png sce_sys/icon0.png
    FILE ../vita/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
    FILE ../vita/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
    FILE ../vita/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
    )
endif()