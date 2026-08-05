
uniform sampler2D u_TextureUnit;
uniform highp float u_AlphaTest;

uniform float u_Alpha;
uniform int u_BlendMode;

varying mediump vec2 v_TextureCoord;

void main( ) {
  vec4 texColor = texture2D( u_TextureUnit, v_TextureCoord );
  texColor.a = texColor.a * u_Alpha;

  if ( texColor.a <= u_AlphaTest ) discard;

  gl_FragColor = texColor;
}
