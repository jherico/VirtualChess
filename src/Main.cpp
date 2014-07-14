#include "Common.h"

#pragma warning( disable : 4068 4244 4099 4305 4101)

#include <oglplus/images/load.hpp>
#include <oglplus/bound/texture.hpp>
#include <oglplus/bound/framebuffer.hpp>
#include <oglplus/bound/renderbuffer.hpp>

#include <oglplus/opt/resources.hpp>
#include <oglplus/opt/list_init.hpp>
#pragma warning( default : 4068 4244 4099 4305 4101)

#include <openctmpp.h>

using namespace oglplus;
using namespace std;

#define SET_PROJECTION(program) \
  Uniform<mat4>(program, Layout::Uniform::Projection).Set(Stacks::projection().top())

#define SET_MODELVIEW(program) \
  Uniform<mat4>(program, Layout::Uniform::ModelView).Set(Stacks::modelview().top())

namespace Piece {
  enum {
    PAWN,
    ROOK,
    KNIGHT,
    BISHOP,
    QUEEN,
    KING,
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

template <typename Function>
void for_each_square(Function f) {
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      f(row, col);
    }
  }
}

/*
class VirtualChess : public RiftApp {
  bool quit{ false };
  mat4 player;

  Program prog;
  Geometry pieces[6];
  AGChess::StandardPosition position;


  Geometry & getPieceGeometry(const AGChess::Piece & piece) {
    switch (piece.enumValue()) {
    case AGChess::Piece::KingPiece:
      return pieces[Piece::KING];
    case AGChess::Piece::QueenPiece:
      return pieces[Piece::QUEEN];
    case AGChess::Piece::KnightPiece:
      return pieces[Piece::KNIGHT];
    case AGChess::Piece::BishopPiece:
      return pieces[Piece::BISHOP];
    case AGChess::Piece::RookPiece:
      return pieces[Piece::ROOK];
    case AGChess::Piece::PawnPiece:
      return pieces[Piece::PAWN];
    }
    throw std::runtime_error("Uknown piece");
  }

public:

  VirtualChess(const RiftWrapperArgs & args) : RiftApp(args) {
    player = glm::inverse(glm::lookAt(vec3(0, 0, 0.5), vec3(0, 0, 0), vec3(0, 1, 0)));
    prog = GlUtils::getProgram(Resource::SHADERS_LIT_VS, Resource::SHADERS_LITCOLORED_FS);
    for (int i = 0; i < Piece::COUNT; ++i) {
      Mesh pieceMesh;
      pieceMesh.model.scale(1.6f);
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
    Stacks::modelview().top() = glm::inverse(player);
  }

  void renderBoard() {
    prog.Use();
    SET_PROJECTION(prog);
    SET_MODELVIEW(prog);
    Uniform<vec4>(prog, 7).Set(vec4(0.3, 0.3, 0.3, 1.0));
    Uniform<int>(prog, "LightCount").Set(1);
    Uniform<int>(prog, "LightCount").Set(1);
    Uniform<vec4>(prog, "LightPosition[0]").Set(vec4(1));
    Uniform<vec4>(prog, "LightColor[0]").Set(vec4(1));

    MatrixStack & mv = Stacks::modelview();
    mv.withPush([&]{
      mv.scale(GlUtils::CHESS_SCALE);
      mv.translate(vec3(-3.5, 0, -3.5));
      for_each_square([&](int row, int col){
        mv.withPush([&] {
          AGChess::ColoredPiece agPiece = position.at(AGChess::Square((7 - row) * 8 + col));
          if (!agPiece.isValid()) {
            return;
          }
          mv.translate(vec3(col, 0, row));

          vec3 color = Colors::white;
          if (agPiece.color() != AGChess::Color::WhiteColor) {
            color = Colors::darkGray;
          } else {
            mv.rotate(PI, GlUtils::Y_AXIS);

          }
          Uniform<vec4>(prog, "Color").Set(vec4(color, 1));
          SET_MODELVIEW(prog);
          Geometry & pieceGeometry = getPieceGeometry(agPiece.piece());
          pieceGeometry.bind();
          pieceGeometry.draw();
        });
      });
    });
    NoProgram().Use();
    NoVertexArray().Bind();

//    Render::renderGeometry(
//      GlUtils::getProgram(Resource::SHADERS_COLORED_VS, Resource::SHADERS_COLORED_FS),
//      GlUtils::getChessBoardGeometry()
//    );
  }

  void drawScene() {
    gl.Clear().DepthBuffer();
    Render::renderSkybox(Resource::IMAGES_SKY_CITY_XNEG_PNG);
    renderBoard();
  }
};

//RUN_OVR_APP(RiftWrapperApp<VirtualChess>);
*/


