#version 410 core
//Based off of https://www.shadertoy.com/view/Ml2GWy

uniform sampler2D gDiffuseTexture;
uniform float gTime;
uniform vec2 gWindowResolution;

in vec2 passUV0;
in vec3 passPosition;

out vec4 outColor;

void main(void)
{
  vec2 position = ((26.0f * gl_FragCoord.xy) / 1600.0f) + gTime;

  vec3 color = vec3(0.0f);
  for(int i = 0; i < 6; ++i)
  {
      vec2 a = floor(position);
      vec2 b = fract(position);

      vec4 diffuseColor = texture(gDiffuseTexture, passUV0);
      diffuseColor.rgb += vec3(0.001f * gTime);
      vec4 w = fract((sin(7.0f * a.x + 11.0 * a.y + 0.01 * gTime) + diffuseColor) * 13.545317); // randoms

      color += w.xyz *                                 // color
             smoothstep(0.45,0.55,w.w) *               // intensity
             sqrt( 16.0*b.x*b.y*(1.0-b.x)*(1.0-b.y) ); // pattern

      //position /= 2.0; // lacunarity
      color /= 2.0; // attenuate high frequencies
  }

  //color = pow( 2.5*color, vec3(1.0,1.0,0.7) );    // contrast and color shape

  outColor = vec4(color, 1.0);
}
