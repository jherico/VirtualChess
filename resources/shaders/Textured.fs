#version 440

uniform sampler2D sampler;
//uniform float Alpha = 1.0;

in vec2 vTexCoord;
in vec4 vColor;
out vec4 vFragColor;

void main() {
    vec4 c = texture(sampler, vTexCoord);
//    c.rgb *= vColor.rgb;
//    c.rgb *= vColor.rgb;
//    c *= vColor;
//    c.r = 1.0 - c.a;
//    c.g = 1.0 - c.a;
//    c.b = 1.0 - c.a;
//    c.a = min(Alpha, c.a);
    vFragColor = c;
}
