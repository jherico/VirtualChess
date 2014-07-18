#include "Common.h"
#include "FicsClient.h"
#include "Rocket.h"


#pragma warning( disable : 4068 4244 4099 4305 4101)

#include <oglplus/images/load.hpp>
#include <oglplus/bound/texture.hpp>
#include <oglplus/bound/framebuffer.hpp>
#include <oglplus/bound/renderbuffer.hpp>

#include <oglplus/opt/resources.hpp>
#include <oglplus/opt/list_init.hpp>
#pragma warning( default : 4068 4244 4099 4305 4101)

#include <openctmpp.h>


#include <Rocket/Core/RenderInterface.h>

using namespace oglplus;
using namespace std;

Resource PIECE_RESOURCES[] = {
  Resource::MESHES_CHESS_PAWN_CTM,
  Resource::MESHES_CHESS_ROOK_CTM,
  Resource::MESHES_CHESS_KNIGHT_CTM,
  Resource::MESHES_CHESS_BISHOP_CTM,
  Resource::MESHES_CHESS_QUEEN_CTM,
  Resource::MESHES_CHESS_KING_CTM,
};

static uvec2 UI_SIZE(1024, 768);


typedef std::shared_ptr<Rocket::Core::Context> RocketContextPtr;
class VirtualChess : public RiftApp {
  bool quit{ false };
  mat4 player;
  Program prog;
  Fics::ClientPtr ficsClient;
  Lights lights;
  Chess::Board board;

  RocketContextPtr rctx;
  oglplus::Texture      rtex;
  oglplus::Framebuffer  rfbo;


  static Geometry & getPieceGeometry(uint8_t piece) {
    int pieceShape = (piece - 1) & 0x0F;
    Resource res = PIECE_RESOURCES[pieceShape];
    return GlUtils::getGeometry(res);
  }

public:

  VirtualChess(const RiftWrapperArgs & args) : RiftApp(args) {
    RocketBridge::init();
    rctx.reset(Rocket::Core::CreateContext("main", 
      Rocket::Core::Vector2i(UI_SIZE.x, UI_SIZE.y)));

    // Load and show the tutorial document.
    Rocket::Core::ElementDocument* document = rctx->LoadDocument("data/tutorial.rml");
    if (document != NULL) {
      document->Show();
      document->RemoveReference();
    }

    gl.Bound(Texture::Target::_2D, rtex)
      .MinFilter(TextureMinFilter::Linear)
      .MagFilter(TextureMagFilter::Linear)
      .WrapS(TextureWrap::ClampToEdge)
      .WrapT(TextureWrap::ClampToEdge)
      .Image2D(0, PixelDataInternalFormat::RGBA8,
        UI_SIZE.x, UI_SIZE.y,
        0, PixelDataFormat::RGB, PixelDataType::UnsignedByte, nullptr
      );
    gl.Bound(Framebuffer::Target::Draw, rfbo)
      .AttachTexture(FramebufferAttachment::Color, rtex, 0)
      .Complete();

    player = glm::inverse(glm::lookAt(vec3(0, 0.25, 0.35), vec3(0, 0.25, 0), vec3(0, 1, 0)));
    prog = GlUtils::getProgram(Resource::SHADERS_LIT_VS, Resource::SHADERS_LITCOLORED_FS);
    string username, password; {
      string homeDir = getenv("HOME");
      istringstream prefs(Files::read(homeDir + "/.ficsLogin"));
      prefs >> username;
      prefs >> password;
    }
    //ficsClient = Fics::Client::create();
    //ficsClient->connect(username, password);
    //ficsClient->setGameCallback(boost::bind(&VirtualChess::onGameState, this, _1));
    //Fics::GameList games = ficsClient->games();
    //ficsClient->observe(8);
  }

  virtual void onGameState(const Fics::GameState & gameState) {
    board = gameState.board;
  }

  virtual bool isDone() {
    return quit;
  }

  void updateState() {
    {
      rfbo.Bind(oglplus::Framebuffer::Target::Draw);
      gl.Viewport(UI_SIZE.x, UI_SIZE.y);
      gl.Clear().ColorBuffer();
      rctx->Update();
      rctx->Render();
      oglplus::DefaultFramebuffer().Bind(oglplus::Framebuffer::Target::Draw);
    }


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
    SET_LIGHTS(prog, lights);

    MatrixStack & mv = Stacks::modelview();
    mv.withPush([&]{
      mv.scale(GlUtils::CHESS_SCALE);
      mv.translate(vec3(-3.5, 0, -3.5));
      Chess::forEachSquare([&](int row, int col){
        mv.withPush([&] {
          uint8_t piece = board.position[row][col];
          if (!piece) {
            return;
          }
          mv.translate(vec3(col, 0, 7 - row));

          vec3 color = Colors::white;
          if (piece & 0x10) {
            color = Colors::dimGrey;
          } else {
            mv.rotate(PI, GlUtils::Y_AXIS);
          }

          Uniform<vec4>(prog, Layout::Uniform::Color).Set(vec4(color, 1));
          mv.scale(1.6f);
          SET_MODELVIEW(prog);
          Geometry & pieceGeometry = getPieceGeometry(piece);
          pieceGeometry.bind();
          pieceGeometry.draw();
          NoVertexArray().Bind();
        });
      });
    });
    NoProgram().Use();
    NoVertexArray().Bind();

    Render::renderGeometry(
      GlUtils::getProgram(Resource::SHADERS_COLORED_VS, Resource::SHADERS_COLORED_FS),
      GlUtils::getChessBoardGeometry()
    );
  }
  void drawScene() {
    gl.Clear().DepthBuffer();
    Render::renderSkybox(Resource::IMAGES_SKY_CITY_XNEG_PNG);
    renderBoard();
  }
};

RUN_OVR_APP(RiftWrapperApp<VirtualChess>);


