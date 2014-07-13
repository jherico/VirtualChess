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
// This is a library for 3D mesh decompression
//#include <openctmpp.h>


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
  typedef std::unordered_map<Resource, Texture> Map;
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
    return itr->second;
  }

  Context & gl = context();
  Texture & texture = skyboxMap[firstResource];
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
          mesh.color = (x + y) % 2 ? Colors::wheat : Colors::saddleBrown;
          mesh.addQuad(vec2(1.0));
          mesh.fillColors(true);
        });
      }
    }
    chess.loadMesh(mesh);
  }
  return chess;
}
#if 0
#define EPSILON 0.002
void GlUtils::renderArtificialHorizon(float alpha) {
  static gl::GeometryPtr geometry;
  if (!geometry) {
    Mesh mesh;
    MatrixStack & m = mesh.getModel();

    m.push();
    {
      m.top() = translate(mat4(), vec3(0, 0, 1.01));
      mesh.getColor() = Colors::yellow;
      mesh.addQuad(0.5, 0.05f);
      mesh.fillColors(true);
      mesh.getColor() = Colors::lightGray;
      vec2 bar(0.5, 0.025);
      vec2 smallBar(0.3, 0.015);
      for (int i = 1; i <= 4; ++i) {
        float angle = i * (TWO_PI / 18.0f);
        m.identity().rotate(angle, GlUtils::X_AXIS).translate(
            GlUtils::Z_AXIS);
        mesh.addQuad(bar);
        m.identity().rotate(PI, GlUtils::Y_AXIS).rotate(angle,
            GlUtils::X_AXIS).translate(GlUtils::Z_AXIS);
        mesh.addQuad(bar);
        m.identity().rotate(PI, GlUtils::Z_AXIS).rotate(angle,
            GlUtils::X_AXIS).translate(GlUtils::Z_AXIS);
        mesh.addQuad(bar);
        m.identity().rotate(PI, GlUtils::Z_AXIS).rotate(PI,
            GlUtils::Y_AXIS).rotate(angle, GlUtils::X_AXIS).translate(
            GlUtils::Z_AXIS);
        mesh.addQuad(bar);

        angle -= (TWO_PI / 36.0f);
        m.identity().rotate(angle, GlUtils::X_AXIS).translate(
            GlUtils::Z_AXIS);
        mesh.addQuad(smallBar);
        m.identity().rotate(PI, GlUtils::Y_AXIS).rotate(angle,
            GlUtils::X_AXIS).translate(GlUtils::Z_AXIS);
        mesh.addQuad(smallBar);
        m.identity().rotate(PI, GlUtils::Z_AXIS).rotate(angle,
            GlUtils::X_AXIS).translate(GlUtils::Z_AXIS);
        mesh.addQuad(smallBar);
        m.identity().rotate(PI, GlUtils::Z_AXIS).rotate(PI,
            GlUtils::Y_AXIS).rotate(angle, GlUtils::X_AXIS).translate(
            GlUtils::Z_AXIS);
        mesh.addQuad(smallBar);
      }
    }
    m.pop();
    mesh.fillNormals(true);

    const Mesh & hemi = getMesh(Resource::MESHES_HEMI_CTM);
    m.top() = rotate(mat4(), -PI / 2.0f, GlUtils::X_AXIS);
    mesh.getColor() = Colors::cyan;
    mesh.addMesh(hemi, true);

    m.top() = rotate(mat4(), PI / 2.0f, GlUtils::X_AXIS);
    mesh.getColor() = Colors::orange;
    mesh.addMesh(hemi);
    {
      std::set<int> poleIndices;
      for (size_t i = 0; i < mesh.positions.size(); ++i) {
        const vec4 & v = mesh.positions[i];
        if (abs(v.x) < EPSILON && abs(v.z) < EPSILON) {
          poleIndices.insert(i);
        }
      }
      for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        bool black = false;
        for (size_t j = i; j < i + 3; ++j) {
          if (poleIndices.count(mesh.indices[j])) {
            black = true;
            break;
          }
        }
        if (black) {
          for (size_t j = i; j < i + 3; ++j) {
            mesh.colors[mesh.indices[j]] = Colors::grey;
          }
        }
      }
    }
    geometry = mesh.getGeometry();
  }
  gl::ProgramPtr program = getProgram(
      Resource::SHADERS_LITCOLORED_VS,
      Resource::SHADERS_LITCOLORED_FS);
  program->use();
  program->setUniform("ForceAlpha", alpha);
  renderGeometry(geometry, program);
}

void GlUtils::renderRift() {
  static gl::GeometryPtr geometry;
  if (!geometry) {
    Mesh mesh;
    mesh.getModel().rotate(-107.0f, GlUtils::X_AXIS).scale(0.5f);
    const Mesh & sourceMesh = GlUtils::getMesh(Resource::MESHES_RIFT_CTM);
    mesh.addMesh(sourceMesh);
    geometry = mesh.getGeometry();
  }
  gl::ProgramPtr program = getProgram(
      Resource::SHADERS_LIT_VS,
      Resource::SHADERS_LITCOLORED_FS);

  // The Rift model is aligned with the wrong axis, so we
  // rotate it by 90 degrees
  MatrixStack & mv = Stacks::modelview();
  mv.push().rotate(angleAxis(-HALF_PI, GlUtils::X_AXIS));
  GlUtils::renderGeometry(geometry, program);
  mv.pop();
}


void GlUtils::drawQuad(const vec2 & min, const vec2 & max) {
  using namespace gl;
  static GeometryPtr g;
  if (!g) {
    Mesh m;
    m.addVertex(vec3(min.x, max.y, 0));
    m.addVertex(vec3(min.x, min.y, 0));
    m.addVertex(vec3(max.x, max.y, 0));
    m.addVertex(vec3(max.x, min.y, 0));
    g = m.getGeometry(GL_TRIANGLE_STRIP);
  }
  ProgramPtr program = getProgram(Resource::SHADERS_SIMPLE_VS, Resource::SHADERS_COLORED_FS);
  renderGeometry(g, program);
}

void GlUtils::drawColorCube(bool lit) {
  static gl::GeometryPtr cube = getColorCubeGeometry();
  Resource fs = lit ? Resource::SHADERS_LITCOLORED_VS : Resource::SHADERS_COLORED_VS;
  Resource vs = lit ? Resource::SHADERS_LITCOLORED_FS : Resource::SHADERS_COLORED_FS;
  renderGeometry(cube, getProgram(fs, vs));
}

void GlUtils::drawAngleTicks() {
  using namespace gl;
  static GeometryPtr g;
  if (!g) {
    float offsets[] = { //
      (float) tan( PI / 6.0f), // 30 degrees
      (float) tan( PI / 4.0f), // 45 degrees
      (float) tan( PI / 3.0f) // 60 degrees
    };
    Mesh m;
    // 43.9 degrees puts tick on the inner edge of the screen
    // 42.6 degrees is the effective fov for wide screen
    // 43.9 degrees puts tick on the inner edge of the screen
    m.addVertex(vec3(-2, 0, 0));
    m.addVertex(vec3(2, 0, 0));
    m.addVertex(vec3(0, -2, 0));
    m.addVertex(vec3(0, 2, 0));
    // By keeping the camera locked at 1 unit away from the origin, all our
    // distances can be computed as tan(angle)
    for (float offset : offsets) {
      m.addVertex(vec3(offset, -0.05, 0));
      m.addVertex(vec3(offset, 0.05, 0));
      m.addVertex(vec3(-offset, -0.05, 0));
      m.addVertex(vec3(-offset, 0.05, 0));
    }
    for (float offset : offsets) {
      m.addVertex(vec3(-0.05, offset, 0));
      m.addVertex(vec3(0.05, offset, 0));
      m.addVertex(vec3(-0.05, -offset, 0));
      m.addVertex(vec3(0.05, -offset, 0));
    }
    g = m.getGeometry(GL_LINES);
  }

  // Fix the modelview at exactly 1 unit away from the origin, no rotation
  Stacks::modelview().push(mat4(1)).translate(vec3(0, 0, -1));
  ProgramPtr program = getProgram(Resource::SHADERS_SIMPLE_VS, Resource::SHADERS_COLORED_FS);
  program->use();
  renderGeometry(g, program);
  Stacks::modelview().pop();
}

void GlUtils::draw3dGrid() {
  static gl::GeometryPtr g;
  if (!g) {
    Mesh m;
    for (int i = 0; i < 5; ++i) {
      float offset = ((float) i * 0.2f) + 0.2f;
      vec3 zOffset(0, 0, offset);
      vec3 xOffset(offset, 0, 0);
      m.addVertex(-X_AXIS + zOffset);
      m.addVertex(X_AXIS + zOffset);
      m.addVertex(-X_AXIS - zOffset);
      m.addVertex(X_AXIS - zOffset);
      m.addVertex(-Z_AXIS + xOffset);
      m.addVertex(Z_AXIS + xOffset);
      m.addVertex(-Z_AXIS - xOffset);
      m.addVertex(Z_AXIS - xOffset);
    }
    m.addVertex(X_AXIS);
    m.addVertex(-X_AXIS);
    m.addVertex(Z_AXIS);
    m.addVertex(-Z_AXIS);
    g = m.getGeometry(GL_LINES);
  }
  gl::ProgramPtr program = getProgram(Resource::SHADERS_SIMPLE_VS, Resource::SHADERS_COLORED_FS);
  GL_CHECK_ERROR;
  program->use();
  GL_CHECK_ERROR;
  program->setUniform("Color", vec4(Colors::gray,1));
  GL_CHECK_ERROR;
  renderGeometry(g, program);
  GL_CHECK_ERROR;
}


void GlUtils::draw3dVector(vec3 vec, const vec3 & col) {
  Mesh m;
  m.color = Colors::gray;

  m.addVertex(vec3());
  m.addVertex(vec3(vec.x, 0, vec.z));

  m.addVertex(vec3(vec.x, 0, vec.z));
  m.addVertex(vec);

  m.fillColors(true);
  m.color = col;
  m.addVertex(vec);
  m.addVertex(vec3());

  m.fillColors();
  static gl::GeometryPtr g = m.getGeometry(GL_LINES);
  g->updateVertices(m.buildVertices());

  gl::ProgramPtr program = getProgram(
      Resource::SHADERS_COLORED_VS,
      Resource::SHADERS_COLORED_FS);
  program->use();
  renderGeometry(g, program);
  gl::Program::clear();
}

std::wstring toUtf16(const std::string & text) {
//    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wide(text.begin(), text.end()); //= converter.from_bytes(narrow.c_str());
  return wide;
}

//
//static gl::GeometryPtr line;
//if (!line) {
//  Mesh mesh;
//  mesh.color = Colors::white;
//  mesh.addVertex(vec3());
//  mesh.addVertex(GlUtils::X_AXIS * 2.0f);
//  mesh.fillColors();
//  line = mesh.getGeometry(GL_LINES);
//}
// Draw a line at the cursor
//{
//  gl::ProgramPtr lineProgram = GlUtils::getProgram(
//    Resource::SHADERS_SIMPLE_VS,
//    Resource::SHADERS_SIMPLE_FS);
//  renderGeometry(line, lineProgram);
//}

void GlUtils::renderString(const std::string & cstr, vec2 & cursor,
    float fontSize, Resource fontResource) {
  getFont(fontResource)->renderString(toUtf16(cstr), cursor, fontSize);
}

void GlUtils::renderParagraph(const std::string & str) {
  vec2 cursor;
  Text::FontPtr font = getFont(Resource::FONTS_INCONSOLATA_MEDIUM_SDFF);
  rectf bounds;
  std::wstring wstr = toUtf16(str);
  for (size_t i = 0; i < wstr.length(); ++i) {
    uint16_t wchar = wstr.at(i);
    rectf letterBound = font->getBounds(wchar);
//    extendLeft(bounds, letterBound);
    SAY("foo");
  }
  renderString(str, cursor);
}

void GlUtils::renderString(const std::string & str, vec3 & cursor3d,
    float fontSize, Resource fontResource) {
  vec4 target = vec4(cursor3d, 0);
  target = Stacks::projection().top() * Stacks::modelview().top() * target;
  vec2 newCursor(target.x, target.y);
  renderString(str, newCursor, fontSize, fontResource);
}

Text::FontPtr GlUtils::getFont(Resource fontName) {
  static std::map<Resource, Text::FontPtr> fonts;
  if (fonts.find(fontName) == fonts.end()) {
    std::string fontData = Platform::getResourceData(fontName);
    Text::FontPtr result(new Text::Font());
    result->read((const void*)fontData.data(), fontData.size());
    fonts[fontName] = result;
  }
  return fonts[fontName];
}

Text::FontPtr GlUtils::getDefaultFont() {
  return getFont(Resource::FONTS_INCONSOLATA_MEDIUM_SDFF);
}

template<GLenum TYPE> struct ShaderInfo {
  time_t modified;
  gl::ShaderPtr shader;

  bool valid(Resource resource) {
    return shader &&
      (Resources::getResourceModified(resource) <= modified);
  }

  void compile(Resource resource) {
    SAY("Compiling shader file %s",
      Resources::getResourcePath(resource).c_str());

    std::string shaderSource =
      Platform::getResourceData(resource);
    modified = Resources::getResourceModified(resource);
    shader = gl::ShaderPtr(new gl::Shader(TYPE, shaderSource));
  }

  bool update(Resource resource) {
    if (!valid(resource)) {
      compile(resource);
      return true;
    }
    return false;
  }
  const Mesh & GlUtils::getMesh(Resource res) {
    typedef std::map<Resource, MeshPtr> MeshMap;
    static MeshMap meshes;
    if (0 == meshes.count(res)) {
      std::string meshData = Platform::getResourceData(res);
      meshes[res] = MeshPtr(new Mesh(meshData));
    }
    return *meshes[res];
}

};


void GlUtils::renderBunny() {
  static gl::GeometryPtr bunnyGeometry =
    getMesh(Resource::MESHES_BUNNY2_CTM).getGeometry();
  gl::ProgramPtr program = GlUtils::getProgram(
    Resource::SHADERS_LITCOLORED_VS,
    Resource::SHADERS_LITCOLORED_FS);
  program->use();
  program->setUniform("Color", vec4(1));
  renderGeometry(bunnyGeometry, program);
}

gl::GeometryPtr GlUtils::getQuadGeometry(float aspect, float size) {
  vec2 min(size / -2.0f), max(size / 2.0f);
  if (aspect > 1.0f) {
    min.y /= aspect;
    max.y /= aspect;
  }
  else if (aspect < 1.0f) {
    min.x *= aspect;
    max.x *= aspect;
  }
  return getQuadGeometry(min, max);
}


gl::GeometryPtr GlUtils::getQuadGeometry(const vec2 & min,
  const vec2 & max, const vec2 & texMin,
  const vec2 & texMax) {
  std::vector<vec4> v;
  v.push_back(vec4(min.x, min.y, 0, 1));
  v.push_back(vec4(texMin.x, texMin.y, 0, 0));
  v.push_back(vec4(max.x, min.y, 0, 1));
  v.push_back(vec4(texMax.x, texMin.y, 0, 1));
  v.push_back(vec4(min.x, max.y, 0, 1));
  v.push_back(vec4(texMin.x, texMax.y, 0, 1));
  v.push_back(vec4(max.x, max.y, 0, 1));
  v.push_back(vec4(texMax.x, texMax.y, 0, 1));

  std::vector<GLuint> i;
  i.push_back(0);
  i.push_back(1);
  i.push_back(2);
  i.push_back(3);
  i.push_back(2);
  i.push_back(1);
  i.push_back(3);
  return gl::GeometryPtr(new gl::Geometry(
    v, i, 2,
    // Buffer has texture coordinates
    gl::Geometry::Flag::HAS_TEXTURE,
    // Indices are for a triangle strip
    GL_TRIANGLES));
}


gl::GeometryPtr GlUtils::getCubeGeometry() {
  static gl::GeometryPtr cube;
  if (!cube) {
    cube = gl::GeometryPtr(new gl::Geometry(
      getCubeVertices(),
      getCubeIndices(),
      12, 0, GL_TRIANGLES, 3));
  }
  return cube;
}

gl::GeometryPtr GlUtils::getWireCubeGeometry() {
  static gl::GeometryPtr wireframe =
    gl::GeometryPtr(new gl::Geometry(
    getCubeVertices(),
    getCubeWireIndices(),
    12, 0, GL_LINES, 3));
  return wireframe;
}

static vec3 AXES[] = {
  GlUtils::X_AXIS,
  GlUtils::Y_AXIS,
  GlUtils::Z_AXIS };

void cubeRecurseDraw(gl::GeometryPtr & cubeGeometry, gl::ProgramPtr & renderProgram, int depth, float elapsed, int axisIndex) {
  if (0 == depth) {
    return;
  }

  static vec3 translation(0, 0, 1.5);

  const vec3 & axis = AXES[axisIndex % 3];

  float angle = elapsed * 0.2f * ((rand() % 10) - 5);

  float scale = 0.7f;
  MatrixStack & mv = Stacks::modelview();
  mv.with_push([&]{
    mv.rotate(angle, axis).translate(translation).scale(scale).apply(renderProgram);
    cubeGeometry->draw();
    cubeRecurseDraw(cubeGeometry, renderProgram, depth - 1, elapsed, axisIndex + 1);
  });
  mv.with_push([&]{
    mv.rotate(angle + PI, axis).translate(translation).scale(scale).apply(renderProgram);
    cubeGeometry->draw();
    cubeRecurseDraw(cubeGeometry, renderProgram, depth - 1, elapsed, axisIndex + 1);
  });
}

void GlUtils::cubeRecurse(int depth, float elapsed) {
  srand(4);
  gl::ProgramPtr renderProgram = GlUtils::getProgram(
    Resource::SHADERS_COLORED_VS, Resource::SHADERS_COLORED_FS);
  renderProgram->use();
  Stacks::projection().apply(renderProgram);

  static gl::GeometryPtr cubeGeometry = GlUtils::getColorCubeGeometry();
  cubeGeometry->bindVertexArray();
  MatrixStack & mv = Stacks::modelview();
  mv.with_push([&]{
    mv.scale(0.4f);
    mv.apply(renderProgram);
    cubeGeometry->draw();
    cubeRecurseDraw(cubeGeometry, renderProgram, depth - 1, elapsed, 1);
  });
  gl::VertexArray::unbind();
  gl::Program::clear();
}

void GlUtils::dancingCubes(int elements, float elapsed) {
  gl::ProgramPtr renderProgram = GlUtils::getProgram(
    Resource::SHADERS_COLORED_VS, Resource::SHADERS_COLORED_FS);
  renderProgram->use();
  Stacks::projection().apply(renderProgram);

  static gl::GeometryPtr cubeGeometry = getColorCubeGeometry();
  cubeGeometry->bindVertexArray();

  static vec3 AXES[] = { GlUtils::X_AXIS, GlUtils::Y_AXIS,
    GlUtils::Z_AXIS };

  MatrixStack & mv = Stacks::modelview();
  mv.with_push([&]{
    mv.scale(0.2f);
    Stacks::modelview().apply(renderProgram);
    cubeGeometry->draw();
  });

  srand(4);
  for (int i = 0; i < elements; ++i) {
    float angle = elapsed * 0.05f * (float)((rand() % 6) - 2);
    float angle2 = elapsed * 0.05f * (float)((rand() % 3) - 1);
    int axisRotate = rand();
    const vec3 & axis = AXES[axisRotate % 3];
    const vec3 & axis2 = AXES[(1 + axisRotate) % 3];
    int axisTranslate = axisRotate + rand() % 2;
    float sc = 1.0f / (float)(5 + rand() % 8);
    vec3 tr = AXES[axisTranslate % 3];
    tr = tr / (float)(1 + (rand() % 6)) + (tr * 0.4f);
    mv.push().rotate(angle, axis).rotate(angle2, axis2);

    mv.with_push([&]{
      mv.translate(tr).scale(sc);
      Stacks::modelview().apply(renderProgram);
      cubeGeometry->draw();
    });
    mv.with_push([&]{
      mv.translate(-tr).scale(sc);
      Stacks::modelview().apply(renderProgram);
      cubeGeometry->draw();
    });
  }
  for (int i = 0; i < elements; ++i) {
    mv.pop();
  }


}
#endif

