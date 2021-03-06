#version 440

layout (location = 0) uniform mat4 Projection = mat4(1);
layout (location = 1) uniform mat4 ModelView = mat4(1);

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord0;
layout(location = 3) in vec4 Color;

out vec2 vTexCoord;
out vec4 vColor;

void main() {
  gl_Position = Projection * ModelView * vec4(Position, 1);
  vTexCoord = TexCoord0;
  vColor = Color;
}
