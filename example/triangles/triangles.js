import createContext from 'quickjs/gl.js'
import { createProgramFromSources, showError } from '../common/utils.js'

function main () {
  // Create context
  console.log("Creating context with size: " + width + "x" + height + "...")
  const gl = createContext(width, height, {
    window: windowPtr,
    platform: {
      msaa: 0, // For PSVita
      name: platformName
    }
  })

  if(gl == null) {
    console.log("gl is null!!!");
    return;
  } else {
    console.log("gl context created!!!");
  }

  showError(gl, "context creation")

  const vertexSrc = [
    'attribute vec2 a_position;',
    'void main() {',
    'gl_Position = vec4(a_position, 0, 1);',
    '}'
  ].join('\n')

  const fragmentSrc = [
    'void main() {',
    'gl_FragColor = vec4(0, 1, 0, 1);  // green',
    '}'
  ].join('\n')

  // setup a GLSL program
  const program = createProgramFromSources(gl, [vertexSrc, fragmentSrc])

  if (!program) {
    console.log("gl program fail!!!");
    return
  }
  gl.useProgram(program)
  showError(gl, "useProgram")

  // look up where the vertex data needs to go.
  const positionLocation = gl.getAttribLocation(program, 'a_position')
  showError(gl, "getAttribLocation")

  // Create a buffer and put a single clipspace rectangle in
  // it (2 triangles)
  const buffer = gl.createBuffer()
  showError(gl, "createBuffer")
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer)
  showError(gl, "bindBuffer")
  gl.bufferData(
    gl.ARRAY_BUFFER,
    new Float32Array([
      -1.0, -1.0,
      1.0, -1.0,
      -1.0, 1.0,
      -1.0, 1.0,
      1.0, -1.0,
      1.0, 1.0]),
    gl.STATIC_DRAW)
  showError(gl, "bufferData")
  gl.enableVertexAttribArray(positionLocation)
  showError(gl, "enableVertexAttribArray")
  gl.vertexAttribPointer(positionLocation, 2, gl.FLOAT, false, 0, 0)
  showError(gl, "vertexAttribPointer")
  
  // draw
  gl.drawArrays(gl.TRIANGLES, 0, 6)
  showError(gl, "drawArrays")

  gl.swap()

  nativeLoop()

  gl.destroy()
}

main()
