void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Output to screen
    fragColor = vec4(iFrame / 256.0f, iFrame / 512.0f, iFrame / 1024.0f, 1.0);
}
