import * as ngl from 'quickjs/gl/bindings.js'
const {
  QJSWebGLRenderingContext
} = ngl;

// VITATODO: remove when implemented
// process.on('exit', NativeWebGL.cleanup)
  
const gl = QJSWebGLRenderingContext.prototype
const NativeWebGLRenderingContext = QJSWebGLRenderingContext;

export { gl,  NativeWebGLRenderingContext }
