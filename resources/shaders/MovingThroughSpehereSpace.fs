#version 440
#extension GL_ARB_shading_language_include : require 
#include "/shaders/noise/cellular3.glsl"
//#include "/shaders/noise/cellular2.glsl"
uniform samplerCube cubemap;
layout (location = 3) uniform float Time = 0.0;

in vec3 texCoord;
out vec4 fragColor;

vec2 component(vec3 input) {
  float theta = atan(input.x, input.z);
  float thetaComponent = length(input.xz); //sin(phi);
  return vec2(sin(theta) * thetaComponent, input.y); // cos(phi));
}

void main (void) {
  float lower = 0.4;
  float upper = 0.75;
  float range = upper - lower;
  float multi = 1.0 / range;
  vec3 normTex = normalize(texCoord);
//  vec3 ncx = vec3(acos(normTex));
  vec2 nc = component(normTex);
  vec2 nc2 = component(normTex.yzx);
  //nc.y = component(normTex.xyz).x * 29.0;
//  nc += component(normTex.yzx);
//  nc += component(normTex.zxy);
  vec3 nc3 = vec3(nc, nc2.y) + Time / 10.0;
  vec2 noise = cellular(nc3 * 5.0);
  if (noise.x > upper) {
    fragColor = vec4(0.1, 0.3, 0.2, 1.0);
    fragColor = vec4(0.0, 0.1, 0.1, 1.0);
//    discard;
    return;
  }
  if (noise.x < lower) {
    fragColor = vec4(0.0, 0.1, 0.1, 1.0);
    return;
  }
    fragColor = vec4(vec3((noise.x - lower) *multi), 1); //texture(cubemap, texCoord);
//  fragColor = vec4(texCoord, 1);
  //fragColor = vec4(texCoord, 1);
}

