
attribute mediump vec2 a_Position;
attribute mediump vec4 a_WashColor;
attribute mediump vec2 a_TextureCoord;

varying mediump vec4 v_WashColor;
varying mediump vec2 v_TextureCoord;

void main() {
  gl_Position = vec4(a_Position, 0.0, 1.0);

  v_WashColor    = a_WashColor;
  v_TextureCoord = a_TextureCoord;
}
