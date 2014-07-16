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

#include "Common.h"
#include <openctmpp.h>

#include <oglplus/images/png.hpp>
#include <oglplus/images/load.hpp>

using namespace oglplus;

Context & GlUtils::context() {
  static Context ctx;
  return ctx;
}

// Some defines to make calculations below more transparent
#define TRIANGLES_PER_FACE 2
#define VERTICES_PER_TRIANGLE 3
#define VERTICES_PER_EDGE 2
#define FLOATS_PER_VERTEX 3

#define CUBE_SIZE 1.0f
#define CUBE_P (CUBE_SIZE / 2.0f)
#define CUBE_N (-1.0f * CUBE_P)

// Cube geometry
#define CUBE_VERT_COUNT 8
#define CUBE_FACE_COUNT 6
#define CUBE_EDGE_COUNT 12
#define CUBE_INDEX_COUNT (CUBE_FACE_COUNT * \
  TRIANGLES_PER_FACE * VERTICES_PER_TRIANGLE)

// Vertices for a unit cube centered at the origin
const vec4 CUBE_VERTICES[CUBE_VERT_COUNT] = { //
    vec4(CUBE_N, CUBE_N, CUBE_P, 1), // ll 0
    vec4(CUBE_P, CUBE_N, CUBE_P, 1), // lr 1
    vec4(CUBE_N, CUBE_P, CUBE_P, 1), // ul 2
    vec4(CUBE_P, CUBE_P, CUBE_P, 1), // ur 3

    vec4(CUBE_N, CUBE_N, CUBE_N, 1), // ll 4
    vec4(CUBE_P, CUBE_N, CUBE_N, 1), // lr 5
    vec4(CUBE_N, CUBE_P, CUBE_N, 1), // ul 6
    vec4(CUBE_P, CUBE_P, CUBE_N, 1), // ur 7
};

const vec3 CUBE_FACE_COLORS[] = { //
    Colors::red, Colors::green, Colors::blue, Colors::yellow, Colors::cyan,
        Colors::magenta, };

// 6 sides * 2 triangles * 3 vertices
const GLuint CUBE_INDICES[CUBE_FACE_COUNT * TRIANGLES_PER_FACE * VERTICES_PER_TRIANGLE] = { //
    0, 1, 2, 2, 1, 3, // X Positive
    2, 3, 6, 6, 3, 7, // X Negative
    6, 7, 4, 4, 7, 5, // Y Positive
    7, 3, 5, 5, 3, 1, // Y Negative
    5, 1, 4, 4, 1, 0, // Z Positive
    4, 0, 6, 6, 0, 2, // Z Negative
};

const unsigned int CUBE_WIRE_INDICES[CUBE_EDGE_COUNT * VERTICES_PER_EDGE] = { //
    0, 1, 1, 3, 3, 2, 2, 0, // square
    4, 5, 5, 7, 7, 6, 6, 4, // facing square
    0, 4, 1, 5, 2, 6, 3, 7, // transverse lines
};


const vec3 GlUtils::X_AXIS = vec3(1.0f, 0.0f, 0.0f);
const vec3 GlUtils::Y_AXIS = vec3(0.0f, 1.0f, 0.0f);
const vec3 GlUtils::Z_AXIS = vec3(0.0f, 0.0f, 1.0f);
const vec3 GlUtils::ORIGIN = vec3(0.0f, 0.0f, 0.0f);
const vec3 GlUtils::ONE = vec3(1.0f, 1.0f, 1.0f);
const vec3 GlUtils::UP = vec3(0.0f, 1.0f, 0.0f);


void GlUtils::getCubeVertices(Buffer & dest) {
  GlUtils::context().Bound(Buffer::Target::Array, dest).Data(CUBE_VERTICES);
}

void GlUtils::getCubeIndices(Buffer & dest) {
  GlUtils::context().Bound(Buffer::Target::ElementArray, dest).Data(CUBE_INDICES);
}

void GlUtils::getCubeWireIndices(Buffer & dest) {
  GlUtils::context().Bound(Buffer::Target::ElementArray, dest).Data(CUBE_WIRE_INDICES);
}

template <class T>
void compileShaders(const Resource * shaders) {
  int i = 0;
  while (shaders[i] != Resource::NO_RESOURCE) {
    SAY("Compiling %s", Resources::getResourcePath(shaders[i]).c_str());
    T shader;
    shader.Source(Platform::getResourceString(shaders[i]));
    shader.Compile();
    ++i;
  }
}

Program GlUtils::getProgram(Resource vsRes, Resource fsRes) {
  static bool shadersChecked = false;
  if (!shadersChecked) {
    shadersChecked = true;
    //compileShaders<VertexShader>(Resources::VERTEX_SHADERS);
    //compileShaders<FragmentShader>(Resources::FRAGMENT_SHADERS);
  }

  VertexShader vs;
  vs.Source(Platform::getResourceString(vsRes)).Compile();
  FragmentShader fs;
  fs.Source(Platform::getResourceString(fsRes)).Compile();

  Program prog;
  // attach the shaders to the program
  prog.AttachShader(vs);
  prog.AttachShader(fs);

  // link and use it
  prog.Link();
  return prog;
}

images::PNGImage getResourceImage(Resource res) {
  std::vector<uint8_t> vec = Platform::getResourceVector(res);
  std::stringstream stream;
  stream.write((char*)&(vec[0]), vec.size());
//  stream.read()
  return images::PNGImage(stream);
}

Texture & GlUtils::getCubemapTexture(Resource firstResource) {
  typedef std::unique_ptr<Texture> TexturePtr;
  typedef std::map<Resource, TexturePtr> Map;
  typedef Map::iterator MapItr;
  static Map skyboxMap;

  static int RESOURCE_ORDER[] = {
    1, // GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    0, // GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    3, // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    2, // GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    5, // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    4, // GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
  };

  MapItr itr = skyboxMap.find(firstResource);
  if (skyboxMap.end() != itr) {
    return *(itr->second);
  }

  Context & gl = context();
  skyboxMap[firstResource].reset(new Texture());
  Texture & texture = *(skyboxMap[firstResource]);
  auto textureBinding = gl.Bound(Texture::Target::CubeMap, texture);
  textureBinding
    .MinFilter(TextureMinFilter::Nearest)
    .MagFilter(TextureMagFilter::Nearest)
    .WrapS(TextureWrap::ClampToEdge)
    .WrapT(TextureWrap::ClampToEdge)
    .WrapR(TextureWrap::ClampToEdge);
  GL_CHECK_ERROR;

  std::vector<uint8_t> data;
  uvec2 size;
  for (int i = 0; i != 6; ++i) {
    Resource image = static_cast<Resource>(firstResource + i);
    Texture::CubeMapFace(RESOURCE_ORDER[i]) << getResourceImage(image);
    GL_CHECK_ERROR;
  }
  return texture;
}


Geometry & GlUtils::getColorCubeGeometry() {
  static Geometry cube;
  static bool initialized = false;
  if (!initialized) {
    Mesh mesh;
    vec3 move(0, 0, 0.5f);
    MatrixStack & m = mesh.model;

    m.push().rotate(glm::angleAxis(PI / 2.0f, Y_AXIS)).translate(move);
    mesh.color = Colors::red;
    mesh.addQuad(vec2(1.0));
    mesh.fillColors(true);
    m.pop();

    m.push().rotate(glm::angleAxis(-PI / 2.0f, X_AXIS)).translate(move);
    mesh.color = Colors::green;
    mesh.addQuad(vec2(1.0));
    m.pop();

    m.push().translate(move);
    mesh.color = Colors::blue;
    mesh.addQuad(vec2(1.0));
    m.pop();

    m.push().rotate(glm::angleAxis(-PI / 2.0f, Y_AXIS)).translate(move);
    mesh.color = Colors::cyan;
    mesh.addQuad(vec2(1.0));
    m.pop();

    m.push().rotate(glm::angleAxis(PI / 2.0f, X_AXIS)).translate(move);
    mesh.color = Colors::yellow;
    mesh.addQuad(vec2(1.0));
    m.pop();

    m.push().rotate(glm::angleAxis(-PI, X_AXIS)).translate(move);
    mesh.color = Colors::magenta;
    mesh.addQuad(vec2(1.0));
    m.pop();
    cube.loadMesh(mesh);
  }
  return cube;
}

const float GlUtils::CHESS_SCALE = 0.055f;

Geometry & GlUtils::getChessBoardGeometry() {
  static Geometry chess;
  static bool initialized = false;
  if (!initialized) {
    Mesh mesh;
    vec3 move(0, 0, 0.5f);
    MatrixStack & m = mesh.model;
    m.scale(CHESS_SCALE);
    m.rotate(PI / 2, X_AXIS);
    m.translate(vec3(-3.5, -3.5, 0));
    for (int x = 0; x < 8; ++x) {
      for (int y = 0; y < 8; ++y) {
        m.withPush([&]{
          m.translate(vec3(x, y, 0));
          mesh.color = (x + y) % 2 ? Colors::saddleBrown : Colors::wheat;
          mesh.addQuad(vec2(1.0));
          mesh.fillColors(true);
        });
      }
    }
    chess.loadMesh(mesh);
  }
  return chess;
}

void Geometry::loadMesh(const Mesh & mesh) {
  {
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
    NoBuffer().Bind(Buffer::Target::Array);
    NoBuffer().Bind(Buffer::Target::ElementArray);
  }
}


Geometry & GlUtils::getGeometry(Resource resource) {
  typedef std::shared_ptr<Geometry> GeometryPtr;
  typedef std::map<Resource, GeometryPtr> Map;
  static Map map;
  if (!map.count(resource)) {
    map[resource].reset(new Geometry());
    Mesh mesh; {
      CTMimporter importer;
      importer.LoadData(Platform::getResourceString(resource));
      int vertexCount = importer.GetInteger(CTM_VERTEX_COUNT);
      mesh.positions.resize(vertexCount);
      const float * ctmData = importer.GetFloatArray(CTM_VERTICES);
      for (int i = 0; i < vertexCount; ++i) {
        glm::vec4 pos(glm::make_vec3(ctmData + (i * 3)), 1);
        pos = mesh.model.top() * pos;
        pos /= pos.w;
        mesh.positions[i] = vec4(glm::make_vec3(&pos.x), 1);
      }

      if (importer.GetInteger(CTM_UV_MAP_COUNT) > 0) {
        const float * ctmData = importer.GetFloatArray(CTM_UV_MAP_1);
        mesh.texCoords.resize(vertexCount);
        for (int i = 0; i < vertexCount; ++i) {
          mesh.texCoords[i] = glm::make_vec2(ctmData + (i * 2));
        }
      }

      bool hasNormals = importer.GetInteger(CTM_HAS_NORMALS) ? true : false;
      if (hasNormals) {
        mesh.normals.resize(vertexCount);
        ctmData = importer.GetFloatArray(CTM_NORMALS);
        for (int i = 0; i < vertexCount; ++i) {
          mesh.normals[i] = vec4(glm::make_vec3(ctmData + (i * 3)), 1);
        }
      }

      int indexCount = 3 * importer.GetInteger(CTM_TRIANGLE_COUNT);
      const CTMuint * ctmIntData = importer.GetIntegerArray(CTM_INDICES);
      mesh.indices.resize(indexCount);
      for (int i = 0; i < indexCount; ++i) {
        mesh.indices[i] = *(ctmIntData + i);
      }
    }
    map[resource]->loadMesh(mesh);
  }
  return (*map[resource]);
}
