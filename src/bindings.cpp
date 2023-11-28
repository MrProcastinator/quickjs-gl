/*
 * bindings.cc
 *
 *  Created on: Dec 13, 2011
 *      Author: ngk437
 */

#include <cstdlib>
#include "webgl.h"

#include <quickjs/quickjspp.hpp>
#include <quickjs/quickjspp-arraybuffer.hpp>

static qjs::ContextContainer contextContainer;
static bool alreadyLoaded = false;

/* Bindings for complex objects */
namespace qjs
{
	template <>
	struct js_traits<WebGLActiveElementContent>
	{
		static JSValue wrap(JSContext * ctx, const WebGLActiveElementContent& element) noexcept
		{
			auto objValue = Context::get(ctx).newObject();
			objValue["size"] = element.size;
			objValue["type"] = element.type;
			objValue["name"] = element.name;
			return objValue.release();
		}

		static WebGLActiveElementContent unwrap(JSContext * ctx, JSValueConst element)
		{
			Value objValue{ctx, std::move(element)};
			return WebGLActiveElementContent(objValue["size"].as<GLsizei>(), objValue["type"].as<GLenum>(), objValue["name"].as<std::string>());
		}
	};

	template <>
	struct js_traits<WebGLShaderPrecisionFormatContent>
	{
		static JSValue wrap(JSContext * ctx, const WebGLShaderPrecisionFormatContent& format) noexcept
		{
			auto objValue = Context::get(ctx).newObject();
			objValue["rangeMin"] = format.rangeMin;
			objValue["rangeMax"] = format.rangeMax;
			objValue["precision"] = format.precision;
			return objValue.release();
		}

		static WebGLShaderPrecisionFormatContent unwrap(JSContext * ctx, JSValueConst format)
		{
			Value objValue{ctx, std::move(format)};
			return WebGLShaderPrecisionFormatContent(objValue["rangeMin"].as<GLint>(), objValue["rangeMax"].as<GLint>(), objValue["precision"].as<GLint>());
		}
	};

	template <>
	struct js_traits<WebGLDrawBuffersExtension>
	{
		static JSValue wrap(JSContext * ctx, const WebGLDrawBuffersExtension& format) noexcept
		{
			auto objValue = Context::get(ctx).newObject();
			objValue["COLOR_ATTACHMENT0_WEBGL"] = GL_COLOR_ATTACHMENT0_EXT;
			objValue["COLOR_ATTACHMENT1_WEBGL"] = GL_COLOR_ATTACHMENT1_EXT;
			objValue["COLOR_ATTACHMENT2_WEBGL"] = GL_COLOR_ATTACHMENT2_EXT;
			objValue["COLOR_ATTACHMENT3_WEBGL"] = GL_COLOR_ATTACHMENT3_EXT;
			objValue["COLOR_ATTACHMENT4_WEBGL"] = GL_COLOR_ATTACHMENT4_EXT;
			objValue["COLOR_ATTACHMENT5_WEBGL"] = GL_COLOR_ATTACHMENT5_EXT;
			objValue["COLOR_ATTACHMENT6_WEBGL"] = GL_COLOR_ATTACHMENT6_EXT;
			objValue["COLOR_ATTACHMENT7_WEBGL"] = GL_COLOR_ATTACHMENT7_EXT;
			objValue["COLOR_ATTACHMENT8_WEBGL"] = GL_COLOR_ATTACHMENT8_EXT;
			objValue["COLOR_ATTACHMENT9_WEBGL"] = GL_COLOR_ATTACHMENT9_EXT;
			objValue["COLOR_ATTACHMENT10_WEBGL"] = GL_COLOR_ATTACHMENT10_EXT;
			objValue["COLOR_ATTACHMENT11_WEBGL"] = GL_COLOR_ATTACHMENT11_EXT;
			objValue["COLOR_ATTACHMENT12_WEBGL"] = GL_COLOR_ATTACHMENT12_EXT;
			objValue["COLOR_ATTACHMENT13_WEBGL"] = GL_COLOR_ATTACHMENT13_EXT;
			objValue["COLOR_ATTACHMENT14_WEBGL"] = GL_COLOR_ATTACHMENT14_EXT;
			objValue["COLOR_ATTACHMENT15_WEBGL"] = GL_COLOR_ATTACHMENT15_EXT;
			objValue["DRAW_BUFFER0_WEBGL"] = GL_DRAW_BUFFER0_EXT;
			objValue["DRAW_BUFFER1_WEBGL"] = GL_DRAW_BUFFER1_EXT;
			objValue["DRAW_BUFFER2_WEBGL"] = GL_DRAW_BUFFER2_EXT;
			objValue["DRAW_BUFFER3_WEBGL"] = GL_DRAW_BUFFER3_EXT;
			objValue["DRAW_BUFFER4_WEBGL"] = GL_DRAW_BUFFER4_EXT;
			objValue["DRAW_BUFFER5_WEBGL"] = GL_DRAW_BUFFER5_EXT;
			objValue["DRAW_BUFFER6_WEBGL"] = GL_DRAW_BUFFER6_EXT;
			objValue["DRAW_BUFFER7_WEBGL"] = GL_DRAW_BUFFER7_EXT;
			objValue["DRAW_BUFFER8_WEBGL"] = GL_DRAW_BUFFER8_EXT;
			objValue["DRAW_BUFFER9_WEBGL"] = GL_DRAW_BUFFER9_EXT;
			objValue["DRAW_BUFFER10_WEBGL"] = GL_DRAW_BUFFER10_EXT;
			objValue["DRAW_BUFFER11_WEBGL"] = GL_DRAW_BUFFER11_EXT;
			objValue["DRAW_BUFFER12_WEBGL"] = GL_DRAW_BUFFER12_EXT;
			objValue["DRAW_BUFFER13_WEBGL"] = GL_DRAW_BUFFER13_EXT;
			objValue["DRAW_BUFFER14_WEBGL"] = GL_DRAW_BUFFER14_EXT;
			objValue["DRAW_BUFFER15_WEBGL"] = GL_DRAW_BUFFER15_EXT;
			objValue["MAX_COLOR_ATTACHMENTS_WEBGL"] = GL_MAX_COLOR_ATTACHMENTS_EXT;
			objValue["MAX_DRAW_BUFFERS_WEBGL"] = GL_MAX_DRAW_BUFFERS_EXT;
			return objValue.release();
		}

		static WebGLDrawBuffersExtension unwrap(JSContext * ctx, JSValueConst format)
		{
			// No need to unwrap
			return WebGLDrawBuffersExtension();
		}
	};
}

extern "C"
{

JSModuleDef* js_init_module_qjsc_quickjs_gl_bindings(JSContext* ctx, const char* module_name) {
  if(alreadyLoaded) {
	// Avoid duplicate loading
	return nullptr;
  }

  qjs::Context& context = contextContainer.contain(ctx);

  auto& bindingsModule = context.addModule(module_name);
  
{
  bindingsModule
    .class_<WebGLRenderingContext>("QJSWebGLRenderingContext")
      .constructor<int, int, bool, bool, bool, bool, bool, bool, bool, bool, std::optional<intptr_t>, qjs::Value>()
      /* WebGL methods */
    	.fun<&WebGLRenderingContext::DrawArraysInstanced>("_drawArraysInstanced")
    	.fun<&WebGLRenderingContext::DrawElementsInstanced>("_drawElementsInstanced")
    	.fun<&WebGLRenderingContext::VertexAttribDivisor>("_vertexAttribDivisor")

    	.fun<&WebGLRenderingContext::Swap>("swap")

    	.fun<&WebGLRenderingContext::GetUniform>("getUniform")
    	.fun<&WebGLRenderingContext::Uniform1f>("uniform1f")
    	.fun<&WebGLRenderingContext::Uniform2f>("uniform2f")
    	.fun<&WebGLRenderingContext::Uniform3f>("uniform3f")
    	.fun<&WebGLRenderingContext::Uniform4f>("uniform4f")
    	.fun<&WebGLRenderingContext::Uniform1i>("uniform1i")
    	.fun<&WebGLRenderingContext::Uniform2i>("uniform2i")
    	.fun<&WebGLRenderingContext::Uniform3i>("uniform3i")
    	.fun<&WebGLRenderingContext::Uniform4i>("uniform4i")
    	.fun<&WebGLRenderingContext::PixelStorei>("pixelStorei")
    	.fun<&WebGLRenderingContext::BindAttribLocation>("bindAttribLocation")
    	.fun<&WebGLRenderingContext::GetError>("getError")
    	.fun<&WebGLRenderingContext::DrawArrays>("drawArrays")
    	.fun<&WebGLRenderingContext::UniformMatrix2fv>("uniformMatrix2fv")
    	.fun<&WebGLRenderingContext::UniformMatrix3fv>("uniformMatrix3fv")
    	.fun<&WebGLRenderingContext::UniformMatrix4fv>("uniformMatrix4fv")
    	.fun<&WebGLRenderingContext::GenerateMipmap>("generateMipmap")
    	.fun<&WebGLRenderingContext::GetAttribLocation>("getAttribLocation")
    	.fun<&WebGLRenderingContext::DepthFunc>("depthFunc")
    	.fun<&WebGLRenderingContext::Viewport>("viewport")
    	.fun<&WebGLRenderingContext::CreateShader>("createShader")
    	.fun<&WebGLRenderingContext::ShaderSource>("shaderSource")
    	.fun<&WebGLRenderingContext::CompileShader>("compileShader")
    	.fun<&WebGLRenderingContext::GetShaderParameter>("getShaderParameter")
    	.fun<&WebGLRenderingContext::GetShaderInfoLog>("getShaderInfoLog")
    	.fun<&WebGLRenderingContext::CreateProgram>("createProgram")
    	.fun<&WebGLRenderingContext::AttachShader>("attachShader")
    	.fun<&WebGLRenderingContext::LinkProgram>("linkProgram")
    	.fun<&WebGLRenderingContext::GetProgramParameter>("getProgramParameter")
    	.fun<&WebGLRenderingContext::GetUniformLocation>("getUniformLocation")
    	.fun<&WebGLRenderingContext::ClearColor>("clearColor")
    	.fun<&WebGLRenderingContext::ClearDepth>("clearDepth")
    	.fun<&WebGLRenderingContext::Disable>("disable")
    	.fun<&WebGLRenderingContext::CreateTexture>("createTexture")
    	.fun<&WebGLRenderingContext::BindTexture>("bindTexture")
    	.fun<&WebGLRenderingContext::TexImage2D>("texImage2D")
    	.fun<&WebGLRenderingContext::TexParameteri>("texParameteri")
    	.fun<&WebGLRenderingContext::TexParameterf>("texParameterf")
    	.fun<&WebGLRenderingContext::Clear>("clear")
    	.fun<&WebGLRenderingContext::UseProgram>("useProgram")
    	.fun<&WebGLRenderingContext::CreateFramebuffer>("createFramebuffer")
    	.fun<&WebGLRenderingContext::BindFramebuffer>("bindFramebuffer")
    	.fun<&WebGLRenderingContext::FramebufferTexture2D>("framebufferTexture2D")
    	.fun<&WebGLRenderingContext::CreateBuffer>("createBuffer")
    	.fun<&WebGLRenderingContext::BindBuffer>("bindBuffer")
    	.fun<&WebGLRenderingContext::BufferData>("bufferData")
    	.fun<&WebGLRenderingContext::BufferSubData>("bufferSubData")
    	.fun<&WebGLRenderingContext::Enable>("enable")
    	.fun<&WebGLRenderingContext::BlendEquation>("blendEquation")
    	.fun<&WebGLRenderingContext::BlendFunc>("blendFunc")
    	.fun<&WebGLRenderingContext::EnableVertexAttribArray>("enableVertexAttribArray")
    	.fun<&WebGLRenderingContext::VertexAttribPointer>("vertexAttribPointer")
    	.fun<&WebGLRenderingContext::ActiveTexture>("activeTexture")
    	.fun<&WebGLRenderingContext::DrawElements>("drawElements")
    	.fun<&WebGLRenderingContext::Flush>("flush")
    	.fun<&WebGLRenderingContext::Finish>("finish")
    	.fun<&WebGLRenderingContext::VertexAttrib1f>("vertexAttrib1f")
    	.fun<&WebGLRenderingContext::VertexAttrib2f>("vertexAttrib2f")
    	.fun<&WebGLRenderingContext::VertexAttrib3f>("vertexAttrib3f")
    	.fun<&WebGLRenderingContext::VertexAttrib4f>("vertexAttrib4f")
    	.fun<&WebGLRenderingContext::BlendColor>("blendColor")
    	.fun<&WebGLRenderingContext::BlendEquationSeparate>("blendEquationSeparate")
    	.fun<&WebGLRenderingContext::BlendFuncSeparate>("blendFuncSeparate")
    	.fun<&WebGLRenderingContext::ClearStencil>("clearStencil")
    	.fun<&WebGLRenderingContext::ColorMask>("colorMask")
    	.fun<&WebGLRenderingContext::CopyTexImage2D>("copyTexImage2D")
    	.fun<&WebGLRenderingContext::CopyTexSubImage2D>("copyTexSubImage2D")
    	.fun<&WebGLRenderingContext::CullFace>("cullFace")
    	.fun<&WebGLRenderingContext::DepthMask>("depthMask")
    	.fun<&WebGLRenderingContext::DepthRange>("depthRange")
    	.fun<&WebGLRenderingContext::DisableVertexAttribArray>("disableVertexAttribArray")
    	.fun<&WebGLRenderingContext::Hint>("hint")
    	.fun<&WebGLRenderingContext::IsEnabled>("isEnabled")
    	.fun<&WebGLRenderingContext::LineWidth>("lineWidth")
    	.fun<&WebGLRenderingContext::PolygonOffset>("polygonOffset")
    	.fun<&WebGLRenderingContext::Scissor>("scissor")
    	.fun<&WebGLRenderingContext::StencilFunc>("stencilFunc")
    	.fun<&WebGLRenderingContext::StencilFuncSeparate>("stencilFuncSeparate")
    	.fun<&WebGLRenderingContext::StencilMask>("stencilMask")
    	.fun<&WebGLRenderingContext::StencilMaskSeparate>("stencilMaskSeparate")
    	.fun<&WebGLRenderingContext::StencilOp>("stencilOp")
    	.fun<&WebGLRenderingContext::StencilOpSeparate>("stencilOpSeparate")
    	.fun<&WebGLRenderingContext::BindRenderbuffer>("bindRenderbuffer")
    	.fun<&WebGLRenderingContext::CreateRenderbuffer>("createRenderbuffer")
    	.fun<&WebGLRenderingContext::DeleteBuffer>("deleteBuffer")
    	.fun<&WebGLRenderingContext::DeleteFramebuffer>("deleteFramebuffer")
    	.fun<&WebGLRenderingContext::DeleteProgram>("deleteProgram")
    	.fun<&WebGLRenderingContext::DeleteRenderbuffer>("deleteRenderbuffer")
    	.fun<&WebGLRenderingContext::DeleteShader>("deleteShader")
    	.fun<&WebGLRenderingContext::DeleteTexture>("deleteTexture")
    	.fun<&WebGLRenderingContext::DetachShader>("detachShader")
    	.fun<&WebGLRenderingContext::FramebufferRenderbuffer>("framebufferRenderbuffer")
    	.fun<&WebGLRenderingContext::GetVertexAttribOffset>("getVertexAttribOffset")
    	.fun<&WebGLRenderingContext::IsBuffer>("isBuffer")
    	.fun<&WebGLRenderingContext::IsFramebuffer>("isFramebuffer")
    	.fun<&WebGLRenderingContext::IsProgram>("isProgram")
    	.fun<&WebGLRenderingContext::IsRenderbuffer>("isRenderbuffer")
    	.fun<&WebGLRenderingContext::IsShader>("isShader")
    	.fun<&WebGLRenderingContext::IsTexture>("isTexture")
    	.fun<&WebGLRenderingContext::RenderbufferStorage>("renderbufferStorage")
    	.fun<&WebGLRenderingContext::GetShaderSource>("getShaderSource")
    	.fun<&WebGLRenderingContext::ValidateProgram>("validateProgram")
    	.fun<&WebGLRenderingContext::TexSubImage2D>("texSubImage2D")
    	.fun<&WebGLRenderingContext::ReadPixels>("readPixels")
    	.fun<&WebGLRenderingContext::GetTexParameter>("getTexParameter")
    	.fun<&WebGLRenderingContext::GetActiveAttrib>("getActiveAttrib")
    	.fun<&WebGLRenderingContext::GetActiveUniform>("getActiveUniform")
    	.fun<&WebGLRenderingContext::GetAttachedShaders>("getAttachedShaders")
    	.fun<&WebGLRenderingContext::GetParameter>("getParameter")
    	.fun<&WebGLRenderingContext::GetBufferParameter>("getBufferParameter")
    	.fun<&WebGLRenderingContext::GetFramebufferAttachmentParameter>("getFramebufferAttachmentParameter")
    	.fun<&WebGLRenderingContext::GetProgramInfoLog>("getProgramInfoLog")
    	.fun<&WebGLRenderingContext::GetRenderbufferParameter>("getRenderbufferParameter")
    	.fun<&WebGLRenderingContext::GetVertexAttrib>("getVertexAttrib")
    	.fun<&WebGLRenderingContext::GetSupportedExtensions>("getSupportedExtensions")
    	//.fun<&WebGLRenderingContext::GetExtension>("getExtension")
    	.fun<&WebGLRenderingContext::CheckFramebufferStatus>("checkFramebufferStatus")
    	.fun<&WebGLRenderingContext::GetShaderPrecisionFormat>("getShaderPrecisionFormat")
    	.fun<&WebGLRenderingContext::FrontFace>("frontFace")
    	.fun<&WebGLRenderingContext::SampleCoverage>("sampleCoverage")
    	.fun<&WebGLRenderingContext::Destroy>("destroy")
    	.fun<&WebGLRenderingContext::DrawBuffersWEBGL>("drawBuffersWEBGL")
    	.fun<&WebGLRenderingContext::EXTWEBGL_draw_buffers>("extWEBGL_draw_buffers")
    	.fun<&WebGLRenderingContext::CreateVertexArrayOES>("createVertexArrayOES")
    	.fun<&WebGLRenderingContext::DeleteVertexArrayOES>("deleteVertexArrayOES")
    	.fun<&WebGLRenderingContext::IsVertexArrayOES>("isVertexArrayOES")
    	.fun<&WebGLRenderingContext::BindVertexArrayOES>("bindVertexArrayOES")
      	
		.fun("NO_ERROR", GL_NO_ERROR)
  		.fun("INVALID_ENUM", GL_INVALID_ENUM)
  		.fun("INVALID_VALUE", GL_INVALID_VALUE)
  		.fun("INVALID_OPERATION", GL_INVALID_OPERATION)
  		.fun("OUT_OF_MEMORY", GL_OUT_OF_MEMORY)

#if SUPPORTS_GLSL_2_1 == 1
      // OpenGL ES 2.1 constants}
      .fun("DEPTH_STENCIL", GL_DEPTH_STENCIL)
      .fun("DEPTH_STENCIL_ATTACHMENT", GL_DEPTH_STENCIL_ATTACHMENT)
#endif
 
  		.fun("MAX_VERTEX_UNIFORM_VECTORS", GL_MAX_VERTEX_UNIFORM_VECTORS)
  		.fun("MAX_VARYING_VECTORS", GL_MAX_VARYING_VECTORS)
  		.fun("MAX_FRAGMENT_UNIFORM_VECTORS", GL_MAX_FRAGMENT_UNIFORM_VECTORS)
  		.fun("RGB565", GL_RGB565)
  		.fun("STENCIL_INDEX8", GL_STENCIL_INDEX8)
  		.fun("FRAMEBUFFER_INCOMPLETE_DIMENSIONS", GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
  		.fun("DEPTH_BUFFER_BIT", GL_DEPTH_BUFFER_BIT)
  		.fun("STENCIL_BUFFER_BIT", GL_STENCIL_BUFFER_BIT)
  		.fun("COLOR_BUFFER_BIT", GL_COLOR_BUFFER_BIT)
  		.fun("POINTS", GL_POINTS)
  		.fun("LINES", GL_LINES)
  		.fun("LINE_LOOP", GL_LINE_LOOP)
  		.fun("LINE_STRIP", GL_LINE_STRIP)
  		.fun("TRIANGLES", GL_TRIANGLES)
  		.fun("TRIANGLE_STRIP", GL_TRIANGLE_STRIP)
  		.fun("TRIANGLE_FAN", GL_TRIANGLE_FAN)
  		.fun("ZERO", GL_ZERO)
  		.fun("ONE", GL_ONE)
  		.fun("SRC_COLOR", GL_SRC_COLOR)
  		.fun("ONE_MINUS_SRC_COLOR", GL_ONE_MINUS_SRC_COLOR)
  		.fun("SRC_ALPHA", GL_SRC_ALPHA)
  		.fun("ONE_MINUS_SRC_ALPHA", GL_ONE_MINUS_SRC_ALPHA)
  		.fun("DST_ALPHA", GL_DST_ALPHA)
  		.fun("ONE_MINUS_DST_ALPHA", GL_ONE_MINUS_DST_ALPHA)
  		.fun("DST_COLOR", GL_DST_COLOR)
  		.fun("ONE_MINUS_DST_COLOR", GL_ONE_MINUS_DST_COLOR)
  		.fun("SRC_ALPHA_SATURATE", GL_SRC_ALPHA_SATURATE)
  		.fun("FUNC_ADD", GL_FUNC_ADD)
  		.fun("BLEND_EQUATION", GL_BLEND_EQUATION)
  		.fun("BLEND_EQUATION_RGB", GL_BLEND_EQUATION_RGB)
  		.fun("BLEND_EQUATION_ALPHA", GL_BLEND_EQUATION_ALPHA)
  		.fun("FUNC_SUBTRACT", GL_FUNC_SUBTRACT)
  		.fun("FUNC_REVERSE_SUBTRACT", GL_FUNC_REVERSE_SUBTRACT)
  		.fun("BLEND_DST_RGB", GL_BLEND_DST_RGB)
  		.fun("BLEND_SRC_RGB", GL_BLEND_SRC_RGB)
  		.fun("BLEND_DST_ALPHA", GL_BLEND_DST_ALPHA)
  		.fun("BLEND_SRC_ALPHA", GL_BLEND_SRC_ALPHA)
  		.fun("CONSTANT_COLOR", GL_CONSTANT_COLOR)
  		.fun("ONE_MINUS_CONSTANT_COLOR", GL_ONE_MINUS_CONSTANT_COLOR)
  		.fun("CONSTANT_ALPHA", GL_CONSTANT_ALPHA)
  		.fun("ONE_MINUS_CONSTANT_ALPHA", GL_ONE_MINUS_CONSTANT_ALPHA)
  		.fun("BLEND_COLOR", GL_BLEND_COLOR)
  		.fun("ARRAY_BUFFER", GL_ARRAY_BUFFER)
  		.fun("ELEMENT_ARRAY_BUFFER", GL_ELEMENT_ARRAY_BUFFER)
  		.fun("ARRAY_BUFFER_BINDING", GL_ARRAY_BUFFER_BINDING)
  		.fun("ELEMENT_ARRAY_BUFFER_BINDING", GL_ELEMENT_ARRAY_BUFFER_BINDING)
  		.fun("STREAM_DRAW", GL_STREAM_DRAW)
  		.fun("STATIC_DRAW", GL_STATIC_DRAW)
  		.fun("DYNAMIC_DRAW", GL_DYNAMIC_DRAW)
  		.fun("BUFFER_SIZE", GL_BUFFER_SIZE)
  		.fun("BUFFER_USAGE", GL_BUFFER_USAGE)
  		.fun("CURRENT_VERTEX_ATTRIB", GL_CURRENT_VERTEX_ATTRIB)
  		.fun("FRONT", GL_FRONT)
  		.fun("BACK", GL_BACK)
  		.fun("FRONT_AND_BACK", GL_FRONT_AND_BACK)
  		.fun("TEXTURE_2D", GL_TEXTURE_2D)
  		.fun("CULL_FACE", GL_CULL_FACE)
  		.fun("BLEND", GL_BLEND)
  		.fun("DITHER", GL_DITHER)
  		.fun("STENCIL_TEST", GL_STENCIL_TEST)
  		.fun("DEPTH_TEST", GL_DEPTH_TEST)
  		.fun("SCISSOR_TEST", GL_SCISSOR_TEST)
  		.fun("POLYGON_OFFSET_FILL", GL_POLYGON_OFFSET_FILL)
  		.fun("SAMPLE_ALPHA_TO_COVERAGE", GL_SAMPLE_ALPHA_TO_COVERAGE)
  		.fun("SAMPLE_COVERAGE", GL_SAMPLE_COVERAGE)
  		.fun("CW", GL_CW)
  		.fun("CCW", GL_CCW)
  		.fun("LINE_WIDTH", GL_LINE_WIDTH)
  		.fun("ALIASED_POINT_SIZE_RANGE", GL_ALIASED_POINT_SIZE_RANGE)
  		.fun("ALIASED_LINE_WIDTH_RANGE", GL_ALIASED_LINE_WIDTH_RANGE)
  		.fun("CULL_FACE_MODE", GL_CULL_FACE_MODE)
  		.fun("FRONT_FACE", GL_FRONT_FACE)
  		.fun("DEPTH_RANGE", GL_DEPTH_RANGE)
  		.fun("DEPTH_WRITEMASK", GL_DEPTH_WRITEMASK)
  		.fun("DEPTH_CLEAR_VALUE", GL_DEPTH_CLEAR_VALUE)
  		.fun("DEPTH_FUNC", GL_DEPTH_FUNC)
  		.fun("STENCIL_CLEAR_VALUE", GL_STENCIL_CLEAR_VALUE)
  		.fun("STENCIL_FUNC", GL_STENCIL_FUNC)
  		.fun("STENCIL_FAIL", GL_STENCIL_FAIL)
  		.fun("STENCIL_PASS_DEPTH_FAIL", GL_STENCIL_PASS_DEPTH_FAIL)
  		.fun("STENCIL_PASS_DEPTH_PASS", GL_STENCIL_PASS_DEPTH_PASS)
  		.fun("STENCIL_REF", GL_STENCIL_REF)
  		.fun("STENCIL_VALUE_MASK", GL_STENCIL_VALUE_MASK)
  		.fun("STENCIL_WRITEMASK", GL_STENCIL_WRITEMASK)
  		.fun("STENCIL_BACK_FUNC", GL_STENCIL_BACK_FUNC)
  		.fun("STENCIL_BACK_FAIL", GL_STENCIL_BACK_FAIL)
  		.fun("STENCIL_BACK_PASS_DEPTH_FAIL", GL_STENCIL_BACK_PASS_DEPTH_FAIL)
  		.fun("STENCIL_BACK_PASS_DEPTH_PASS", GL_STENCIL_BACK_PASS_DEPTH_PASS)
  		.fun("STENCIL_BACK_REF", GL_STENCIL_BACK_REF)
  		.fun("STENCIL_BACK_VALUE_MASK", GL_STENCIL_BACK_VALUE_MASK)
  		.fun("STENCIL_BACK_WRITEMASK", GL_STENCIL_BACK_WRITEMASK)
  		.fun("VIEWPORT", GL_VIEWPORT)
  		.fun("SCISSOR_BOX", GL_SCISSOR_BOX)
  		.fun("COLOR_CLEAR_VALUE", GL_COLOR_CLEAR_VALUE)
  		.fun("COLOR_WRITEMASK", GL_COLOR_WRITEMASK)
  		.fun("UNPACK_ALIGNMENT", GL_UNPACK_ALIGNMENT)
  		.fun("PACK_ALIGNMENT", GL_PACK_ALIGNMENT)
  		.fun("MAX_TEXTURE_SIZE", GL_MAX_TEXTURE_SIZE)
  		.fun("MAX_VIEWPORT_DIMS", GL_MAX_VIEWPORT_DIMS)
  		.fun("SUBPIXEL_BITS", GL_SUBPIXEL_BITS)
  		.fun("RED_BITS", GL_RED_BITS)
  		.fun("GREEN_BITS", GL_GREEN_BITS)
  		.fun("BLUE_BITS", GL_BLUE_BITS)
  		.fun("ALPHA_BITS", GL_ALPHA_BITS)
  		.fun("DEPTH_BITS", GL_DEPTH_BITS)
  		.fun("STENCIL_BITS", GL_STENCIL_BITS)
  		.fun("POLYGON_OFFSET_UNITS", GL_POLYGON_OFFSET_UNITS)
  		.fun("POLYGON_OFFSET_FACTOR", GL_POLYGON_OFFSET_FACTOR)
  		.fun("TEXTURE_BINDING_2D", GL_TEXTURE_BINDING_2D)
  		.fun("SAMPLE_BUFFERS", GL_SAMPLE_BUFFERS)
  		.fun("SAMPLES", GL_SAMPLES)
  		.fun("SAMPLE_COVERAGE_VALUE", GL_SAMPLE_COVERAGE_VALUE)
  		.fun("SAMPLE_COVERAGE_INVERT", GL_SAMPLE_COVERAGE_INVERT)
  		.fun("COMPRESSED_TEXTURE_FORMATS", GL_COMPRESSED_TEXTURE_FORMATS)
  		.fun("DONT_CARE", GL_DONT_CARE)
  		.fun("FASTEST", GL_FASTEST)
  		.fun("NICEST", GL_NICEST)
  		.fun("GENERATE_MIPMAP_HINT", GL_GENERATE_MIPMAP_HINT)
  		.fun("BYTE", GL_BYTE)
  		.fun("UNSIGNED_BYTE", GL_UNSIGNED_BYTE)
  		.fun("SHORT", GL_SHORT)
  		.fun("UNSIGNED_SHORT", GL_UNSIGNED_SHORT)
  		.fun("INT", GL_INT)
  		.fun("UNSIGNED_INT", GL_UNSIGNED_INT)
  		.fun("FLOAT", GL_FLOAT)
  		.fun("DEPTH_COMPONENT", GL_DEPTH_COMPONENT)
  		.fun("ALPHA", GL_ALPHA)
  		.fun("RGB", GL_RGB)
  		.fun("RGBA", GL_RGBA)
  		.fun("LUMINANCE", GL_LUMINANCE)
  		.fun("LUMINANCE_ALPHA", GL_LUMINANCE_ALPHA)
  		.fun("UNSIGNED_SHORT_4_4_4_4", GL_UNSIGNED_SHORT_4_4_4_4)
  		.fun("UNSIGNED_SHORT_5_5_5_1", GL_UNSIGNED_SHORT_5_5_5_1)
  		.fun("UNSIGNED_SHORT_5_6_5", GL_UNSIGNED_SHORT_5_6_5)
  		.fun("FRAGMENT_SHADER", GL_FRAGMENT_SHADER)
  		.fun("VERTEX_SHADER", GL_VERTEX_SHADER)
  		.fun("MAX_VERTEX_ATTRIBS", GL_MAX_VERTEX_ATTRIBS)
  		.fun("MAX_COMBINED_TEXTURE_IMAGE_UNITS", GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
  		.fun("MAX_VERTEX_TEXTURE_IMAGE_UNITS", GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS)
  		.fun("MAX_TEXTURE_IMAGE_UNITS", GL_MAX_TEXTURE_IMAGE_UNITS)
  		.fun("SHADER_TYPE", GL_SHADER_TYPE)
  		.fun("DELETE_STATUS", GL_DELETE_STATUS)
  		.fun("LINK_STATUS", GL_LINK_STATUS)
  		.fun("VALIDATE_STATUS", GL_VALIDATE_STATUS)
  		.fun("ATTACHED_SHADERS", GL_ATTACHED_SHADERS)
  		.fun("ACTIVE_UNIFORMS", GL_ACTIVE_UNIFORMS)
  		.fun("ACTIVE_ATTRIBUTES", GL_ACTIVE_ATTRIBUTES)
  		.fun("SHADING_LANGUAGE_VERSION", GL_SHADING_LANGUAGE_VERSION)
  		.fun("CURRENT_PROGRAM", GL_CURRENT_PROGRAM)
  		.fun("NEVER", GL_NEVER)
  		.fun("LESS", GL_LESS)
  		.fun("EQUAL", GL_EQUAL)
  		.fun("LEQUAL", GL_LEQUAL)
  		.fun("GREATER", GL_GREATER)
  		.fun("NOTEQUAL", GL_NOTEQUAL)
  		.fun("GEQUAL", GL_GEQUAL)
  		.fun("ALWAYS", GL_ALWAYS)
  		.fun("KEEP", GL_KEEP)
  		.fun("REPLACE", GL_REPLACE)
  		.fun("INCR", GL_INCR)
  		.fun("DECR", GL_DECR)
  		.fun("INVERT", GL_INVERT)
  		.fun("INCR_WRAP", GL_INCR_WRAP)
  		.fun("DECR_WRAP", GL_DECR_WRAP)
  		.fun("VENDOR", GL_VENDOR)
  		.fun("RENDERER", GL_RENDERER)
  		.fun("NEAREST", GL_NEAREST)
  		.fun("LINEAR", GL_LINEAR)
  		.fun("NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST)
  		.fun("LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST)
  		.fun("NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR)
  		.fun("LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR)
  		.fun("TEXTURE_MAG_FILTER", GL_TEXTURE_MAG_FILTER)
  		.fun("TEXTURE_MIN_FILTER", GL_TEXTURE_MIN_FILTER)
  		.fun("TEXTURE_WRAP_S", GL_TEXTURE_WRAP_S)
  		.fun("TEXTURE_WRAP_T", GL_TEXTURE_WRAP_T)
  		.fun("TEXTURE", GL_TEXTURE)
  		.fun("TEXTURE_CUBE_MAP", GL_TEXTURE_CUBE_MAP)
  		.fun("TEXTURE_BINDING_CUBE_MAP", GL_TEXTURE_BINDING_CUBE_MAP)
  		.fun("TEXTURE_CUBE_MAP_POSITIVE_X", GL_TEXTURE_CUBE_MAP_POSITIVE_X)
  		.fun("TEXTURE_CUBE_MAP_NEGATIVE_X", GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
  		.fun("TEXTURE_CUBE_MAP_POSITIVE_Y", GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
  		.fun("TEXTURE_CUBE_MAP_NEGATIVE_Y", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
  		.fun("TEXTURE_CUBE_MAP_POSITIVE_Z", GL_TEXTURE_CUBE_MAP_POSITIVE_Z)
  		.fun("TEXTURE_CUBE_MAP_NEGATIVE_Z", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
  		.fun("MAX_CUBE_MAP_TEXTURE_SIZE", GL_MAX_CUBE_MAP_TEXTURE_SIZE)
  		.fun("TEXTURE0", GL_TEXTURE0)
  		.fun("TEXTURE1", GL_TEXTURE1)
  		.fun("TEXTURE2", GL_TEXTURE2)
  		.fun("TEXTURE3", GL_TEXTURE3)
  		.fun("TEXTURE4", GL_TEXTURE4)
  		.fun("TEXTURE5", GL_TEXTURE5)
  		.fun("TEXTURE6", GL_TEXTURE6)
  		.fun("TEXTURE7", GL_TEXTURE7)
  		.fun("TEXTURE8", GL_TEXTURE8)
  		.fun("TEXTURE9", GL_TEXTURE9)
  		.fun("TEXTURE10", GL_TEXTURE10)
  		.fun("TEXTURE11", GL_TEXTURE11)
  		.fun("TEXTURE12", GL_TEXTURE12)
  		.fun("TEXTURE13", GL_TEXTURE13)
  		.fun("TEXTURE14", GL_TEXTURE14)
  		.fun("TEXTURE15", GL_TEXTURE15)
  		.fun("TEXTURE16", GL_TEXTURE16)
  		.fun("TEXTURE17", GL_TEXTURE17)
  		.fun("TEXTURE18", GL_TEXTURE18)
  		.fun("TEXTURE19", GL_TEXTURE19)
  		.fun("TEXTURE20", GL_TEXTURE20)
  		.fun("TEXTURE21", GL_TEXTURE21)
  		.fun("TEXTURE22", GL_TEXTURE22)
  		.fun("TEXTURE23", GL_TEXTURE23)
  		.fun("TEXTURE24", GL_TEXTURE24)
  		.fun("TEXTURE25", GL_TEXTURE25)
  		.fun("TEXTURE26", GL_TEXTURE26)
  		.fun("TEXTURE27", GL_TEXTURE27)
  		.fun("TEXTURE28", GL_TEXTURE28)
  		.fun("TEXTURE29", GL_TEXTURE29)
  		.fun("TEXTURE30", GL_TEXTURE30)
  		.fun("TEXTURE31", GL_TEXTURE31)
  		.fun("ACTIVE_TEXTURE", GL_ACTIVE_TEXTURE)
  		.fun("REPEAT", GL_REPEAT)
  		.fun("CLAMP_TO_EDGE", GL_CLAMP_TO_EDGE)
  		.fun("MIRRORED_REPEAT", GL_MIRRORED_REPEAT)
  		.fun("FLOAT_VEC2", GL_FLOAT_VEC2)
  		.fun("FLOAT_VEC3", GL_FLOAT_VEC3)
  		.fun("FLOAT_VEC4", GL_FLOAT_VEC4)
  		.fun("INT_VEC2", GL_INT_VEC2)
  		.fun("INT_VEC3", GL_INT_VEC3)
  		.fun("INT_VEC4", GL_INT_VEC4)
  		.fun("BOOL", GL_BOOL)
  		.fun("BOOL_VEC2", GL_BOOL_VEC2)
  		.fun("BOOL_VEC3", GL_BOOL_VEC3)
  		.fun("BOOL_VEC4", GL_BOOL_VEC4)
  		.fun("FLOAT_MAT2", GL_FLOAT_MAT2)
  		.fun("FLOAT_MAT3", GL_FLOAT_MAT3)
  		.fun("FLOAT_MAT4", GL_FLOAT_MAT4)
  		.fun("SAMPLER_2D", GL_SAMPLER_2D)
  		.fun("SAMPLER_CUBE", GL_SAMPLER_CUBE)
  		.fun("VERTEX_ATTRIB_ARRAY_ENABLED", GL_VERTEX_ATTRIB_ARRAY_ENABLED)
  		.fun("VERTEX_ATTRIB_ARRAY_SIZE", GL_VERTEX_ATTRIB_ARRAY_SIZE)
  		.fun("VERTEX_ATTRIB_ARRAY_STRIDE", GL_VERTEX_ATTRIB_ARRAY_STRIDE)
  		.fun("VERTEX_ATTRIB_ARRAY_TYPE", GL_VERTEX_ATTRIB_ARRAY_TYPE)
  		.fun("VERTEX_ATTRIB_ARRAY_NORMALIZED", GL_VERTEX_ATTRIB_ARRAY_NORMALIZED)
  		.fun("VERTEX_ATTRIB_ARRAY_POINTER", GL_VERTEX_ATTRIB_ARRAY_POINTER)
  		.fun("VERTEX_ATTRIB_ARRAY_BUFFER_BINDING", GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING)
  		.fun("COMPILE_STATUS", GL_COMPILE_STATUS)
  		.fun("LOW_FLOAT", GL_LOW_FLOAT)
  		.fun("MEDIUM_FLOAT", GL_MEDIUM_FLOAT)
  		.fun("HIGH_FLOAT", GL_HIGH_FLOAT)
  		.fun("LOW_INT", GL_LOW_INT)
  		.fun("MEDIUM_INT", GL_MEDIUM_INT)
  		.fun("HIGH_INT", GL_HIGH_INT)
  		.fun("FRAMEBUFFER", GL_FRAMEBUFFER)
  		.fun("RENDERBUFFER", GL_RENDERBUFFER)
  		.fun("RGBA4", GL_RGBA4)
  		.fun("RGB5_A1", GL_RGB5_A1)
  		.fun("DEPTH_COMPONENT16", GL_DEPTH_COMPONENT16)
  		.fun("RENDERBUFFER_WIDTH", GL_RENDERBUFFER_WIDTH)
  		.fun("RENDERBUFFER_HEIGHT", GL_RENDERBUFFER_HEIGHT)
  		.fun("RENDERBUFFER_INTERNAL_FORMAT", GL_RENDERBUFFER_INTERNAL_FORMAT)
  		.fun("RENDERBUFFER_RED_SIZE", GL_RENDERBUFFER_RED_SIZE)
  		.fun("RENDERBUFFER_GREEN_SIZE", GL_RENDERBUFFER_GREEN_SIZE)
  		.fun("RENDERBUFFER_BLUE_SIZE", GL_RENDERBUFFER_BLUE_SIZE)
  		.fun("RENDERBUFFER_ALPHA_SIZE", GL_RENDERBUFFER_ALPHA_SIZE)
  		.fun("RENDERBUFFER_DEPTH_SIZE", GL_RENDERBUFFER_DEPTH_SIZE)
  		.fun("RENDERBUFFER_STENCIL_SIZE", GL_RENDERBUFFER_STENCIL_SIZE)
  		.fun("FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE", GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE)
  		.fun("FRAMEBUFFER_ATTACHMENT_OBJECT_NAME", GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME)
  		.fun("FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL", GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL)
  		.fun("FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE", GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE)
  		.fun("COLOR_ATTACHMENT0", GL_COLOR_ATTACHMENT0)
  		.fun("DEPTH_ATTACHMENT", GL_DEPTH_ATTACHMENT)
  		.fun("STENCIL_ATTACHMENT", GL_STENCIL_ATTACHMENT)
  		.fun("NONE", GL_NONE)
  		.fun("FRAMEBUFFER_COMPLETE", GL_FRAMEBUFFER_COMPLETE)
  		.fun("FRAMEBUFFER_INCOMPLETE_ATTACHMENT", GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
  		.fun("FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT", GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
  		.fun("FRAMEBUFFER_UNSUPPORTED", GL_FRAMEBUFFER_UNSUPPORTED)
  		.fun("FRAMEBUFFER_BINDING", GL_FRAMEBUFFER_BINDING)
  		.fun("RENDERBUFFER_BINDING", GL_RENDERBUFFER_BINDING)
  		.fun("MAX_RENDERBUFFER_SIZE", GL_MAX_RENDERBUFFER_SIZE)
  		.fun("INVALID_FRAMEBUFFER_OPERATION", GL_INVALID_FRAMEBUFFER_OPERATION)

      	// WebGL-specific enums
  		.fun("STENCIL_INDEX", 0x1901)
  		.fun("UNPACK_FLIP_Y_WEBGL", 0x9240)
  		.fun("UNPACK_PREMULTIPLY_ALPHA_WEBGL", 0x9241)
  		.fun("CONTEXT_LOST_WEBGL", 0x9242)
  		.fun("UNPACK_COLORSPACE_CONVERSION_WEBGL", 0x9243)
  		.fun("BROWSER_DEFAULT_WEBGL", 0x9244)
  		.fun("VERSION", 0x1F02)
  		.fun("IMPLEMENTATION_COLOR_READ_TYPE", 0x8B9A)
  		.fun("IMPLEMENTATION_COLOR_READ_FORMAT", 0x8B9B)
  
  	  	//Export helper methods for clean up and error handling
		.static_fun<&WebGLRenderingContext::DisposeAll>("cleanup")
    	.fun<&WebGLRenderingContext::SetError>("setError")
	;
}
	alreadyLoaded = true;
	return bindingsModule.m;	
}

void js_destroy_module_qjsc_quickjs_gl_bindings() {
	contextContainer.release();
}

}