function translateError(error) {
  switch(error) {
    case 0: return 'GL_NO_ERROR'
    case 0x0500: return 'GL_INVALID_ENUM'
    case 0x0501: return 'GL_INVALID_VALUE'
    case 0x0502: return 'GL_INVALID_OPERATION'
    case 0x0505: return 'GL_OUT_OF_MEMORY'
    case 0x0506: return 'GL_INVALID_FRAMEBUFFER_OPERATION'
    default: return 'Unknown OpenGL error (' + error + ')'
  }
}

function showError(gl, operation) {
  var times = 1;
  var error = gl.getError();
  while(error) {
    console.log("--- OpenGL Error at last operation" + (operation ? " [" + operation + "] ": " ") + "#" + times + ": " + translateError(error));
    times++;
    error = gl.getError();
  }
}

function drawTriangle (gl) {
  const buffer = gl.createBuffer()
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer)
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-2, -2, -2, 4, 4, -2]), gl.STREAM_DRAW)
  gl.enableVertexAttribArray(0)
  gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 0, 0)
  gl.drawArrays(gl.TRIANGLES, 0, 3)
  gl.bindBuffer(gl.ARRAY_BUFFER, null)
  gl.disableVertexAttribArray(0)
  gl.deleteBuffer(buffer)
}

function loadShader (gl, shaderSource, shaderType) {
  const shader = gl.createShader(shaderType)
  gl.shaderSource(shader, shaderSource)
  gl.compileShader(shader)

  // Check the compile status
  const compiled = gl.getShaderParameter(shader, gl.COMPILE_STATUS)
  if (!compiled) {
    // Something went wrong during compilation; get the error
    const lastError = gl.getShaderInfoLog(shader)
    console.log("*** Error compiling shader '" + shader + "':" + lastError)
    gl.deleteShader(shader)
    return null
  }

  return shader
}

function createProgram (gl, shaders, optAttribs, optLocations) {
  const program = gl.createProgram()
  shaders.forEach(function (shader) {
    gl.attachShader(program, shader)
  })
  if (optAttribs) {
    optAttribs.forEach(function (attrib, ndx) {
      gl.bindAttribLocation(
        program,
        optLocations ? optLocations[ndx] : ndx,
        attrib)
    })
  }
  gl.linkProgram(program)

  // Check the link status
  const linked = gl.getProgramParameter(program, gl.LINK_STATUS)
  if (!linked) {
    // something went wrong with the link
    const lastError = gl.getProgramInfoLog(program)
    console.log('Error in program linking:' + lastError)

    gl.deleteProgram(program)
    return null
  }
  return program
}

function createProgramFromSources (gl, shaderSources, optAttribs, optLocations) {
  const defaultShaderType = [
    'VERTEX_SHADER',
    'FRAGMENT_SHADER'
  ]

  const shaders = []
  for (let ii = 0; ii < shaderSources.length; ++ii) {
    shaders.push(loadShader(gl, shaderSources[ii], gl[defaultShaderType[ii]]))
  }
  return createProgram(gl, shaders, optAttribs, optLocations)
}

export {
  translateError,
  showError,
  drawTriangle,
  loadShader,
  createProgram,
  createProgramFromSources
}