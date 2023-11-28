#include <cstring>
#include <vector>
#include <iostream>
#include <exception>

#include "webgl.h"

#include <quickjs/quickjspp.hpp>
#include <quickjs/quickjspp-arraybuffer.hpp>

bool                   WebGLRenderingContext::HAS_DISPLAY = false;
EGLDisplay             WebGLRenderingContext::DISPLAY;
WebGLRenderingContext* WebGLRenderingContext::ACTIVE = NULL;
WebGLRenderingContext* WebGLRenderingContext::CONTEXT_LIST_HEAD = NULL;

const char* REQUIRED_EXTENSIONS[] = {
  "GL_OES_packed_depth_stencil",
  //TODO: add when angle is ported
  //"GL_ANGLE_instanced_arrays",
  NULL
};

WebGLRenderingContext::WebGLRenderingContext(
    int32_t width,
    int32_t height,
    bool alpha,
    bool depth,
    bool stencil,
    bool antialias,
    bool premultipliedAlpha,
    bool preserveDrawingBuffer,
    bool preferLowPowerToHighPerformance,
    bool failIfMajorPerformanceCaveat,
    std::optional<intptr_t> uWindow,
    qjs::Value platformOptions)
    : state(GLCONTEXT_STATE_INIT),
      unpack_flip_y(false),
      unpack_premultiply_alpha(false),
      unpack_colorspace_conversion(0x9244),
      unpack_alignment(4),
      next(nullptr),
      prev(nullptr),
      lastError(GL_NO_ERROR) {
  EGLNativeWindowType* window = 
    uWindow.has_value()
      ? reinterpret_cast<EGLNativeWindowType*>(uWindow.value())
      : nullptr;
  // Get display
  if (!HAS_DISPLAY) {
    DISPLAY = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (DISPLAY == EGL_NO_DISPLAY) {
      throw std::runtime_error("Error creating WebGLContext: no display found");
    }

    // Initialize EGL
    if (!eglInitialize(DISPLAY, nullptr, nullptr)) {
      throw std::runtime_error("Error creating WebGLContext: couldn't initialize EGL");
    }

    // Save display
    HAS_DISPLAY = true;
  }

  //Set up configuration
  EGLint attrib_list[] = {
      EGL_SURFACE_TYPE, EGL_PBUFFER_BIT
    , EGL_RED_SIZE,     8
    , EGL_GREEN_SIZE,   8
    , EGL_BLUE_SIZE,    8
    , EGL_ALPHA_SIZE,   8
    , EGL_DEPTH_SIZE,   24
    , EGL_STENCIL_SIZE, 8
    , EGL_NONE
  };
  EGLint num_config;
  if (!eglChooseConfig(
      DISPLAY,
      attrib_list,
      &config,
      1,
      &num_config) ||
      num_config != 1) {
      throw std::runtime_error("Error creating WebGLContext: couldn't choose config");
  }

   //Create context
  EGLint contextAttribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };
  context = eglCreateContext(DISPLAY, config, EGL_NO_CONTEXT, contextAttribs);
  if (context == EGL_NO_CONTEXT) {
    throw std::runtime_error("Error creating WebGLContext: couldn't create context");
  }

  if (window) {
    surface = eglCreateWindowSurface(DISPLAY, config, *window, nullptr);
  } else {
  #ifdef __vita__
    std::vector<EGLint> surfaceAttribs {{
          EGL_WIDTH,  (EGLint)width
        , EGL_HEIGHT, (EGLint)height
    }};
    if(platformOptions) {
      if(platformOptions.contains("msaa")) {
        surfaceAttribs.push_back(EGL_VITA_MULTISAMPLE_MODE);
        surfaceAttribs.push_back(platformOptions["msaa"].as<EGLint>());
      }
    }
    surfaceAttribs.push_back(EGL_NONE);
  #else
    std::array<EGLint, 5> surfaceAttribs = {
          EGL_WIDTH,  (EGLint)width
        , EGL_HEIGHT, (EGLint)height
        , EGL_NONE
    };
  #endif
    surface = eglCreatePbufferSurface(DISPLAY, config, surfaceAttribs.data());
  }

  if (surface == EGL_NO_SURFACE) {
    throw std::runtime_error("Error creating WebGLContext: couldn't initialize surface");
  }

  //Set active
  if (!eglMakeCurrent(DISPLAY, surface, surface, context)) {
    throw std::runtime_error("Error creating WebGLContext: couldn't set current context surface");
  }

  //Success
  state = GLCONTEXT_STATE_OK;
  registerContext();
  ACTIVE = this;

  //Initialize function pointers
  initPointers();

  //Check extensions
  const char *extensionString = (const char*)((glGetString)(GL_EXTENSIONS));

  //Load required extensions
  for(const char** rext = REQUIRED_EXTENSIONS; *rext; ++rext) {
    if(!strstr(extensionString, *rext)) {
      dispose();
      throw std::runtime_error("Error creating WebGLContext: couldn't find WebGL extension");
    }
  }

  //Select best preferred depth
  preferredDepth = GL_DEPTH_COMPONENT16;
  if(strstr(extensionString, "GL_OES_depth32")) {
    preferredDepth = GL_DEPTH_COMPONENT32_OES;
  } else if(strstr(extensionString, "GL_OES_depth24")) {
    preferredDepth = GL_DEPTH_COMPONENT24_OES;
  }
}

bool WebGLRenderingContext::swap() {
  if (state != GLCONTEXT_STATE_OK) {
    return false;
  }
  if (!eglSwapBuffers(DISPLAY, surface)) {
    state = GLCONTEXT_STATE_ERROR;
    return false;
  }
  return true;
}

bool WebGLRenderingContext::setActive() {
  if (state != GLCONTEXT_STATE_OK) {
    return false;
  }
  if (this == ACTIVE) {
    return true;
  }
  if (!eglMakeCurrent(DISPLAY, surface, surface, context)) {
    state = GLCONTEXT_STATE_ERROR;
    return false;
  }
  ACTIVE = this;
  return true;
}

void WebGLRenderingContext::setError(GLenum error) {
  if (error == GL_NO_ERROR || lastError != GL_NO_ERROR) {
    return;
  }
  GLenum prevError = glGetError();
  if (prevError == GL_NO_ERROR) {
    lastError = error;
  }
}

void WebGLRenderingContext::dispose() {
  // Unregister context
  unregisterContext();

  if (!setActive()) {
    state = GLCONTEXT_STATE_ERROR;
    return;
  }

  // Update state
  state = GLCONTEXT_STATE_DESTROY;

  // Destroy all object references
  for (auto iter = objects.begin(); iter != objects.end(); ++iter) {
    GLuint obj = iter->first.first;

    switch (iter->first.second) {
      case GLOBJECT_TYPE_PROGRAM:
        glDeleteProgram(obj);
        break;
      case GLOBJECT_TYPE_BUFFER:
        glDeleteBuffers(1, &obj);
        break;
      case GLOBJECT_TYPE_FRAMEBUFFER:
        glDeleteFramebuffers(1, &obj);
        break;
      case GLOBJECT_TYPE_RENDERBUFFER:
        glDeleteRenderbuffers(1, &obj);
        break;
      case GLOBJECT_TYPE_SHADER:
        glDeleteShader(obj);
        break;
      case GLOBJECT_TYPE_TEXTURE:
        glDeleteTextures(1, &obj);
        break;
      case GLOBJECT_TYPE_VERTEX_ARRAY:
        glDeleteVertexArraysOES(1, &obj);
        break;
      default:
        break;
    }
  }

  // Deactivate context
  eglMakeCurrent(DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  ACTIVE = nullptr;

  // Destroy surface and context

  // FIXME: This shouldn't be commented out
  // eglDestroySurface(DISPLAY, surface);
  eglDestroyContext(DISPLAY, context);
}

void WebGLRenderingContext::SetError(GLenum error) {
  this->setError(error);
}

void WebGLRenderingContext::DisposeAll() {
  while(CONTEXT_LIST_HEAD) {
    CONTEXT_LIST_HEAD->dispose();
  }

  if(WebGLRenderingContext::HAS_DISPLAY) {
    eglTerminate(WebGLRenderingContext::DISPLAY);
    WebGLRenderingContext::HAS_DISPLAY = false;
  }
}

void WebGLRenderingContext::Destroy() {
    this->dispose();
}

void WebGLRenderingContext::Swap() {
  this->swap();
}

void WebGLRenderingContext::Uniform1f(GLint location, GLfloat x) {
  (this->glUniform1f)(location, x);
}

void WebGLRenderingContext::Uniform2f(GLint location, GLfloat x, GLfloat y) {
  (this->glUniform2f)(location, x, y);
}

void WebGLRenderingContext::Uniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z) {
  (this->glUniform3f)(location, x, y, z);
}

void WebGLRenderingContext::Uniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
  (this->glUniform4f)(location, x, y, z, w);
}

void WebGLRenderingContext::Uniform1i(GLint location, GLint x) {
  (this->glUniform1i)(location, x);
}

void WebGLRenderingContext::Uniform2i(GLint location, GLint x, GLint y) {
  (this->glUniform2i)(location, x, y);
}

void WebGLRenderingContext::Uniform3i(GLint location, GLint x, GLint y, GLint z) {
  (this->glUniform3i)(location, x, y, z);
}

void WebGLRenderingContext::Uniform4i(GLint location, GLint x, GLint y, GLint z, GLint w) {
  (this->glUniform4i)(location, x, y, z, w);
}

void WebGLRenderingContext::PixelStorei(GLenum pname, GLenum param) {
  //Handle WebGL specific extensions
  switch(pname) {
    case 0x9240:
      this->unpack_flip_y = param != 0;
    break;

    case 0x9241:
      this->unpack_premultiply_alpha = param != 0;
    break;

    case 0x9243:
      this->unpack_colorspace_conversion = param;
    break;

    case GL_UNPACK_ALIGNMENT:
      this->unpack_alignment = param;
      (this->glPixelStorei)(pname, param);
    break;

    case GL_MAX_DRAW_BUFFERS_EXT:
      (this->glPixelStorei)(pname, param);
    break;

    default:
      (this->glPixelStorei)(pname, param);
    break;
  }
}

void WebGLRenderingContext::BindAttribLocation(GLint program, GLint index, const std::string& name) {
  (this->glBindAttribLocation)(program, index, name.c_str());
}

GLenum WebGLRenderingContext::getError() {
  GLenum error = (this->glGetError)();
  if (lastError != GL_NO_ERROR) {
    error = lastError;
  }
  lastError = GL_NO_ERROR;
  return error;
}

GLenum WebGLRenderingContext::GetError() {
  return this->getError();
}

void WebGLRenderingContext::VertexAttribDivisor(GLuint index, GLuint divisor) {
#if !defined(__vita__)
  (this->glVertexAttribDivisor)(index, divisor);
#else
  // throw std::runtime_error("glVertexAttribDivisor is not implemented in this platform!");
#endif
}

void WebGLRenderingContext::DrawArraysInstanced(GLenum mode, GLint first, GLuint count, GLuint icount) {
  (this->glDrawArraysInstanced)(mode, first, count, icount);
}

void WebGLRenderingContext::DrawElementsInstanced(GLenum mode, GLint count, GLenum type, GLint offset, GLuint icount) {
  (this->glDrawElementsInstanced)(
    mode,
    count,
    type,
    reinterpret_cast<GLvoid*>(offset),
    icount);
}

void WebGLRenderingContext::DrawArrays(GLenum mode, GLint first, GLint count) {
  (this->glDrawArrays)(mode, first, count);
}

void WebGLRenderingContext::UniformMatrix2fv(GLint location, GLboolean transpose, const std::vector<GLfloat> data) {
  (this->glUniformMatrix2fv)(location, data.size() / 4, transpose, data.data());
}

void WebGLRenderingContext::UniformMatrix3fv(GLint location, GLboolean transpose, const std::vector<GLfloat> data) {
  (this->glUniformMatrix3fv)(location, data.size() / 9, transpose, data.data());
}

void WebGLRenderingContext::UniformMatrix4fv(GLint location, GLboolean transpose, const std::vector<GLfloat> data) {
  (this->glUniformMatrix4fv)(location, data.size() / 16, transpose, data.data());
}

void WebGLRenderingContext::GenerateMipmap(GLint target) {
  (this->glGenerateMipmap)(target);
}

GLint WebGLRenderingContext::GetAttribLocation(GLint program, const std::string& name) {
  return (this->glGetAttribLocation)(program, name.c_str());
}


void WebGLRenderingContext::DepthFunc(GLenum func) {
  (this->glDepthFunc)(func);
}


void WebGLRenderingContext::Viewport(GLint x, GLint y, GLsizei width, GLsizei height) {
  (this->glViewport)(x, y, width, height);
}

GLuint WebGLRenderingContext::CreateShader(GLenum shaderType) {
  GLuint shader=(this->glCreateShader)(shaderType);
  this->registerGLObj(GLOBJECT_TYPE_SHADER, shader);
  return shader;
}


void WebGLRenderingContext::ShaderSource(GLint id, const std::string& code) {
  const char* codes[] = { code.c_str() };
  GLint length = code.length();

  (this->glShaderSource)(id, 1, codes, &length);
}


void WebGLRenderingContext::CompileShader(GLuint shader) {  
  (this->glCompileShader)(shader);
}

void WebGLRenderingContext::FrontFace(GLenum mode) {
  (this->glFrontFace)(mode);
}


GLint WebGLRenderingContext::GetShaderParameter(GLint shader, GLenum pname) {
  GLint value;
  (this->glGetShaderiv)(shader, pname, &value);

  return value;
}

std::string WebGLRenderingContext::GetShaderInfoLog(GLint id) {
  GLint infoLogLength;
  (this->glGetShaderiv)(id, GL_INFO_LOG_LENGTH, &infoLogLength);

  char* _error = new char[infoLogLength+1];
  (this->glGetShaderInfoLog)(id, infoLogLength+1, &infoLogLength, _error);

  std::string error(_error);

  delete[] _error;

  return error;
}


GLuint WebGLRenderingContext::CreateProgram() {
  GLuint program=(this->glCreateProgram)();
  this->registerGLObj(GLOBJECT_TYPE_PROGRAM, program);

  return program;
}


void WebGLRenderingContext::AttachShader(GLint program, GLint shader) {
  (this->glAttachShader)(program, shader);
}

void WebGLRenderingContext::ValidateProgram(GLuint program) {
#if !defined(__vita__)
  (this->glValidateProgram)(program);
#else
  // throw std::runtime_error("glValidateProgram is not implemented in this platform!");
#endif
}

void WebGLRenderingContext::LinkProgram(GLuint program) {
  (this->glLinkProgram)(program);
}


GLint WebGLRenderingContext::GetProgramParameter(GLint program, GLenum pname) {
  GLint value = 0;
  (this->glGetProgramiv)(program, pname, &value);
  return value;
}


GLint WebGLRenderingContext::GetUniformLocation(GLint program, const std::string& name) {
  return (this->glGetUniformLocation)(program, name.c_str());
}


void WebGLRenderingContext::ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
  (this->glClearColor)(red, green, blue, alpha);
}


void WebGLRenderingContext::ClearDepth(GLfloat depth) {
  (this->glClearDepthf)(depth);
}

void WebGLRenderingContext::Disable(GLenum cap) {
  (this->glDisable)(cap);
}

void WebGLRenderingContext::Enable(GLenum cap) {
  (this->glEnable)(cap);
}


GLuint WebGLRenderingContext::CreateTexture() {
  GLuint texture;
  (this->glGenTextures)(1, &texture);
  this->registerGLObj(GLOBJECT_TYPE_TEXTURE, texture);

  return texture;
}


void WebGLRenderingContext::BindTexture(GLenum target, GLint texture) {
  (this->glBindTexture)(target, texture);
}

unsigned char* WebGLRenderingContext::unpackPixels(
  GLenum type,
  GLenum format,
  GLint width,
  GLint height,
  unsigned char* pixels) {

  //Compute pixel size
  GLint pixelSize = 1;
  if(type == GL_UNSIGNED_BYTE || type == GL_FLOAT) {
    if(type == GL_FLOAT) {
      pixelSize = 4;
    }
    switch(format) {
      case GL_ALPHA:
      case GL_LUMINANCE:
      break;
      case GL_LUMINANCE_ALPHA:
        pixelSize *= 2;
      break;
      case GL_RGB:
        pixelSize *= 3;
      break;
      case GL_RGBA:
        pixelSize *= 4;
      break;
    }
  } else {
    pixelSize = 2;
  }

  //Compute row stride
  GLint rowStride = pixelSize * width;
  if((rowStride % unpack_alignment) != 0) {
    rowStride += unpack_alignment - (rowStride % unpack_alignment);
  }

  GLint imageSize = rowStride * height;
  unsigned char* unpacked = new unsigned char[imageSize];

  if(unpack_flip_y) {
    for(int i=0,j=height-1; j>=0; ++i, --j) {
      memcpy(
          reinterpret_cast<void*>(unpacked + j*rowStride)
        , reinterpret_cast<void*>(pixels   + i*rowStride)
        , width * pixelSize);
    }
  } else {
    memcpy(
        reinterpret_cast<void*>(unpacked)
      , reinterpret_cast<void*>(pixels)
      , imageSize);
  }

  //Premultiply alpha unpacking
  if(unpack_premultiply_alpha &&
     (format == GL_LUMINANCE_ALPHA ||
      format == GL_RGBA)) {

    for(int row=0; row<height; ++row) {
      for(int col=0; col<width; ++col) {
        unsigned char* pixel = unpacked + (row * rowStride) + (col * pixelSize);
        if(format == GL_LUMINANCE_ALPHA) {
          pixel[0] *= pixel[1] / 255.0;
        } else if(type == GL_UNSIGNED_BYTE) {
          float scale = pixel[3] / 255.0;
          pixel[0] *= scale;
          pixel[1] *= scale;
          pixel[2] *= scale;
        } else if(type == GL_UNSIGNED_SHORT_4_4_4_4) {
          int r = pixel[0]&0x0f;
          int g = pixel[0]>>4;
          int b = pixel[1]&0x0f;
          int a = pixel[1]>>4;

          float scale = a / 15.0;
          r *= scale;
          g *= scale;
          b *= scale;

          pixel[0] = r + (g<<4);
          pixel[1] = b + (a<<4);
        } else if(type == GL_UNSIGNED_SHORT_5_5_5_1) {
          if((pixel[0]&1) == 0) {
            pixel[0] = 1; //why does this get set to 1?!?!?!
            pixel[1] = 0;
          }
        }
      }
    }
  }

  return unpacked;
}

void WebGLRenderingContext::TexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLint type, WebGLByteArray pixels) {
  if(pixels.has_value() && !pixels.value().empty()) {
    if(this->unpack_flip_y || this->unpack_premultiply_alpha) {
      unsigned char* unpacked = this->unpackPixels(
          type
        , format
        , width
        , height
        , pixels.value().data());
      (this->glTexImage2D)(
          target
        , level
        , internalformat
        , width
        , height
        , border
        , format
        , type
        , unpacked);
      delete[] unpacked;
    } else {
      (this->glTexImage2D)(
          target
        , level
        , internalformat
        , width
        , height
        , border
        , format
        , type
        , pixels.value().data());
    }
  } else {
    size_t length = width * height * 4;
    if(type == GL_FLOAT) {
      length *= 4;
    }
    char* data = new char[length];
    memset(data, 0, length);
    (this->glTexImage2D)(
        target
      , level
      , internalformat
      , width
      , height
      , border
      , format
      , type
      , data);
    delete[] data;
  }
}

void WebGLRenderingContext::TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, WebGLByteArray pixels) {
  if(pixels.has_value()) {
    if(this->unpack_flip_y ||
     this->unpack_premultiply_alpha) {
      unsigned char* unpacked = this->unpackPixels(
          type
        , format
        , width
        , height
        , pixels.value().data());
      (this->glTexSubImage2D)(
          target
        , level
        , xoffset
        , yoffset
        , width
        , height
        , format
        , type
        , unpacked);
      delete[] unpacked;
    } else {
      (this->glTexSubImage2D)(
          target
        , level
        , xoffset
        , yoffset
        , width
        , height
        , format
        , type
        , pixels.value().data());
    }
  }
}

void WebGLRenderingContext::TexParameteri(GLenum target, GLenum pname, GLint param) {
  (this->glTexParameteri)(target, pname, param);
}

void WebGLRenderingContext::TexParameterf(GLenum target, GLenum pname, GLfloat param) {
  (this->glTexParameterf)(target, pname, param);
}

void WebGLRenderingContext::Clear(GLbitfield mask) { 
  (this->glClear)(mask);
}

void WebGLRenderingContext::UseProgram(GLuint program) {  
  (this->glUseProgram)(program);
}

GLuint WebGLRenderingContext::CreateBuffer() {  
  GLuint buffer;
  (this->glGenBuffers)(1, &buffer);
  this->registerGLObj(GLOBJECT_TYPE_BUFFER, buffer);

  return buffer;
}

void WebGLRenderingContext::BindBuffer(GLenum target, GLuint buffer) {
  (this->glBindBuffer)(target,buffer);
}


GLuint WebGLRenderingContext::CreateFramebuffer() {
  GLuint buffer;
  (this->glGenFramebuffers)(1, &buffer);
  this->registerGLObj(GLOBJECT_TYPE_FRAMEBUFFER, buffer);

  return buffer;
}


void WebGLRenderingContext::BindFramebuffer(GLint target, GLint buffer) { 
  (this->glBindFramebuffer)(target, buffer);
}

void WebGLRenderingContext::FramebufferTexture2D(GLenum target, GLenum attachment, GLint textarget, GLint texture, GLint level) {
  // Handle depth stencil case separately
  if(attachment == 0x821A) {
    (this->glFramebufferTexture2D)(
        target
      , GL_DEPTH_ATTACHMENT
      , textarget
      , texture
      , level);
    (this->glFramebufferTexture2D)(
        target
      , GL_STENCIL_ATTACHMENT
      , textarget
      , texture
      , level);
  } else {
    (this->glFramebufferTexture2D)(
        target
      , attachment
      , textarget
      , texture
      , level);
  }
}

template <typename... Args>
struct GLBufferData{};

template <typename T1, typename T2, typename... Args>
struct GLBufferData<T1, T2, Args...> 
{
  static void apply(PFNGLBUFFERDATAPROC glBufferData, GLint target, WebGLBufferDataInternal data, GLenum usage)
  {
    if(std::holds_alternative<T1>(data)) {
      GLBufferData<T1>::apply(glBufferData, target, data, usage);
    } else {
      GLBufferData<T2, Args...>::apply(glBufferData, target, data, usage);
    }
  }
};

template <typename T>
struct GLBufferData<T> 
{
  static void apply(PFNGLBUFFERDATAPROC glBufferData, GLint target, WebGLBufferDataInternal data, GLenum usage) {
    if(std::holds_alternative<T>(data)) {
      const auto& array = std::get<T>(data);
      glBufferData(target, static_cast<GLsizeiptr>(array.size() * T::bytesPerElement), (void*)(array.data()), usage); 
    }
  }
};

void WebGLRenderingContext::BufferData(GLint target, WebGLBufferData data, GLenum usage) {
  if(data.has_value()) {
    const auto& _data = data.value();
    if(std::holds_alternative<GLsizeiptr>(_data)) {
      const auto& size = std::get<GLsizeiptr>(_data);
      (this->glBufferData)(target, size, NULL, usage);
    } else if(std::holds_alternative<qjs::ArrayBuffer>(_data)) {
      const auto& array = std::get<qjs::ArrayBuffer>(_data);
      (this->glBufferData)(target, array.size(), (void*)(array.data()), usage);
    } else {
      GLBufferData< 
        qjs::Int8Array,
        qjs::Int16Array,
        qjs::Int32Array,
        qjs::Uint8Array,
        qjs::Uint16Array,
        qjs::Uint32Array,
        qjs::Float32Array,
        qjs::Float64Array,
        qjs::BigInt64Array,
        qjs::BigUint64Array
      >::apply(this->glBufferData, target, _data, usage);
    }
  }
}

void WebGLRenderingContext::BufferSubData(GLenum target, GLint offset, WebGLByteArray array) {
  if(array.has_value())
    (this->glBufferSubData)(target, offset, array.value().size(), array.value().data());
}


void WebGLRenderingContext::BlendEquation(GLenum mode) {
  (this->glBlendEquation)(mode);
}


void WebGLRenderingContext::BlendFunc(GLenum sfactor, GLenum dfactor) {
  (this->glBlendFunc)(sfactor,dfactor);
}


void WebGLRenderingContext::EnableVertexAttribArray(GLuint index) {
  (this->glEnableVertexAttribArray)(index);
}

void WebGLRenderingContext::VertexAttribPointer(GLint index, GLint size, GLenum type, GLboolean normalized, GLint stride, qjs_size_t offset) {
  (this->glVertexAttribPointer)(
    index,
    size,
    type,
    normalized,
    stride,
    reinterpret_cast<GLvoid*>(offset));
}


void WebGLRenderingContext::ActiveTexture(GLenum texture) {
  (this->glActiveTexture)(texture);
}


void WebGLRenderingContext::DrawElements(GLenum mode, GLint count, GLenum type, qjs_size_t offset) {
  (this->glDrawElements)(mode, count, type, reinterpret_cast<GLvoid*>(offset));
}


void WebGLRenderingContext::Flush() {
  (this->glFlush)();
}

void WebGLRenderingContext::Finish() {
  (this->glFinish)();
}

void WebGLRenderingContext::VertexAttrib1f(GLuint index, GLfloat x) {
  (this->glVertexAttrib1f)(index, x);
}

void WebGLRenderingContext::VertexAttrib2f(GLuint index, GLfloat x, GLfloat y) {
  (this->glVertexAttrib2f)(index, x, y);
}

void WebGLRenderingContext::VertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) {
  (this->glVertexAttrib3f)(index, x, y, z);
}

void WebGLRenderingContext::VertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
  (this->glVertexAttrib4f)(index, x, y, z, w);
}

void WebGLRenderingContext::BlendColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a) {
#if !defined(__vita__)
  (this->glBlendColor)(r, g, b, a);
#else
  // throw std::runtime_error("glBlendColor is not implemented in this platform!");
#endif
}

void WebGLRenderingContext::BlendEquationSeparate(GLenum mode_rgb, GLenum mode_alpha) {
  (this->glBlendEquationSeparate)(mode_rgb, mode_alpha);
}

void WebGLRenderingContext::BlendFuncSeparate(GLenum src_rgb, GLenum dst_rgb, GLenum src_alpha, GLenum dst_alpha) {
  (this->glBlendFuncSeparate)(src_rgb, dst_rgb, src_alpha, dst_alpha);
}

void WebGLRenderingContext::ClearStencil(GLint s) {
  (this->glClearStencil)(s);
}

void WebGLRenderingContext::ColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a) {
  (this->glColorMask)(r, g, b, a);
}

void WebGLRenderingContext::CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
  (this->glCopyTexImage2D)(target, level, internalformat, x, y, width, height, border);
}

void WebGLRenderingContext::CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
  (this->glCopyTexSubImage2D)(target, level, xoffset, yoffset, x, y, width, height);
}

void WebGLRenderingContext::CullFace(GLenum mode) {
  (this->glCullFace)(mode);
}

void WebGLRenderingContext::DepthMask(GLboolean flag) {
  (this->glDepthMask)(flag);
}

void WebGLRenderingContext::DepthRange(GLclampf zNear, GLclampf zFar) {
  (this->glDepthRangef)(zNear, zFar);
}

void WebGLRenderingContext::DisableVertexAttribArray(GLuint index) {
  (this->glDisableVertexAttribArray)(index);
}

void WebGLRenderingContext::Hint(GLenum target, GLenum mode) {
  (this->glHint)(target, mode);
}

bool WebGLRenderingContext::IsEnabled(GLenum cap) {
  return (this->glIsEnabled)(cap) != 0;
}

void WebGLRenderingContext::LineWidth(GLfloat width) {
  (this->glLineWidth)(width);
}

void WebGLRenderingContext::PolygonOffset(GLfloat factor, GLfloat units) {
  (this->glPolygonOffset)(factor, units);
}

void WebGLRenderingContext::SampleCoverage(GLclampf value, GLboolean invert) {
#if !defined(__vita__)
  (this->glSampleCoverage)(value, invert);
#else
  // throw std::runtime_error("glSampleCoverage is not implemented in this platform!");
#endif
}

void WebGLRenderingContext::Scissor(GLint x, GLint y, GLsizei width, GLsizei height) {
  (this->glScissor)(x, y, width, height);
}

void WebGLRenderingContext::StencilFunc(GLenum func, GLint ref, GLuint mask) {
  (this->glStencilFunc)(func, ref, mask);
}

void WebGLRenderingContext::StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
  (this->glStencilFuncSeparate)(face, func, ref, mask);
}

void WebGLRenderingContext::StencilMask(GLuint mask) {
  (this->glStencilMask)(mask);
}

void WebGLRenderingContext::StencilMaskSeparate(GLenum face, GLuint mask) {
  (this->glStencilMaskSeparate)(face, mask);
}

void WebGLRenderingContext::StencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
  (this->glStencilOp)(fail, zfail, zpass);
}

void WebGLRenderingContext::StencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass) {
  (this->glStencilOpSeparate)(face, fail, zfail, zpass);
}

void WebGLRenderingContext::BindRenderbuffer(GLenum target, GLuint buffer) {
  (this->glBindRenderbuffer)(target, buffer);
}

GLuint WebGLRenderingContext::CreateRenderbuffer() {
  GLuint renderbuffers;
  (this->glGenRenderbuffers)(1, &renderbuffers);

  this->registerGLObj(GLOBJECT_TYPE_RENDERBUFFER, renderbuffers);

  return renderbuffers;
}

void WebGLRenderingContext::DeleteBuffer(GLuint buffer) {
  (this->glDeleteBuffers)(1, &buffer);
}

void WebGLRenderingContext::DeleteFramebuffer(GLuint buffer) {
  this->unregisterGLObj(GLOBJECT_TYPE_FRAMEBUFFER, buffer);

  (this->glDeleteFramebuffers)(1, &buffer);
}

void WebGLRenderingContext::DeleteProgram(GLuint program) {
  this->unregisterGLObj(GLOBJECT_TYPE_PROGRAM, program);

  (this->glDeleteProgram)(program);
}

void WebGLRenderingContext::DeleteRenderbuffer(GLuint renderbuffer) {
  this->unregisterGLObj(GLOBJECT_TYPE_RENDERBUFFER, renderbuffer);

  (this->glDeleteRenderbuffers)(1, &renderbuffer);
}

void WebGLRenderingContext::DeleteShader(GLuint shader) {
  this->unregisterGLObj(GLOBJECT_TYPE_SHADER, shader);

  (this->glDeleteShader)(shader);
}

void WebGLRenderingContext::DeleteTexture(GLuint texture) {
  this->unregisterGLObj(GLOBJECT_TYPE_TEXTURE, texture);

  (this->glDeleteTextures)(1, &texture);
}

void WebGLRenderingContext::DetachShader(GLuint program, GLuint shader) {
#if !defined(__vita__)
  (this->glDetachShader)(program, shader);
#else
  // throw std::runtime_error("glDetachShader is not implemented in this platform!"); 
#endif
}

void WebGLRenderingContext::FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
  // Handle depth stencil case separately
  if(attachment == 0x821A) {
    (this->glFramebufferRenderbuffer)(
        target
      , GL_DEPTH_ATTACHMENT
      , renderbuffertarget
      , renderbuffer);
    (this->glFramebufferRenderbuffer)(
        target
      , GL_STENCIL_ATTACHMENT
      , renderbuffertarget
      , renderbuffer);
  } else {
    (this->glFramebufferRenderbuffer)(
        target
      , attachment
      , renderbuffertarget
      , renderbuffer);
  }
}

GLuint WebGLRenderingContext::GetVertexAttribOffset(GLuint index, GLenum pname) {
  void *ret = NULL;
  (this->glGetVertexAttribPointerv)(index, pname, &ret);

  GLuint offset = static_cast<GLuint>(reinterpret_cast<size_t>(ret));
  return offset;
}

GLboolean WebGLRenderingContext::IsBuffer(GLuint buffer) {
#if !defined(__vita__)
  return (this->glIsBuffer)(buffer);
#else
  // throw std::runtime_error("glGetShaderPrecisionFormat is not implemented in this platform!");
  return GL_FALSE;
#endif
}

GLboolean WebGLRenderingContext::IsFramebuffer(GLuint framebuffer) {
  return (this->glIsFramebuffer)(framebuffer);
}

GLboolean WebGLRenderingContext::IsProgram(GLuint program) {
  return (this->glIsProgram)(program);
}

GLboolean WebGLRenderingContext::IsRenderbuffer(GLuint renderbuffer) {
  return (this->glIsRenderbuffer)(renderbuffer);
}

GLboolean WebGLRenderingContext::IsShader(GLuint shader) {
#if !defined(__vita__)
  return (this->glIsShader)(shader);
#else
  // throw std::runtime_error("glGetShaderPrecisionFormat is not implemented in this platform!");
  return GL_FALSE;
#endif
}

GLboolean WebGLRenderingContext::IsTexture(GLuint texture) {
  return (this->glIsTexture)(texture);
}

void WebGLRenderingContext::RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
  //In WebGL, we map GL_DEPTH_STENCIL to GL_DEPTH24_STENCIL8
  if (internalformat == GL_DEPTH_STENCIL_OES) {
    internalformat = GL_DEPTH24_STENCIL8_OES;
  } else if (internalformat == GL_DEPTH_COMPONENT32_OES) {
    internalformat = this->preferredDepth;
  }

  (this->glRenderbufferStorage)(target, internalformat, width, height);
}

std::string WebGLRenderingContext::GetShaderSource(GLint shader) {
  GLint len;
  (this->glGetShaderiv)(shader, GL_SHADER_SOURCE_LENGTH, &len);

  GLchar *source = new GLchar[len];
  (this->glGetShaderSource)(shader, len, NULL, source);
  std::string str = source;
  delete[] source;

  return str;
}

void WebGLRenderingContext::ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, WebGLByteArray pixels) {
  if(pixels.has_value())
    (this->glReadPixels)(x, y, width, height, format, type, pixels.value().data());
}

WebGLTexParameter WebGLRenderingContext::GetTexParameter(GLenum target, GLenum pname) {
#if !defined(__vita__)
  if (pname == GL_TEXTURE_MAX_ANISOTROPY_EXT) {
    GLfloat param_value = 0;
    (this->glGetTexParameterfv)(target, pname, &param_value);
    return WebGLTexParameter(param_value);
  } else {
    GLint param_value = 0;
    (this->glGetTexParameteriv)(target, pname, &param_value);
    return WebGLTexParameter(param_value);
  }
#else
  // throw std::runtime_error("glSampleCoverage is not implemented in this platform!");
  return std::nullopt;
#endif
} 

WebGLActiveElement WebGLRenderingContext::GetActiveAttrib(GLuint program, GLuint index) {
  GLint maxLength;
  (this->glGetProgramiv)(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);

  char* name = new char[maxLength];
  GLsizei length = 0;
  GLenum  type;
  GLsizei size;
  (this->glGetActiveAttrib)(program, index, maxLength, &length, &size, &type, name);

  std::string sname(name);
  delete[] name;

  if (length > 0) {
    return WebGLActiveElementContent(size, type, sname);
  } else {
    return std::nullopt;
  }
}

WebGLActiveElement WebGLRenderingContext::GetActiveUniform(GLuint program, GLuint index) {
  GLint maxLength;
  (this->glGetProgramiv)(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);

  char* name = new char[maxLength];
  GLsizei length = 0;
  GLenum  type;
  GLsizei size;
  (this->glGetActiveUniform)(program, index, maxLength, &length, &size, &type, name);

  std::string sname(name);
  delete[] name;

  if (length > 0) {
    return WebGLActiveElementContent(size, type, sname);
  } else {
    return std::nullopt;
  }
}

std::vector<GLsizei> WebGLRenderingContext::GetAttachedShaders(GLuint program) {
  GLint numAttachedShaders;
  (this->glGetProgramiv)(program, GL_ATTACHED_SHADERS, &numAttachedShaders);

  GLuint* shaders = new GLuint[numAttachedShaders];
  GLsizei count;
  (this->glGetAttachedShaders)(program, numAttachedShaders, &count, shaders);

  std::vector<GLsizei> ret(shaders, shaders + count);

  delete[] shaders;

  return ret;
}

 WebGLParameter WebGLRenderingContext::GetParameter(GLenum name) {
  switch(name) {
    case 0x9240 /* UNPACK_FLIP_Y_WEBGL */:
      return WebGLParameter(this->unpack_flip_y);

    case 0x9241 /* UNPACK_PREMULTIPLY_ALPHA_WEBGL*/:
      return WebGLParameter(this->unpack_premultiply_alpha);

    case 0x9243 /* UNPACK_COLORSPACE_CONVERSION_WEBGL */:
      return WebGLParameter(this->unpack_colorspace_conversion);

    case GL_BLEND:
    case GL_CULL_FACE:
    case GL_DEPTH_TEST:
    case GL_DEPTH_WRITEMASK:
    case GL_DITHER:
    case GL_POLYGON_OFFSET_FILL:
    case GL_SAMPLE_COVERAGE_INVERT:
    case GL_SCISSOR_TEST:
    case GL_STENCIL_TEST:
    {
      GLboolean params;
      (this->glGetBooleanv)(name, &params);

      return WebGLParameter(params != 0);
    }

    case GL_DEPTH_CLEAR_VALUE:
    case GL_LINE_WIDTH:
    case GL_POLYGON_OFFSET_FACTOR:
    case GL_POLYGON_OFFSET_UNITS:
    case GL_SAMPLE_COVERAGE_VALUE:
    case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
    {
      GLfloat params;
      (this->glGetFloatv)(name, &params);

      return WebGLParameter(params);
    }

    case GL_RENDERER:
    case GL_SHADING_LANGUAGE_VERSION:
    case GL_VENDOR:
    case GL_VERSION:
    case GL_EXTENSIONS:
    {
      const char *params = reinterpret_cast<const char*>((this->glGetString)(name));
      if(params) {
        return WebGLParameter(params);
      }
    }

    case GL_MAX_VIEWPORT_DIMS:
    {
      GLint params[2];
      (this->glGetIntegerv)(name, params);

      return WebGLParameter(std::vector<GLint>(params, params + 2));
    }

    case GL_SCISSOR_BOX:
    case GL_VIEWPORT:
    {
      GLint params[4];
      (this->glGetIntegerv)(name, params);

      return WebGLParameter(std::vector<GLint>(params, params + 4));
    }

    case GL_ALIASED_LINE_WIDTH_RANGE:
    case GL_ALIASED_POINT_SIZE_RANGE:
    case GL_DEPTH_RANGE:
    {
      GLfloat params[2];
      (this->glGetFloatv)(name, params);

      return WebGLParameter(std::vector<GLfloat>(params, params + 2));
    }

    case GL_BLEND_COLOR:
    case GL_COLOR_CLEAR_VALUE:
    {
      GLfloat params[4];
      (this->glGetFloatv)(name, params);

      return WebGLParameter(std::vector<GLfloat>(params, params + 4));
    }

    case GL_COLOR_WRITEMASK:
    {
      GLboolean params[4];
      (this->glGetBooleanv)(name, params);

      return WebGLParameter(std::vector<GLboolean>(params, params + 4));
    }

    default:
    {
      GLint params;
      (this->glGetIntegerv)(name, &params);
      return WebGLParameter(params);
    }
  }
}

GLint WebGLRenderingContext::GetBufferParameter(GLenum target, GLenum pname) {
  GLint params;
  (this->glGetBufferParameteriv)(target, pname, &params);

  return params;
}

GLint WebGLRenderingContext::GetFramebufferAttachmentParameter(GLenum target, GLenum attachment, GLenum pname) {
  GLint params;
  (this->glGetFramebufferAttachmentParameteriv)(target, attachment, pname, &params);

  return params;
}

std::string WebGLRenderingContext::GetProgramInfoLog(GLuint program) {
  GLint infoLogLength;
  (this->glGetProgramiv)(program, GL_INFO_LOG_LENGTH, &infoLogLength);

  char* error = new char[infoLogLength+1];
  (this->glGetProgramInfoLog)(program, infoLogLength+1, &infoLogLength, error);

  std::string ret(error);

  delete[] error;

  return ret;
}

WebGLShaderPrecisionFormat WebGLRenderingContext::GetShaderPrecisionFormat(GLenum shaderType, GLenum precisionType) {
#if !defined(__vita__)
  GLint range[2];
  GLint precision;

  (this->glGetShaderPrecisionFormat)(shaderType, precisionType, range, &precision);

  return WebGLShaderPrecisionFormatContent(range[0], range[1], precision);
#else
  // throw std::runtime_error("glGetShaderPrecisionFormat is not implemented in this platform!");
  return std::nullopt;
#endif
}

int WebGLRenderingContext::GetRenderbufferParameter(GLenum target, GLenum pname) {
#if !defined(__vita__)
  int value;
  (this->glGetRenderbufferParameteriv)(target, pname, &value);

  return value;
#else
  // throw std::runtime_error("glGetRenderbufferParameteriv is not implemented in this platform!");
  return 0;
#endif
}

std::vector<float> WebGLRenderingContext::GetUniform(GLint program, GLint location) {
#if !defined(__vita__)
  float data[16];
  (this->glGetUniformfv)(program, location, data);

  return std::vector<float>(data, data + 16);
#else
  // throw std::runtime_error("glSampleCoverage is not implemented in this platform!");
  return std::vector<float>{};
#endif
}

WebGLAttrib WebGLRenderingContext::GetVertexAttrib(GLint index, GLenum pname) {
  GLint value;

  switch (pname) {
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
    {
      (this->glGetVertexAttribiv)(index, pname, &value);
      return WebGLAttrib(value);
    }

    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
    {
      (this->glGetVertexAttribiv)(index, pname, &value);
      return WebGLAttrib(value);
    }

    case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
    {
      (this->glGetVertexAttribiv)(index, pname, &value);
      return WebGLAttrib(value);
    }

    case GL_CURRENT_VERTEX_ATTRIB:
    {
      std::array<float, 4> vextex_attribs;

      (this->glGetVertexAttribfv)(index, pname, vextex_attribs.data());

      std::vector<float> vextex_attribs_vector(vextex_attribs.begin(), vextex_attribs.end());
      return WebGLAttrib(std::move(vextex_attribs_vector));
    }

    default:
      return WebGLAttrib(GL_INVALID_ENUM);
  }
}

const char* WebGLRenderingContext::GetSupportedExtensions() {
  
  const char *extensions = reinterpret_cast<const char*>(
    (this->glGetString)(GL_EXTENSIONS));
  
  return extensions;
}

// WebGLExtension WebGLRenderingContext::GetExtension(const std::string& name) {
//   if(name == "WEBGL_draw_buffers") {
//     WebGLDrawBuffersExtension ret;
//     return ret;
//   }
//   //TODO
//   return nullptr;
// }

GLenum WebGLRenderingContext::CheckFramebufferStatus(GLenum target) {
  return (this->glCheckFramebufferStatus)(target);
}

void WebGLRenderingContext::DrawBuffersWEBGL(const std::vector<GLenum>& buffersArray) {
#if !defined(__vita__)
  (this->glDrawBuffersEXT)(buffersArray.size(), buffersArray.data());
#else
  // throw std::runtime_error("glSampleCoverage is not implemented in this platform!");
#endif
}

WebGLDrawBuffersExtension WebGLRenderingContext::EXTWEBGL_draw_buffers() {
  return WebGLDrawBuffersExtension{};
}

void WebGLRenderingContext::BindVertexArrayOES(GLuint array) {
  (this->glBindVertexArrayOES)(array);
}

GLuint WebGLRenderingContext::CreateVertexArrayOES() {
  GLuint array = 0;
  (this->glGenVertexArraysOES)(1, &array);
  this->registerGLObj(GLOBJECT_TYPE_VERTEX_ARRAY, array);

  return array;
}

void WebGLRenderingContext::DeleteVertexArrayOES(GLuint array) {
  this->unregisterGLObj(GLOBJECT_TYPE_VERTEX_ARRAY, array);

  (this->glDeleteVertexArraysOES)(1, &array);
}

bool WebGLRenderingContext::IsVertexArrayOES(GLuint array) {
#if !defined(__vita__)
  return (this->glIsVertexArrayOES)(array) != 0;
#else
  // throw std::runtime_error("glSampleCoverage is not implemented in this platform!");
  return false;
#endif
}
