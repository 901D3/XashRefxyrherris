
attribute mediump vec2 a_Position;
attribute mediump vec2 a_TextureCoord;

varying mediump vec2 v_TextureCoord;

void main() {
  gl_Position    = vec4(a_Position, 0.0, 1.0);
  v_TextureCoord = a_TextureCoord;
}
