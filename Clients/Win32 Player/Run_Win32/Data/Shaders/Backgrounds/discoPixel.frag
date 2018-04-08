
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 passUV = fragCoord/iResolution.xy;

  vec2 position = ((26.0f * gl_FragCoord.xy) / 1600.0f) + iTime;

  vec3 color = vec3(0.0f);
  for(int i = 0; i < 6; ++i)
  {
      vec2 a = floor(position);
      vec2 b = fract(position);

      vec4 diffuseColor = vec4(0.3f, 0.6f, 0.9f, 1.0f);
      diffuseColor.rgb += vec3(0.001f * iTime);
      vec4 w = fract((sin(7.0f * a.x + 11.0 * a.y + 0.01 * iTime) + diffuseColor) * 13.545317); // randoms

      color += w.xyz *                                 // color
             smoothstep(0.45,0.55,w.w) *               // intensity
             sqrt( 16.0*b.x*b.y*(1.0-b.x)*(1.0-b.y) ); // pattern

      //position /= 2.0; // lacunarity
      color /= 2.0; // attenuate high frequencies
  }

  //color = pow( 2.5*color, vec3(1.0,1.0,0.7) );    // contrast and color shape

  fragColor = vec4(color, 1.0);
}
