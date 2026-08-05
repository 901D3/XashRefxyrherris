
uniform sampler2D u_TextureUnit;

// our wash color passed from cpu
uniform mediump vec4 u_WashColor;

// passed over from vertex shader
varying mediump vec2 v_TextureCoord;

uniform int u_BlendMode;

void main( ) {
  // get texture pixel
  mediump vec4 pixel = texture2D( u_TextureUnit, v_TextureCoord );

  if ( u_BlendMode == 0 )
    gl_FragColor = pixel;
  else
    gl_FragColor = pixel * u_WashColor;
}
