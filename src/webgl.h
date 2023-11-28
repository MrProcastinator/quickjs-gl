#ifndef WEBGL_H_
#define WEBGL_H_

#include <algorithm>
#include <vector>
#include <map>
#include <utility>
#include <memory>
#include <variant>
#include <optional>

#include <quickjs/quickjspp.hpp>
#include <quickjs/quickjspp-arraybuffer.hpp>

#ifdef __vita__
#include "platform/vita/egl.h"
#else
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

enum GLObjectType {
  GLOBJECT_TYPE_BUFFER,
  GLOBJECT_TYPE_FRAMEBUFFER,
  GLOBJECT_TYPE_PROGRAM,
  GLOBJECT_TYPE_RENDERBUFFER,
  GLOBJECT_TYPE_SHADER,
  GLOBJECT_TYPE_TEXTURE,
  GLOBJECT_TYPE_VERTEX_ARRAY,
};

enum GLContextState {
  GLCONTEXT_STATE_INIT,
  GLCONTEXT_STATE_OK,
  GLCONTEXT_STATE_DESTROY,
  GLCONTEXT_STATE_ERROR
};

// QuickJS uses up to signed 64-bit integers so we downcast to 32-bit 
// in those cases (may cause bugs in 64-bit architectures
using qjs_size_t = std::conditional<std::is_same_v<size_t, uint64_t>, uint32_t, size_t>::type;

using WebGLParameter = std::variant<
  GLboolean,
  GLint,
  GLfloat,
  const char*,
  std::vector<GLfloat>,
  std::vector<GLint>,
  std::vector<GLboolean>
>;

using WebGLAttrib = std::variant<
  GLint,
  std::vector<float>
>;

using WebGLTexParameter = std::optional<std::variant<
  GLfloat,
  GLint
>>;

using WebGLBufferDataInternal = std::variant<
  qjs::ArrayBuffer,
  qjs::Int8Array,
  qjs::Int16Array,
  qjs::Int32Array,
  qjs::Uint8Array,
  qjs::Uint16Array,
  qjs::Uint32Array,
  qjs::Float32Array,
  qjs::Float64Array,
  qjs::BigInt64Array,
  qjs::BigUint64Array,
  GLsizeiptr
>;

using WebGLBufferData = std::optional<WebGLBufferDataInternal>;

using WebGLByteArray = std::optional<qjs::Uint8Array>;

using GLObjectReference = std::pair<GLuint, GLObjectType>;

struct WebGLActiveElementContent {
  GLsizei size;
  GLenum  type;
  std::string name;

  WebGLActiveElementContent(GLsizei size, GLenum  type, std::string name) : size(size), type(type), name(std::move(name)) {}
};

using WebGLActiveElement = std::optional<WebGLActiveElementContent>;

struct WebGLShaderPrecisionFormatContent {
  GLint rangeMin;
  GLint rangeMax;
  GLint precision;

  WebGLShaderPrecisionFormatContent(GLint rangeMin, GLint rangeMax, GLint precision) : rangeMin(rangeMin), rangeMax(rangeMax), precision(precision) {}
};

using WebGLShaderPrecisionFormat = std::optional<WebGLShaderPrecisionFormatContent>;

// WebGL Extensions (defined in bindings)
struct WebGLDrawBuffersExtension {};

// Empty struct to use with quickjspp
typedef std::optional<std::variant<
  WebGLDrawBuffersExtension
>> WebGLExtension;

struct WebGLRenderingContext {

  //The underlying OpenGL context
  static bool       HAS_DISPLAY;
  static EGLDisplay DISPLAY;

  /* WebGL-specific enums */
  static const GLenum STENCIL_INDEX = 0x1901;
  static const GLenum UNPACK_FLIP_Y_WEBGL = 0x9240;
  static const GLenum UNPACK_PREMULTIPLY_ALPHA_WEBGL = 0x9241;
  static const GLenum CONTEXT_LOST_WEBGL = 0x9242;
  static const GLenum UNPACK_COLORSPACE_CONVERSION_WEBGL = 0x9243;
  static const GLenum BROWSER_DEFAULT_WEBGL = 0x9244;
  static const GLenum VERSION = 0x1F02;
  static const GLenum IMPLEMENTATION_COLOR_READ_TYPE = 0x8B9A;
  static const GLenum IMPLEMENTATION_COLOR_READ_FORMAT = 0x8B9B;

  EGLContext context;
  EGLConfig  config;
  EGLSurface surface;
  GLContextState  state;

  //Pixel storage flags
  bool  unpack_flip_y;
  bool  unpack_premultiply_alpha;
  GLint unpack_colorspace_conversion;
  GLint unpack_alignment;

  //A list of object references, need do destroy them at program exit
  std::map< std::pair<GLuint, GLObjectType>, bool > objects;
  void registerGLObj(GLObjectType type, GLuint obj) {
    objects[std::make_pair(obj, type)] = true;
  }
  void unregisterGLObj(GLObjectType type, GLuint obj) {
    objects.erase(std::make_pair(obj, type));
  }

  //Context list
  WebGLRenderingContext *next, *prev;
  static WebGLRenderingContext* CONTEXT_LIST_HEAD;
  void registerContext() {
    if(CONTEXT_LIST_HEAD) {
      CONTEXT_LIST_HEAD->prev = this;
    }
    next = CONTEXT_LIST_HEAD;
    prev = NULL;
    CONTEXT_LIST_HEAD = this;
  }
  void unregisterContext() {
    if(next) {
      next->prev = this->prev;
    }
    if(prev) {
      prev->next = this->next;
    }
    if(CONTEXT_LIST_HEAD == this) {
      CONTEXT_LIST_HEAD = this->next;
    }
    next = prev = NULL;
  }

  //Constructor
  WebGLRenderingContext(
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
    qjs::Value platformOptions);
    
  virtual ~WebGLRenderingContext() {
    dispose();
  }

  bool swap();

  //Context validation
  static WebGLRenderingContext* ACTIVE;
  bool setActive();

  //Unpacks a buffer full of pixels into memory
  unsigned char* unpackPixels(
    GLenum type,
    GLenum format,
    GLint width,
    GLint height,
    unsigned char* pixels);

  //Error handling
  GLenum lastError;
  void setError(GLenum error);
  GLenum getError();
  GLenum GetError();
  void SetError(GLenum error);

  //Preferred depth format
  GLenum preferredDepth;

  //Destructors
  void dispose();

  static void DisposeAll();

  void Destroy();

  void Swap();

  void VertexAttribDivisor(GLuint index, GLuint divisor);
  void DrawArraysInstanced(GLenum mode, GLint first, GLuint count, GLuint icount);
  void DrawElementsInstanced(GLenum mode, GLint count, GLenum type, GLint offset, GLuint icount);

  void Uniform1f(GLint location, GLfloat x);
  void Uniform2f(GLint location, GLfloat x, GLfloat y);
  void Uniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z);
  void Uniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
  void Uniform1i(GLint location, GLint x);
  void Uniform2i(GLint location, GLint x, GLint y);
  void Uniform3i(GLint location, GLint x, GLint y, GLint z);
  void Uniform4i(GLint location, GLint x, GLint y, GLint z, GLint w);

  void PixelStorei(GLenum pname, GLenum param);
  void BindAttribLocation(GLint program, GLint index, const std::string& name);
  void DrawArrays(GLenum mode, GLint first, GLint count);
  void UniformMatrix2fv(GLint location, GLboolean transpose, const std::vector<GLfloat> data);
  void UniformMatrix3fv(GLint location, GLboolean transpose, const std::vector<GLfloat> data);
  void UniformMatrix4fv(GLint location, GLboolean transpose, const std::vector<GLfloat> data);
  void GenerateMipmap(GLint target);
  GLint GetAttribLocation(GLint program, const std::string& name);
  void DepthFunc(GLenum func);
  void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);
  GLuint CreateShader(GLenum shaderType);
  void ShaderSource(GLint id, const std::string& code);
  void CompileShader(GLuint shader);  
  void FrontFace(GLenum mode);
  GLint GetShaderParameter(GLint shader, GLenum pname);
  std::string GetShaderInfoLog(GLint id);
  GLuint CreateProgram();
  void AttachShader(GLint program, GLint shader);
  void ValidateProgram(GLuint program);
  void LinkProgram(GLuint program);
  GLint GetProgramParameter(GLint program, GLenum pname);
  GLint GetUniformLocation(GLint program, const std::string& name);
  void ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
  void ClearDepth(GLfloat depth);
  void Disable(GLenum cap);
  void Enable(GLenum cap);
  GLuint CreateTexture();
  void BindTexture(GLenum target, GLint texture);
  void TexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLint type, WebGLByteArray pixels);
  void TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, WebGLByteArray pixels);
  void TexParameteri(GLenum target, GLenum pname, GLint param);
  void TexParameterf(GLenum target, GLenum pname, GLfloat param);
  void Clear(GLbitfield mask); 
  void UseProgram(GLuint program);  
  GLuint CreateBuffer();
  void BindBuffer(GLenum target, GLuint buffer);
  GLuint CreateFramebuffer();
  void BindFramebuffer(GLint target, GLint buffer); 
  void FramebufferTexture2D(GLenum target, GLenum attachment, GLint textarget, GLint texture, GLint level);
  void BufferData(GLint target, WebGLBufferData data, GLenum usage);
  void BufferSubData(GLenum target, GLint offset, WebGLByteArray array);
  void BlendEquation(GLenum mode);
  void BlendFunc(GLenum sfactor, GLenum dfactor);
  void EnableVertexAttribArray(GLuint index);
  void VertexAttribPointer(GLint index, GLint size, GLenum type, GLboolean normalized, GLint stride, qjs_size_t offset);
  void ActiveTexture(GLenum texture);
  void DrawElements(GLenum mode, GLint count, GLenum type, qjs_size_t offset);
  void Flush();
  void Finish();

  void VertexAttrib1f(GLuint index, GLfloat x);
  void VertexAttrib2f(GLuint index, GLfloat x, GLfloat y);
  void VertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z);
  void VertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
  
  void BlendColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
  void BlendEquationSeparate(GLenum mode_rgb, GLenum mode_alpha);
  void BlendFuncSeparate(GLenum src_rgb, GLenum dst_rgb, GLenum src_alpha, GLenum dst_alpha);
  void ClearStencil(GLint s);
  void ColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a);
  void CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
  void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
  void CullFace(GLenum mode);
  void DepthMask(GLboolean flag);
  void DepthRange(GLclampf zNear, GLclampf zFar);
  void DisableVertexAttribArray(GLuint index);
  void Hint(GLenum target, GLenum mode);
  bool IsEnabled(GLenum cap);
  void LineWidth(GLfloat width);
  void PolygonOffset(GLfloat factor, GLfloat units);
  void SampleCoverage(GLclampf value, GLboolean invert);
  void Scissor(GLint x, GLint y, GLsizei width, GLsizei height);
  void StencilFunc(GLenum func, GLint ref, GLuint mask);
  void StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
  void StencilMask(GLuint mask);
  void StencilMaskSeparate(GLenum face, GLuint mask);
  void StencilOp(GLenum fail, GLenum zfail, GLenum zpass);
  void StencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
  void BindRenderbuffer(GLenum target, GLuint buffer);
  GLuint CreateRenderbuffer();
  void DeleteBuffer(GLuint buffer);
  void DeleteFramebuffer(GLuint buffer);
  void DeleteProgram(GLuint program);
  void DeleteRenderbuffer(GLuint renderbuffer);
  void DeleteShader(GLuint shader);
  void DeleteTexture(GLuint texture);
  void DetachShader(GLuint program, GLuint shader);
  void FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
  GLuint GetVertexAttribOffset(GLuint index, GLenum pname);
  GLboolean IsBuffer(GLuint buffer);
  GLboolean IsFramebuffer(GLuint framebuffer);
  GLboolean IsProgram(GLuint program);
  GLboolean IsRenderbuffer(GLuint renderbuffer);
  GLboolean IsShader(GLuint shader);
  GLboolean IsTexture(GLuint texture);
  void RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
  std::string GetShaderSource(GLint shader);
  void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei heigh, GLenum format, GLenum type, WebGLByteArray pixels);
  WebGLTexParameter GetTexParameter(GLenum target, GLenum pname);
  WebGLActiveElement GetActiveAttrib(GLuint program, GLuint index);
  WebGLActiveElement GetActiveUniform(GLuint program, GLuint index);
  std::vector<GLsizei> GetAttachedShaders(GLuint program);
  WebGLParameter GetParameter(GLenum name);
  GLint GetBufferParameter(GLenum target, GLenum pname);
  GLint GetFramebufferAttachmentParameter(GLenum target, GLenum attachment, GLenum pname);
  std::string GetProgramInfoLog(GLuint program);
  WebGLShaderPrecisionFormat GetShaderPrecisionFormat(GLenum shaderType, GLenum precisionType);
  int GetRenderbufferParameter(GLenum target, GLenum pname);
  std::vector<float> GetUniform(GLint program, GLint location);
  WebGLAttrib GetVertexAttrib(GLint index, GLenum pname);
  const char* GetSupportedExtensions();
  // WebGLExtension GetExtension(const std::string& name);
  GLenum CheckFramebufferStatus(GLenum target);
  void DrawBuffersWEBGL(const std::vector<GLenum>& buffersArray);
  WebGLDrawBuffersExtension EXTWEBGL_draw_buffers();
  void BindVertexArrayOES(GLuint array);
  GLuint CreateVertexArrayOES();
  void DeleteVertexArrayOES(GLuint array);
  bool IsVertexArrayOES(GLuint array);
  
  void initPointers();

  #include "procs.h"
};

#endif
