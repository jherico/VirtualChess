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
#include <Rocket/Controls/Controls.h>
#include <Rocket/Controls/DataSource.h>

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

static uvec2 UI_SIZE(800, 600);


class GameListSource : public Rocket::Controls::DataSource
{
  Fics::GameList & list;
public:
  GameListSource(Fics::GameList & list) : 
    Rocket::Controls::DataSource("gameList"), 
    list(list) 
  {
  }

  virtual ~GameListSource() {

  }

  virtual void GetRow(Rocket::Core::StringList& row, const Rocket::Core::String& table, int row_index, const Rocket::Core::StringList& columns) {
    const Fics::GameSummary & game = list.at(row_index);
    for (int i = 0; i < columns.size(); ++i) {
      if (columns[i] == "Player1") {
        row.push_back(game.players[0].c_str());
      } else if (columns[i] == "Player2") {
        row.push_back(game.players[1].c_str());
      } else if (columns[i] == "Type") {
        row.push_back(Platform::format("%d", game.type).c_str());
      } 
    }

  }

  virtual int GetNumRows(const Rocket::Core::String& table) {
    //return list.size();
    return std::min((int)list.size(), 10);
  }

  void Update() {
    NotifyRowAdd("gameList", 0, list.size());
  }
};

class GameListFormatter : public Rocket::Controls::DataFormatter
{
public:
  GameListFormatter() : Rocket::Controls::DataFormatter("game") {

  }

  virtual ~GameListFormatter() {

  }

  void FormatData(Rocket::Core::String& formatted_data, const Rocket::Core::StringList& raw_data) {
    formatted_data = "Test";
  }
};



class VirtualChess : public RiftApp, public Rocket::Core::EventListener {
  bool quit{ false };
  mat4 player;
  Fics::ClientPtr ficsClient;
  Lights lights;
  Fics::GameList games;
  GameListSource listSource;
  GameListFormatter listFormatter;
  Chess::Board board;
  RocketSurface uiSurface; 
  Geometry uiGeometry;
  Rocket::Core::ElementDocument* document;
  boost::mutex documentMutex;
  Rocket::Controls::ElementTabSet* tabset;


  static Geometry & getPieceGeometry(Chess::Piece piece) {
    int pieceShape = (piece - 1) & 0x0F;
    Resource res = PIECE_RESOURCES[pieceShape];
    return GlUtils::getGeometry(res);
  }

public:

  VirtualChess(const RiftWrapperArgs & args) : 
    RiftApp(args), 
    uiSurface(UI_SIZE), 
    ficsClient(Fics::Client::create()),
    listSource(games)
  {
    // We handle our own mouse cursor
    SDL_ShowCursor(0);
    // We want text input
    SDL_StartTextInput();

    // Starting position of the player
    player = glm::inverse(glm::lookAt(vec3(0, 0.25, 0.35), vec3(0, 0.25, 0), vec3(0, 1, 0)));
    reloadDocument();
    // Load the rocket UI
    uiSurface.ctx->LoadMouseCursor("/users/bdavis/git/VirtualChess/resources/rocket/cursor.rml");
    string username, password; {
      string homeDir = getenv("HOME");
      istringstream prefs(Files::read(homeDir + "/.ficsLogin"));
      prefs >> username;
      prefs >> password;
    }

    ficsClient->setEventHandler(boost::bind(&VirtualChess::onFicsEvent, this, _1));
//    ficsClient->connect(username, password);
//    Fics::GameList games = ficsClient->games();
//    ficsClient->observeGame(8);
    {
      Mesh uiMesh;
      uiMesh.addTexturedQuad(0.8f, 0.6f);
      uiGeometry.loadMesh(uiMesh);
    }
  }


  void readLogin(std::string & username, std::string & password) {
    string homeDir = getenv("HOME");
    istringstream prefs(Files::read(homeDir + "/.ficsLogin"));
    prefs >> username;
    prefs >> password;
  }

  void readLoginControls(std::string & username, std::string & password) {
    Rocket::Controls::ElementFormControlInput * uel = (Rocket::Controls::ElementFormControlInput *)document->GetElementById("username");
    username = uel->GetValue().CString();
    Rocket::Controls::ElementFormControlInput * pel = (Rocket::Controls::ElementFormControlInput *)document->GetElementById("password");
    password = pel->GetValue().CString();
  }

  virtual void reloadDocument() {
    withScopedLock(documentMutex, [&](const boost::mutex::scoped_lock &){
      if (nullptr != document) {
        uiSurface.ctx->UnloadAllDocuments();
        document = nullptr;
      }


      document = uiSurface.ctx->LoadDocument("/users/bdavis/git/VirtualChess/resources/rocket/tutorial.rml");
      if (nullptr != document)
      {
        document->Show(Rocket::Core::ElementDocument::MODAL);
        tabset = (Rocket::Controls::ElementTabSet*)document->GetElementById("tabset");
        string username, password; readLogin(username, password);
        Rocket::Controls::ElementFormControlInput * uel = (Rocket::Controls::ElementFormControlInput *)document->GetElementById("username");
        uel->SetValue(username.c_str());
        Rocket::Controls::ElementFormControlInput * pel = (Rocket::Controls::ElementFormControlInput *)document->GetElementById("password");
        pel->SetValue(password.c_str());
        Rocket::Controls::ElementFormControlInput * cel = (Rocket::Controls::ElementFormControlInput *)document->GetElementById("connect");
        document->AddEventListener("click", this);
        document->RemoveReference();
      }
    });
  }

  /// Process the incoming Rocket 
  virtual void ProcessEvent(Rocket::Core::Event& event) {
    if (Rocket::Core::String("connect") == event.GetTargetElement()->GetId()) {
      string username, password;
      readLoginControls(username, password);
      ficsClient->connect(username, password);
      return;
    }
    //EventListener::ProcessEvent(event);
    SAY("Event");
  }

  virtual void onFicsEvent(const Fics::Event & event) {
    SAY("FICS event");
    switch (event.type) {
      case Fics::EventType::NETWORK: {
        ficsClient->listGames();
      }return;
      case Fics::EventType::GAME_LIST: {
        games = *event.gameList.list;
        reloadDocument();
        listSource.Update();
        SAY("Game List");
      }return;
    }
  }

  virtual bool isDone() {
    return quit;
  }

  void updateState() {
    withScopedLock(documentMutex, [&](const boost::mutex::scoped_lock &){
      uiSurface.render();
    });

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_TEXTINPUT: {
          auto te = event.text;
          size_t len = strlen(te.text);
          for (int i = 0; i < len; ++i) {
            uiSurface.ctx->ProcessTextInput((Rocket::Core::word) te.text[i]);
          }
        } return;

      case SDL_MOUSEMOTION: {
          uvec2 mpos(event.motion.x, event.motion.y);
          mpos = glm::min(mpos, UI_SIZE);
          uiSurface.ctx->ProcessMouseMove(mpos.x, mpos.y, RocketBridge::GetKeyModifiers());
        } return;

      case SDL_MOUSEWHEEL:
        uiSurface.ctx->ProcessMouseWheel(event.wheel.y, RocketBridge::GetKeyModifiers());
        return;

      case SDL_MOUSEBUTTONDOWN:
        uiSurface.ctx->ProcessMouseButtonDown(RocketBridge::TranslateMouseButton(event.button.button), RocketBridge::GetKeyModifiers());
        return;

      case SDL_MOUSEBUTTONUP:
        uiSurface.ctx->ProcessMouseButtonUp(RocketBridge::TranslateMouseButton(event.button.button), RocketBridge::GetKeyModifiers());
        return;

      case SDL_KEYDOWN: {
        if (event.key.keysym.sym == SDLK_F1) {
          reloadDocument();
          return;
        }
        uiSurface.ctx->ProcessKeyDown(RocketBridge::TranslateKey(event.key.keysym.sym), RocketBridge::GetKeyModifiers());
        Rocket::Core::Input::KeyIdentifier key = RocketBridge::TranslateKey(event.key.keysym.sym);
        // Send through the ASCII value as text input if it is printable.
        if (key == Rocket::Core::Input::KI_RETURN) {
          uiSurface.ctx->ProcessTextInput((Rocket::Core::word) '\n');
        } 
      } return;

      case SDL_KEYUP:
        uiSurface.ctx->ProcessKeyUp(RocketBridge::TranslateKey(event.key.keysym.sym), RocketBridge::GetKeyModifiers());
        return;
        //case SDL_KEYDOWN:
      //  switch (event.key.keysym.sym) {
      //  case SDLK_r:
      //    ovrHmd_ResetSensor(hmd);
      //    return;

      //  case SDLK_ESCAPE:
      //    quit = true;
      //    return;
      //  }
      }
      CameraControl::instance().onEvent(event);
    }

    CameraControl::instance().applyInteraction(player);
    Stacks::modelview().top() = glm::inverse(player);
  }


  void renderBoard() {
    Program & prog = GlUtils::getProgram(
        Resource::SHADERS_LIT_VS, 
        Resource::SHADERS_LITCOLORED_FS);
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
          Chess::Piece piece = board.position[row][col];
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
    uiSurface.tex.Bind(Texture::Target::_2D);
    MatrixStack & mv = Stacks::modelview();
    mv.withPush([&]{
      mv.translate(vec3(0, 0.35, -0.35f));
      Render::renderGeometry(
        GlUtils::getProgram(Resource::SHADERS_TEXTURED_VS, Resource::SHADERS_TEXTURED_FS),
        uiGeometry
      );
    });
    //    MatrixStack & pr = Stacks::projection();
    //    pr.identity();
    renderBoard();
  }
};

RUN_OVR_APP(RiftWrapperApp<VirtualChess>);


