class STACKGLResizeDrawingBuffer {
  constructor (ctx) {
    this.resize = ctx.resize.bind(ctx)
  }
}

function getSTACKGLResizeDrawingBuffer (ctx) {
  return new STACKGLResizeDrawingBuffer(ctx)
}

export { getSTACKGLResizeDrawingBuffer, STACKGLResizeDrawingBuffer }
