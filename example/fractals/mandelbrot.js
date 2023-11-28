/* global __line */
import createContext from 'quickjs/gl.js'
import * as utils from '../common/utils.js'
import * as utilsLog from '../common/utils_log.js'
const log = new utilsLog.Log('mandelbrot', 'DEBUG')

function main () {
  // Create context
  const gl = createContext(width, height, {
    window: windowPtr,
    platform: {
      msaa: 0 // For PSVita
    }
  })
  utils.showError(gl, "context creation")

  const vertexSrc = `
  attribute vec2 a_position;
  void main() {
    gl_Position = vec4(a_position,0,1);
  }
  `

  const fragmentSrc = `
  precision mediump float;
  const int max_iterations = 255;

  vec2 complex_square( vec2 v ) {
    return vec2(
      v.x * v.x - v.y * v.y,
      v.x * v.y * 2.0
    );
  }

  void main()
  {
    vec2 uv = gl_FragCoord.xy - vec2(512.0,512.0) * 0.5;
    uv *= 2.5 / min( 512.0, 512.0 );

#if 0 // Mandelbrot
    vec2 c = uv;
    vec2 v = vec2( 0.0 );
    float scale = 0.06;
#else // Julia
    vec2 c = vec2( 0.285, 0.01 );
    vec2 v = uv;
    float scale = 0.01;
#endif

    int count = max_iterations;

    for ( int i = 0 ; i < max_iterations; i++ ) {
      v = c + complex_square( v );
      if ( dot( v, v ) > 4.0 ) {
        count = i;
        break;
      }
    }

    gl_FragColor = vec4( float( count ) * scale );
  }
  `

  // setup a GLSL program
  const program = utils.createProgramFromSources(gl, [vertexSrc, fragmentSrc])
  utils.showError(gl, "utils.createProgramFromSources")

  if (!program) {
    console.log("gl program fail!!!");
    return
  }
  gl.useProgram(program)

  // look up where the vertex data needs to go.
  const positionLocation = gl.getAttribLocation(program, 'a_position')

  // Create a buffer and put a single clipspace rectangle in
  // it (2 triangles)
  const buffer = gl.createBuffer()
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer)
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
  gl.enableVertexAttribArray(positionLocation)
  gl.vertexAttribPointer(positionLocation, 2, gl.FLOAT, false, 0, 0)

  // draw
  gl.drawArrays(gl.TRIANGLES, 0, 6)

  gl.swap()

  nativeLoop()

  gl.destroy()
}

main()
