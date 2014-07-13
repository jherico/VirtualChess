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

  void loadMesh(const Mesh & mesh)  {
    using namespace oglplus;
    size_t attributeCount = 1;
    if (!mesh.normals.empty()) {
      attributeCount++;
    }
    if (!mesh.colors.empty()) {
      attributeCount++;
    }
    if (!mesh.texCoords.empty()) {
      attributeCount++;
    }

    GLsizei stride = attributeCount * 4 * sizeof(GLfloat);
    {
      size_t vertexCount = mesh.positions.size();
      VVec4 vertices;
      vertices.reserve(vertexCount * attributeCount);
      for (size_t i = 0; i < vertexCount; ++i) {
        vertices.push_back(mesh.positions[i]);
        if (!mesh.normals.empty()) {
          vertices.push_back(mesh.normals[i]);
        }
        if (!mesh.colors.empty()) {
          vertices.push_back(vec4(mesh.colors[i], 1));
        }
        if (!mesh.texCoords.empty()) {
          vertices.push_back(vec4(mesh.texCoords[i], 1, 1));
        }
      }
      Context().Bound(Buffer::Target::Array, vertexBuffer).Data(vertices);
    }
    Context().Bound(Buffer::Target::ElementArray, indexBuffer).Data(mesh.indices);
    elements = mesh.indices.size();

    GLsizei offset = 0;
    vao.Bind();
    vertexBuffer.Bind(Buffer::Target::Array);
    indexBuffer.Bind(Buffer::Target::ElementArray);

    // setup the vertex attribs array for the vertices
    VertexArrayAttrib(Layout::Attribute::Position).
      Pointer(3, DataType::Float, false, stride, (void*)offset).
      Enable();

    offset += (sizeof(GLfloat) * 4);
    if (!mesh.normals.empty()) {
      VertexArrayAttrib(Layout::Attribute::Normal).
        Pointer(3, DataType::Float, false, stride, (void*)offset).
        Enable();
      offset += (sizeof(GLfloat) * 4);
    }
    if (!mesh.colors.empty()) {
      VertexArrayAttrib(Layout::Attribute::Color).
        Pointer(3, DataType::Float, false, stride, (void*)offset).
        Enable();
      offset += (sizeof(GLfloat) * 4);
    }
    if (!mesh.texCoords.empty()) {
      VertexArrayAttrib(Layout::Attribute::TexCoord0).
        Pointer(2, DataType::Float, false, stride, (void*)offset).
        Enable();
      offset += (sizeof(GLfloat) * 4);
    }
    NoVertexArray().Bind();
  }
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

  /*
  static void drawColorCube(bool lit = false);
  static void drawQuad(
  const vec2 & min = vec2(-1),
  const vec2 & max = vec2(1));

  static void drawAngleTicks();
  static void draw3dGrid();
  static void draw3dVector(vec3 vec, const vec3 & col);

  

  static gl::GeometryPtr getCubeGeometry();
  static gl::GeometryPtr getWireCubeGeometry();

  static gl::GeometryPtr getQuadGeometry(
    float aspect, float size = 2.0f
  );

  static gl::GeometryPtr getQuadGeometry(
      const vec2 & min = vec2(-1),
      const vec2 & max = vec2(1),
      const vec2 & texMin = vec2(0),
      const vec2 & texMax = vec2(1)
  );
  */


  /*
  static void getImageData(
    const std::vector<unsigned char> & indata,
    uvec2 & outSize,
    std::vector<unsigned char> & outData,
    bool flip = true
  );

  static void getImageData(
    Resource resource,
    uvec2 & outSize,
    std::vector<unsigned char> & outData,
    bool flip = true
    );

  template<GLenum TextureType>
  static void getImageAsTexture(
    std::shared_ptr<gl::Texture<TextureType> > & texture,
    Resource resource,
    uvec2 & outSize,
    GLenum target = TextureType) {
    typedef gl::Texture<TextureType>
      Texture;
    typedef std::shared_ptr<gl::Texture<TextureType> >
      TexturePtr;
    typedef std::vector<unsigned char>
      Vector;
    Vector imageData;
    getImageData(resource, outSize, imageData);
    texture = TexturePtr(new Texture());
    texture->bind();
#ifdef HAVE_OPENCV
    glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
#endif
    texture->image2d(outSize, &imageData[0]);
    texture->parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture->parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    Texture::unbind();
  }
  */
  /**
  * A convenience method for loading images into textures
  * when you don't care about the dimensions of the image
  template<
    GLenum TextureType = GL_TEXTURE_2D, 
    GLenum TextureFomat = GL_RGBA8
  >
  static typename gl::Texture<TextureType, TextureFomat>::Ptr  
    getImageAsTexture(
      Resource resource,
      GLenum target = TextureType) {
    uvec2 imageSize;
    std::shared_ptr<gl::Texture<TextureType, TextureFomat> > texture;
    getImageAsTexture(texture, resource, target);
    return texture;
  }

  */

  /**
   * A convenience method for loading images into textures
   * when you don't care about the dimensions of the image
  template<
    GLenum TextureType = GL_TEXTURE_2D,
    GLenum TextureFomat = GL_RGBA8
  >
  static void getImageAsTexture(
    std::shared_ptr<gl::Texture<TextureType, TextureFomat> > & texture,
    Resource resource,
    GLenum target = TextureType) {
    uvec2 imageSize;
    getImageAsTexture(texture, resource, imageSize, target);
  }


  template<GLenum TextureType = GL_TEXTURE_2D>
  static void getImageAsGeometryAndTexture(
    Resource resource,
    gl::GeometryPtr & geometry,
    std::shared_ptr<gl::Texture<TextureType> > & texture) {

    uvec2 imageSize;
    GlUtils::getImageAsTexture(texture, resource, imageSize);
    float imageAspectRatio = aspect(imageSize);
    vec2 geometryMax(1.0f, 1.0f / imageAspectRatio);
    vec2 geometryMin = geometryMax * -1.0f;
    geometry = GlUtils::getQuadGeometry(geometryMin, geometryMax);
  }
  */

  //static const Mesh & getMesh(Resource resource);
  /*
  static void renderGeometry(
      const gl::GeometryPtr & geometry,
      gl::ProgramPtr program);

      */
  /*
  static void renderBunny();
  static void renderArtificialHorizon( float alpha = 1.0f );
  static void renderRift();

  static void renderParagraph(const std::string & str);

  static void renderString(const std::string & str, vec2 & cursor,
      float fontSize = 12.0f, Resource font =
          Resource::FONTS_INCONSOLATA_MEDIUM_SDFF);

  static void renderString(const std::string & str, vec3 & cursor,
      float fontSize = 12.0f, Resource font =
          Resource::FONTS_INCONSOLATA_MEDIUM_SDFF);

  static void tumble(const vec3 & camera = vec3(0, 0, 1));

  static vec2 quantize(const vec2 & t, float scale) {
    float width = scale * 2.0f;
    vec2 t2 = t + scale;
    vec2 t3 = floor(t2 / width) * width;
    return t3;
  }

  static void scaleRenderGrid(float scale, const vec2 & p) {
    vec2 p3 = quantize(p, scale);
    MatrixStack & mv = Stacks::modelview();
    mv.push().translate(vec3(p3.x - scale, 0, p3.y - scale)).scale(
      scale);
    GlUtils::draw3dGrid();
    mv.pop();

    mv.push().translate(vec3(p3.x - scale, 0, p3.y + scale)).scale(
      scale);
    GlUtils::draw3dGrid();
    mv.pop();

    mv.push().translate(vec3(p3.x + scale, 0, p3.y - scale)).scale(
      scale);
    GlUtils::draw3dGrid();
    mv.pop();

    mv.push().translate(vec3(p3.x + scale, 0, p3.y + scale)).scale(
      scale);
    GlUtils::draw3dGrid();
    mv.pop();
  }

  static void renderFloorGrid(const vec2 & position) {
    scaleRenderGrid(1.0, position);
    scaleRenderGrid(10.0, position);
    scaleRenderGrid(100.0, position);
  }

  static void renderFloorGrid(const vec3 & position) {
    renderFloorGrid(vec2(position.x, position.z));
  }

  static void renderFloorGrid(const mat4 & camera) {
    renderFloorGrid(vec2(camera[3].x, camera[3].z));
  }

  static void cubeRecurse(int depth = 6, float elapsed = Platform::elapsedSeconds());
  static void dancingCubes(int elements = 8, float elapsed = Platform::elapsedSeconds());
  */

  static const vec3 X_AXIS;
  static const vec3 Y_AXIS;
  static const vec3 Z_AXIS;
  static const vec3 ORIGIN;
  static const vec3 ONE;
  static const vec3 UP;
};

