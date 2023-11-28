# `quickjs-gl` - QuickJS implementation of WebGL

This is a WebGL wrapper for QuickJS, conforming to WebGL1 specification, based on the node implementation in [`@kmamal/sdl`](https://github.com/kmamal/node-sdl#readme).

It should work on Linux, Mac, Windows and Playstation Vita.

## Compiling for Linux

To compile for Linux, you need cmake and the following dependencies installed:
```
egl sdl2 g++ gcc quickjs quickjspp
```

In order to compile this project you need the following quickjs port [here](https://github.com/MrProcastinator/quickjs-vita)

For example:
```bash
mkdir build
cd build
cmake -DVITA=1 ..
make
sudo make install
```

## Compiling for PSVita

To compile for Playstation Vita using vitasdk, add the VITA flag to the cmake invocation with the value 1.

For example:
```bash
mkdir build
cd build
cmake -DVITA=1 ..
make
sudo make install
```
After compiling, you'll also get a .vpk file for each example in the build/example directory, to test it directly on the device.

For dependency switching purposes, the following optional CMake flags are provided:

| **Flag**        | **Description**
|:-------------------|:------------------------------------------
|VITA_GL_LIB         | Name of the (vita)GL dependency library (default: vitaGL)
|VITA_VITASHARK_LIB | Name of the vitashark dependency library (default: vitashark)
|VITA_PTHREAD_LIB | Name of the pthread dependency library (default: pthread)

## Example

For Linux

```js
import createContext from 'quickjs/gl.js'

// Clear screen to red
const gl = createContext(width, height, { platform: { name: "linux" } })
gl.clearColor(1, 0, 0, 1)
gl.clear(gl.COLOR_BUFFER_BIT)
gl.swap()
gl.destroy()
```

For Playstation Vita

```js
import createContext from 'quickjs/gl.js'

// Clear screen to red
// msaa possible values: 0, 2, 4
const gl = createContext(width, height, { platform: { name: "vita", msaa: 0 } })
gl.clearColor(1, 0, 0, 1)
gl.clear(gl.COLOR_BUFFER_BIT)
gl.swap()
gl.destroy()
```

# How to use with QuickJS

1. Write your WebGL script (e.g.: webgl.js):
```js
import createContext from 'quickjs/gl.js'

// Clear screen to red
// msaa possible values: 0, 2, 4
const gl = createContext(width, height, { platform: { name: "linux" } })
gl.clearColor(1, 0, 0, 1)
gl.clear(gl.COLOR_BUFFER_BIT)
gl.swap()
gl.destroy()
```

2. Create a C++ wrapper either manually or using qjsc:
```bash
qjsc -e -o main.cpp -M quickjs/gl.js -m webgl.js
```

3. Edit the C++ wrapper to use [quickjspp](https://github.com/ftk/quickjspp) context and runtime classes (this is needed to avoid memory leaks in the QuickJS binding code):
``` C
#include <quickjs/quickjs-libc.h>
#include <quickjs/quickjspp.hpp>

/** Your bytecode here **/

/* We need to declare this function in order to initialize the library */
extern "C" {
extern JSModuleDef *js_init_module_qjsc_quickjs_gl(JSContext* context, const char* name);
}

static JSContext *JS_NewCustomContext(JSRuntime *rt)
{
  JSContext *ctx = JS_NewContextRaw(rt);
  if (!ctx) { 
    std::cout << "JS_NewContextRaw(rt): failed";
    return NULL;
  }
  JS_AddIntrinsicBaseObjects(ctx);
  JS_AddIntrinsicDate(ctx);
  JS_AddIntrinsicEval(ctx);
  JS_AddIntrinsicStringNormalize(ctx);
  JS_AddIntrinsicRegExp(ctx);
  JS_AddIntrinsicJSON(ctx);
  JS_AddIntrinsicProxy(ctx);
  JS_AddIntrinsicMapSet(ctx);
  JS_AddIntrinsicTypedArrays(ctx);
  JS_AddIntrinsicPromise(ctx);
  JS_AddIntrinsicBigInt(ctx);
  return ctx;
}

int main(int argc, char **argv)
{
  
  qjs::Runtime runtime;
  JSRuntime *rt = runtime.rt;

  js_std_set_worker_new_context_func(JS_NewCustomContext);
  js_std_init_handlers(rt);
  JS_SetModuleLoaderFunc(rt, NULL, js_module_loader, NULL);

  qjs::Context context(JS_NewCustomContext(rt));
  JSContext *ctx = context.ctx;
  // Optional if you-re using this library
  js_init_module_std(ctx, "std");
  // Optional if you-re using this library
  js_init_module_os(ctx, "os");
  js_std_add_helpers(ctx, argc - 1, argv + 1);
  {
    js_init_module_qjsc_quickjs_gl(ctx, "quickjs/gl.js");
  }

  js_std_eval_binary(ctx, qjsc_webgl, qjsc_webgl_size, JS_EVAL_TYPE_GLOBAL);
  js_std_loop(ctx);

  return 0;
}
```

4. Compile with quickjs-gl-bindings and quickjs-gl as dependencies (in that order):
```bash
g++ -o webgl main.cpp -L/usr/share/lib/quickjs -lquickjs-gl-bindings -lquickjs -lEGL -lSDL2 -lpthread -lm -ldl
```
