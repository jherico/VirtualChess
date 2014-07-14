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
  glm::vec4 position;
  glm::vec4 color;

  Light(const glm::vec4 & position = glm::vec4(1), const glm::vec4 & color =
      glm::vec4(1)) {
    this->position = position;
    this->color = color;
  }
};

class Lights {
  std::vector<glm::vec4> lightPositions;
  std::vector<glm::vec4> lightColors;
  glm::vec4 ambient;

public:
  // Singleton class
  Lights()
      : ambient(glm::vec4(0.2, 0.2, 0.2, 1.0)) {
    addLight();
  }

  void addLight(const glm::vec4 & position = glm::vec4(1),
      const glm::vec4 & color = glm::vec4(1)) {
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
    };
  }
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

  static oglplus::Program getProgram(
    Resource vertexResource,
    Resource fragmentResource);

  static Geometry & getColorCubeGeometry();
  static Geometry & getChessBoardGeometry();

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

