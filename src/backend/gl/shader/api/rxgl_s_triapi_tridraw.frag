
uniform sampler2D u_TextureUnit;
uniform int u_BlendMode;

varying highp vec4 v_WashColor;
varying highp vec2 v_TextureCoord;

void main( ) {
  highp float isBlendWashColor = ( u_BlendMode == 2 || u_BlendMode == 4 || u_BlendMode == 6 ) ? 1.0 : 0.0;

  highp float isBlendNormal = ( u_BlendMode == 0 ) ? 1.0 : 0.0;

  highp vec4 pixel = texture2D( u_TextureUnit, v_TextureCoord );

  gl_FragColor = isBlendNormal * pixel + isBlendWashColor * ( pixel * v_WashColor );
}
