/************************************************************************************

 Authors     :   Bradley Austin Davis <bdavis@saintandreas.org>
 Copyright   :   Copyright Brad Davis. All Rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 ************************************************************************************/

#pragma once

#include <vector>
#include <string>


struct Light {
  vec3 position;
  vec4 color;

  Light(const vec3 & position = vec3(1), const vec4 & color = vec4(1)) {
    this->position = position;
    this->color = color;
  }
};

class Lights {
public:
  std::vector<vec3> lightPositions;
  std::vector<vec4> lightColors;
  vec4 ambient;

  // Singleton class
  Lights()
      : ambient(glm::vec4(0.2, 0.2, 0.2, 1.0)) {
    addLight();
  }

  void addLight(const glm::vec3 & position = vec3(1),
      const vec4 & color = glm::vec4(1)) {
    lightPositions.push_back(position);
    lightColors.push_back(color);
  }

  void addLight(const Light & light) {
    addLight(light.position, light.color);
  }

  void setAmbient(const glm::vec4 & ambient) {
    this->ambient = ambient;
  }
};


namespace Layout {
  namespace Attribute {
    enum {
      Position = 0,
      TexCoord0 = 1,
      Normal = 2,
      Color = 3,
      TexCoord1 = 4,
      InstanceTransform = 5,
    };
  }

  namespace Uniform {
    enum {
      Projection = 0,
      ModelView = 1,
      NormalMatrix = 2,
      Time = 3,
      Color = 4,
      LightAmbient = 8,
      LightCount = 9,
      ForceAlpha = 10,
      LightPosition = 16,
      LightColor = 24,
    };
  }
}

#define SET_UNIFORM(p, u, t, v) \
  oglplus::Uniform<t>(p, Layout::Uniform::u).Set(v)

#define SET_PROJECTION(program) \
  SET_UNIFORM(program, Projection, mat4, Stacks::projection().top())

#define SET_MODELVIEW(program) \
  SET_UNIFORM(program, ModelView, mat4, Stacks::modelview().top())

#define SET_LIGHTS(program, lights) \
  SET_UNIFORM(program, LightAmbient, vec4, lights.ambient); \
  SET_UNIFORM(program, LightCount, int, lights.lightPositions.size()); \
  for(size_t  i = 0; i < lights.lightPositions.size(); ++i) { \
    SET_UNIFORM(program, LightPosition + i, vec3, lights.lightPositions[i]); \
    SET_UNIFORM(program, LightColor + i, vec4, lights.lightColors[i]); \
  }



class Geometry {
public:
  typedef std::vector<vec4> VVec4;

  static const int VERTEX_ATTRIBUTE_SIZE = 4;
  static const int BYTES_PER_ATTRIBUTE = (sizeof(float) * VERTEX_ATTRIBUTE_SIZE);

  oglplus::Buffer vertexBuffer;
  oglplus::Buffer indexBuffer;
  oglplus::VertexArray vao;

  GLsizei elements;
  GLenum  elementType{ GL_TRIANGLES };

  Geometry() {
  }

  template <typename Function>
  Geometry(Function func, GLsizei count, GLenum elementType = GL_TRIANGLES) : elements(count), elementType(elementType) {
    func(vertexBuffer, indexBuffer, vao);
  }
  void bind() {
    vao.Bind();
  }

  void draw() {
    glDrawElements(elementType, elements, GL_UNSIGNED_INT, (void*)0);
  }

  void drawInstanced(int count) {
    glDrawElementsInstanced(elementType, elements, GL_UNSIGNED_INT, (void*)0, count);
  }

  void loadMesh(const Mesh & mesh);
};


class GlUtils {
public:
  static oglplus::Context & context();

  static oglplus::Texture & getCubemapTexture(
    Resource resource);

  static oglplus::Program & getProgram(
    Resource vertexResource,
    Resource fragmentResource);

  static Geometry & getColorCubeGeometry();
  static Geometry & getChessBoardGeometry();

  static Geometry & getGeometry(Resource resource);
  static void getCubeVertices(oglplus::Buffer & dest);

  static void getCubeIndices(oglplus::Buffer & dest);

  static void getCubeWireIndices(oglplus::Buffer & dest);

  static const float CHESS_SCALE;


  static const vec3 X_AXIS;
  static const vec3 Y_AXIS;
  static const vec3 Z_AXIS;
  static const vec3 ORIGIN;
  static const vec3 ONE;
  static const vec3 UP;
};

