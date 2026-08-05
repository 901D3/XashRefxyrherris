
attribute mediump vec3 a_Position;
attribute mediump vec2 a_TextureCoord;

uniform mediump mat4 u_MVP;

varying mediump vec2 v_TextureCoord;

void main( ) {
  v_TextureCoord = a_TextureCoord;

  gl_Position = u_MVP * vec4( a_Position, 1.0 );
}
