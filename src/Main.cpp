#include "Common.h"

#pragma warning( disable : 4068 4244 4099 4305 4101)
#include <oglplus/shapes/cube.hpp>

#include <oglplus/shapes/blender_mesh.hpp>
#include <oglplus/shapes/wrapper.hpp>
#include <oglplus/shapes/torus.hpp>
#include <oglplus/shapes/obj_mesh.hpp>

#include <oglplus/opt/resources.hpp>
#include <oglplus/images/png.hpp>
#include <oglplus/images/load.hpp>

#include <oglplus/bound/texture.hpp>
#include <oglplus/bound/framebuffer.hpp>
#include <oglplus/bound/renderbuffer.hpp>

#include <oglplus/opt/resources.hpp>
#include <oglplus/opt/list_init.hpp>
#pragma warning( default : 4068 4244 4099 4305 4101)

#include <openctmpp.h>

using namespace oglplus;


void loadCtm(Mesh & mesh, const std::string & data) {
  CTMimporter importer;
  importer.LoadData(data);

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

#define SET_PROJECTION(program) \
  Uniform<mat4>(program, Layout::Uniform::Projection).Set(gl::Stacks::projection().top())

#define SET_MODELVIEW(program) \
  Uniform<mat4>(program, Layout::Uniform::ModelView).Set(gl::Stacks::modelview().top())

namespace Piece {
  enum {
    PAWN,
    ROOK,
    KNIGHT,
    BISHOP,
    QUEEN,
//    KING,
    COUNT
  };
}

Resource PIECE_RESOURCES[] = {
  Resource::MESHES_CHESS_PAWN_CTM,
  Resource::MESHES_CHESS_ROOK_CTM,
  Resource::MESHES_CHESS_KNIGHT_CTM,
  Resource::MESHES_CHESS_BISHOP_CTM,
  Resource::MESHES_CHESS_QUEEN_CTM,
  Resource::MESHES_CHESS_KING_CTM,
};


class VirtualChess : public RiftApp {
  bool quit{ false };
  mat4 player;

  Program prog;
  Geometry pieces[5];
  
public:
  VirtualChess(const RiftWrapperArgs & args) : RiftApp(args) {
    player = glm::inverse(glm::lookAt(vec3(0, 0, 0.5), vec3(0, 0, 0), vec3(0, 1, 0)));
    prog = GlUtils::getProgram(Resource::SHADERS_LIT_VS, Resource::SHADERS_LITCOLORED_FS);
    for (int i = 0; i < Piece::COUNT; ++i) {
      Mesh pieceMesh;
      pieceMesh.model.scale(0.08f);
      loadCtm(pieceMesh, Platform::getResourceString(PIECE_RESOURCES[i]));
      pieces[i].loadMesh(pieceMesh);
    }
  }

  virtual bool isDone() {
    return quit;
  }

  void updateState() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_r:
          ovrHmd_ResetSensor(hmd);
          return;

        case SDLK_ESCAPE:
          quit = true;
          return;
        }
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        quit = true;
        return;
      }
      CameraControl::instance().onEvent(event);
    }

    CameraControl::instance().applyInteraction(player);
    gl::Stacks::modelview().top() = glm::inverse(player);
  }

  void drawScene() {
    glClearColor(0.4, 0.4, 0.4, 1);
    gl.Clear().ColorBuffer().DepthBuffer();
    GlUtils::renderSkybox(Resource::IMAGES_SKY_CITY_XNEG_PNG);
    prog.Use();
    SET_PROJECTION(prog);
    SET_MODELVIEW(prog);

    Uniform<vec4>(prog, "Ambient").Set(vec4(0.3, 0.3, 0.3, 1.0));
    Uniform<int>(prog, "LightCount").Set(1);
    Uniform<int>(prog, "LightCount").Set(1);
    Uniform<vec4>(prog, "LightPosition[0]").Set(vec4(1));
    Uniform<vec4>(prog, "LightColor[0]").Set(vec4(1));

    gl::MatrixStack & mv = gl::Stacks::modelview();
#define SQUARE_LENGTH  0.05f
    int piece = 0;
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < 8; ++j) {
        mv.withPush([&] {
          mv.translate(vec3(SQUARE_LENGTH * i, 0, SQUARE_LENGTH * j));
          SET_MODELVIEW(prog);
          ++piece;
          bool white = 0 == (piece % 2);
          Uniform<vec4>(prog, "Color").Set(vec4(white ? Colors::white : Colors::darkBlue, 1));
          piece %= Piece::COUNT;
          pieces[piece].bind();
          pieces[piece].draw();
        });
      }
    }
    NoProgram().Use();
    NoVertexArray().Bind();

    mv.withPush([&]{
      mv.scale(0.06f);
      Geometry & cube = GlUtils::getColorCubeGeometry();
      Program & cubeProgram = GlUtils::getProgram(Resource::SHADERS_COLORED_VS, Resource::SHADERS_COLORED_FS);
      cubeProgram.Use();
      SET_MODELVIEW(cubeProgram);
      SET_PROJECTION(cubeProgram);
      cube.bind();
      cube.draw();
      NoProgram().Use();
    });
    NoVertexArray().Bind();
  }
};

RUN_OVR_APP(RiftWrapperApp<VirtualChess>);
