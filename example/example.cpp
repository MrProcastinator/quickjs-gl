#include <quickjs/quickjspp.hpp>
#include <quickjs/quickjs-libc.h>

#include <iostream>
#include <filesystem>

#include "example.h"

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
  create_native_display(EXAMPLE_WIDTH, EXAMPLE_HEIGHT);

  qjs::Runtime runtime;
  JSRuntime *rt = runtime.rt;

  js_std_set_worker_new_context_func(JS_NewCustomContext);
  js_std_init_handlers(rt);
  JS_SetModuleLoaderFunc(rt, NULL, js_module_loader, NULL);

  qjs::Context context(JS_NewCustomContext(rt));
  JSContext *ctx = context.ctx;
  js_init_module_std(ctx, "std");
  js_init_module_os(ctx, "os");
  js_std_add_helpers(ctx, argc - 1, argv + 1);
  {
    js_init_module_qjsc_quickjs_gl(ctx, "quickjs/gl.js");
  }

  int64_t window_value = (int64_t)get_native_window();

  {
    auto globalThis = context.global();
    globalThis["windowPtr"] = window_value;  
    globalThis["nativeLoop"] = []() {
      handle_loop_native_display();
    };
    globalThis["width"] = EXAMPLE_WIDTH;
    globalThis["height"] = EXAMPLE_HEIGHT;  
    globalThis["platformName"] = EXAMPLE_PLATFORM;  
  }
  
  try {
    context.evalFile(EVAL_FILE, JS_EVAL_TYPE_MODULE);
  } catch (const std::exception& e) {
    std::cerr << "File evaluation failed: " << e.what() << std::endl;
  } catch (const qjs::exception& e) {
    auto exc = context.getException();
    std::cerr << (exc.isError() ? "Error: " : "Throw: ") << (std::string)exc << std::endl;
    if((bool)exc["stack"])
        std::cerr << (std::string)exc["stack"] << std::endl;
  }

  destroy_native_display();

  return 0;
}
