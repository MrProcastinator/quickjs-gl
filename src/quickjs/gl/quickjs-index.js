/* eslint-disable */

import bits from './bit-twiddle.js'
import { WebGLContextAttributes } from './webgl-context-attributes.js'
import { WebGLRenderingContext, wrapContext } from './webgl-rendering-context.js'
import { WebGLTextureUnit } from './webgl-texture-unit.js'
import { WebGLVertexArrayObjectState, WebGLVertexArrayGlobalState } from './webgl-vertex-attribute.js'

let CONTEXT_COUNTER = 0

// WebGL context init features
function applyTextureUnits(ctx) {
  // Initialize texture units
  const numTextures = ctx.getParameter(ctx.MAX_COMBINED_TEXTURE_IMAGE_UNITS)
  ctx._textureUnits = new Array(numTextures)
  for (let i = 0; i < numTextures; ++i) {
    ctx._textureUnits[i] = new WebGLTextureUnit(i)
  }
  ctx._activeTextureUnit = 0
  ctx.activeTexture(ctx.TEXTURE0)
}

function applyAllocateDrawingBuffer(ctx, args) {
  const hasWindow = (args && args[0]) || false
  ctx._allocateDrawingBuffer(width, height, hasWindow)
}

function applyDefaultBindings(ctx) {
  ctx.bindBuffer(ctx.ARRAY_BUFFER, null)
  ctx.bindBuffer(ctx.ELEMENT_ARRAY_BUFFER, null)
  ctx.bindFramebuffer(ctx.FRAMEBUFFER, null)
  ctx.bindRenderbuffer(ctx.RENDERBUFFER, null)
}

function applyViewport(ctx) {
  ctx.viewport(0, 0, width, height)
}

function applyScissor(ctx) {
  ctx.scissor(0, 0, width, height)
}

function applyClearBuffer(ctx) {
  ctx.clearDepth(1)
  ctx.clearColor(0, 0, 0, 0)
  ctx.clearStencil(0)
  ctx.clear(ctx.COLOR_BUFFER_BIT | ctx.DEPTH_BUFFER_BIT | ctx.STENCIL_BUFFER_BIT)
}

function applyNothing(ctx) {
  // Do nothing
}

// Initialization features supported by 
const PlatformFeature = {
  TEXTURE_UNITS: 0,
  ALLOCATE_DRAWING_BUFFER: 1,
  DEFAULT_BINDINGS: 2,
  VIEWPORT: 3,
  SCISSOR: 4,
  CLEAR_BUFFER: 5
}

const _platformSupportDefault = {
  [PlatformFeature.TEXTURE_UNITS]: applyTextureUnits,
  [PlatformFeature.ALLOCATE_DRAWING_BUFFER]: applyAllocateDrawingBuffer,
  [PlatformFeature.DEFAULT_BINDINGS]: applyDefaultBindings,
  [PlatformFeature.VIEWPORT]: applyViewport,
  [PlatformFeature.SCISSOR]: applyScissor,
  [PlatformFeature.CLEAR_BUFFER]: applyClearBuffer,
}

const _platformSupportVita = {
  [PlatformFeature.TEXTURE_UNITS]: applyNothing,
  [PlatformFeature.ALLOCATE_DRAWING_BUFFER]: function(ctx) {
    ctx._allocateDrawingBuffer(width, height, true, false)
  },
  [PlatformFeature.DEFAULT_BINDINGS]: applyNothing,
  [PlatformFeature.VIEWPORT]: applyViewport,
  [PlatformFeature.SCISSOR]: applyScissor,
  [PlatformFeature.CLEAR_BUFFER]: function(ctx) {
    ctx.clearColor(0, 0, 0, 0)
    ctx.clear(ctx.COLOR_BUFFER_BIT)
  },
}

const _platformSupport = {
  "vita": _platformSupportVita,
  "linux": _platformSupportDefault,
  "default": _platformSupportDefault
}

function applyFeature(platform, feature, ctx, ...args) {
  platform = platform || "default"
  console.log("Applying feature: " + feature  + " for platform: " + platform)
  return _platformSupport[platform][feature](ctx, args)
}

function flag (options, name, dflt) {
  if (!options || !(typeof options === 'object') || !(name in options)) {
    return dflt
  }
  return !!options[name]
}

function createContext (width, height, options) {
  width = width | 0
  height = height | 0

  if (!(width > 0 && height > 0)) {
    return null
  }

  const contextAttributes = new WebGLContextAttributes(
    flag(options, 'alpha', true),
    flag(options, 'depth', true),
    flag(options, 'stencil', false),
    false, // flag(options, 'antialias', true),
    flag(options, 'premultipliedAlpha', true),
    flag(options, 'preserveDrawingBuffer', false),
    flag(options, 'preferLowPowerToHighPerformance', false),
    flag(options, 'failIfMajorPerformanceCaveat', false))

  // Can only use premultipliedAlpha if alpha is set
  contextAttributes.premultipliedAlpha =
    contextAttributes.premultipliedAlpha && contextAttributes.alpha

  const window = options && options.window
  const hasWindow = Boolean(window)
  const platformParameters = options && options.platform
  const currentPlatform = platformParameters && platformParameters.name

  let ctx
  try {
    ctx = new WebGLRenderingContext(
      1,
      1,
      contextAttributes.alpha,
      contextAttributes.depth,
      contextAttributes.stencil,
      contextAttributes.antialias,
      contextAttributes.premultipliedAlpha,
      contextAttributes.preserveDrawingBuffer,
      contextAttributes.preferLowPowerToHighPerformance,
      contextAttributes.failIfMajorPerformanceCaveat,
      window,
      platformParameters)
  } catch (e) {}
  if (!ctx) {
    return null
  }

  ctx.drawingBufferWidth = width
  ctx.drawingBufferHeight = height

  ctx._ = CONTEXT_COUNTER++

  ctx._contextAttributes = contextAttributes

  ctx._extensions = {}
  ctx._programs = {}
  ctx._shaders = {}
  ctx._buffers = {}
  ctx._textures = {}
  ctx._framebuffers = {}
  ctx._renderbuffers = {}

  ctx._activeProgram = null
  ctx._activeFramebuffer = null
  ctx._activeRenderbuffer = null
  ctx._checkStencil = false
  ctx._stencilState = true

  applyFeature(currentPlatform, PlatformFeature.TEXTURE_UNITS, ctx)

  ctx._errorStack = []

  // Vertex array attributes that are in vertex array objects.
  ctx._defaultVertexObjectState = new WebGLVertexArrayObjectState(ctx)
  ctx._vertexObjectState = ctx._defaultVertexObjectState

  // Vertex array attibures that are not in vertex array objects.
  ctx._vertexGlobalState = new WebGLVertexArrayGlobalState(ctx)

  // Store limits
  ctx._maxTextureSize = ctx.getParameter(ctx.MAX_TEXTURE_SIZE)
  ctx._maxTextureLevel = bits.log2(bits.nextPow2(ctx._maxTextureSize))
  ctx._maxCubeMapSize = ctx.getParameter(ctx.MAX_CUBE_MAP_TEXTURE_SIZE)
  ctx._maxCubeMapLevel = bits.log2(bits.nextPow2(ctx._maxCubeMapSize))

  // Unpack alignment
  ctx._unpackAlignment = 4
  ctx._packAlignment = 4

  // Allocate framebuffer
  applyFeature(currentPlatform, PlatformFeature.ALLOCATE_DRAWING_BUFFER, ctx, hasWindow)

  const attrib0Buffer = ctx.createBuffer()
  ctx._attrib0Buffer = attrib0Buffer

  // Initialize defaults
  applyFeature(currentPlatform, PlatformFeature.DEFAULT_BINDINGS, ctx)
  
  // Set viewport and scissor
  applyFeature(currentPlatform, PlatformFeature.VIEWPORT, ctx)
  applyFeature(currentPlatform, PlatformFeature.SCISSOR, ctx)

  // Clear buffers
  applyFeature(currentPlatform, PlatformFeature.CLEAR_BUFFER, ctx)

  // TODO: uncomment when Object.keys(proto) is supported
  // return hasWindow ? ctx : wrapContext(ctx)
  return ctx
}

export { createContext }
