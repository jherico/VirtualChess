#include "Common.h"
#include "FicsClient.h"
#include <queue>



#pragma warning( disable : 4068 4244 4099 4305 4101)

#include <oglplus/images/load.hpp>
#include <oglplus/bound/texture.hpp>
#include <oglplus/bound/framebuffer.hpp>
#include <oglplus/bound/renderbuffer.hpp>

#include <oglplus/opt/resources.hpp>
#include <oglplus/opt/list_init.hpp>
#pragma warning( default : 4068 4244 4099 4305 4101)


using namespace oglplus;
using namespace std;
using namespace CEGUI;

Resource PIECE_RESOURCES[] = {
  Resource::MESHES_CHESS_PAWN_CTM,
  Resource::MESHES_CHESS_ROOK_CTM,
  Resource::MESHES_CHESS_KNIGHT_CTM,
  Resource::MESHES_CHESS_BISHOP_CTM,
  Resource::MESHES_CHESS_QUEEN_CTM,
  Resource::MESHES_CHESS_KING_CTM,
};

static uvec2 UI_SIZE(640, 480);

static Geometry & getPieceGeometry(Chess::Piece piece) {
  int pieceShape = (piece - 1) & 0x0F;
  Resource res = PIECE_RESOURCES[pieceShape];
  return GlUtils::getGeometry(res);
}

class TaskQueue {
  boost::mutex m;
  std::queue<boost::function<void()>> q;
public:

  template <typename Function>
  void add(Function f) {
    withScopedLock(m, [&](const boost::mutex::scoped_lock &){
      q.push(f);
    });
  }

  void drain(long maxTimeMs = 0) {
    long start = Platform::elapsedMillis();
    while (!q.empty()) {
      boost::function<void()> f;
      withScopedLock(m, [&](const boost::mutex::scoped_lock &){
        f = q.front();
        q.pop();
      });
      f();
      long now = Platform::elapsedMillis();
      if (maxTimeMs && ((now - start) > maxTimeMs)) {
        break;
      }
    }
  }
};

struct OffscreenFrame {
  uvec2                 size;
  oglplus::Texture      tex;
  oglplus::Framebuffer  fbo;

private:
  oglplus::Context gl;

public:
  OffscreenFrame(const uvec2 & size) : size(size) {
    using namespace oglplus;
    gl.Bound(oglplus::Texture::Target::_2D, tex)
      .MinFilter(TextureMinFilter::Linear)
      .MagFilter(TextureMagFilter::Linear)
      .WrapS(TextureWrap::ClampToEdge)
      .WrapT(TextureWrap::ClampToEdge)
      .Image2D(0, PixelDataInternalFormat::RGBA8,
      size.x, size.y,
      0, PixelDataFormat::RGB, PixelDataType::UnsignedByte, nullptr
      );
    gl.Bound(Framebuffer::Target::Draw, fbo)
      .AttachTexture(FramebufferAttachment::Color, tex, 0)
      .Complete();
  }

  template <typename Function>
  void withFbo(Function f) {
    using namespace oglplus;
    fbo.Bind(Framebuffer::Target::Draw);
    gl.Viewport(size.x, size.y);
    gl.Clear().ColorBuffer();
    f();
    DefaultFramebuffer().Bind(Framebuffer::Target::Draw);
  }
};


class VirtualChess : public RiftApp {
  bool quit{ false };
  // Task queue

  // GL rendering
  mat4 player{ glm::inverse(glm::lookAt(vec3(0, 0.25, 0.35), vec3(0, 0.25, 0), vec3(0, 1, 0))) };
  Lights lights;

  // FICS state and interaction
  Fics::ClientPtr ficsClient;
  Fics::GameList games;
  Chess::Board board;
  OffscreenFrame ui{ UI_SIZE };
  int activeGame{ -1 };
  bool loggedIn{ false };
  TaskQueue taskQueue;

  // UI rendering
  Geometry uiGeometry;

  FrameWindow * rootWindow;


public:

  VirtualChess(const RiftWrapperArgs & args) : 
    RiftApp(args), 
    ficsClient(Fics::Client::create())
  {
    // Set the callback for FICS events   
    ficsClient->setEventHandler(boost::bind(&VirtualChess::onFicsEvent, this, _1));

    // Load the rocket UI
    {
      Gui::init(UI_SIZE);
      Mesh uiMesh;
      vec2 quadSize(UI_SIZE);
      if (quadSize.x > quadSize.y) {
        quadSize /= quadSize.x;
      } else {
        quadSize /= quadSize.y;
      }
      quadSize *= 0.8f;
      uiMesh.addTexturedQuad(quadSize.x, quadSize.y);
      uiGeometry.loadMesh(uiMesh);

      WindowManager & wmgr = WindowManager::getSingleton();
      rootWindow = dynamic_cast<FrameWindow *>(wmgr.createWindow("TaharezLook/FrameWindow", "root"));
      rootWindow->setTitleBarEnabled(false);
      rootWindow->setAlpha(0.8f);
      System::getSingleton().getDefaultGUIContext().setRootWindow(rootWindow);
      rootWindow->setSize(CEGUI::USize(cegui_absdim(UI_SIZE.x), cegui_absdim(UI_SIZE.y)));

      {
        rootWindow->addChild(wmgr.loadLayoutFromFile("Login.layout"));
        rootWindow->getChild("LoginWindow/Login")->
          subscribeEvent(PushButton::EventClicked,
          [&](const EventArgs& e) -> bool {
            string username, password;
            username = rootWindow->getChild("LoginWindow/Username")->getText().c_str();
            password = rootWindow->getChild("LoginWindow/Password")->getText().c_str();
            rootWindow->getChild("LoginWindow/Login")->setEnabled(false);
            ficsClient->connect(username, password);
            return true;
          });
        rootWindow->getChild("LoginWindow")->hide();

      }

      {
        rootWindow->addChild(wmgr.loadLayoutFromFile("Tabs.layout"));
        TabControl* tc = static_cast<TabControl*>(rootWindow->getChild("Tabs"));
        //// Add some pages to tab control
        tc->addTab(wmgr.loadLayoutFromFile("TabChat.layout"));
        tc->addTab(wmgr.loadLayoutFromFile("TabGames.layout"));
        tc->addTab(wmgr.loadLayoutFromFile("TabPlayers.layout"));
        tc->hide();

        MultiColumnList* mcl = static_cast<MultiColumnList*>(rootWindow->getChild("Tabs/Games/MultiColumnList"));
        mcl->setSelectionMode(MultiColumnList::RowSingle);
        mcl->addColumn("Id", 0, cegui_reldim(0.1f));
        mcl->addColumn("P", 1, cegui_reldim(0.05f));
        mcl->addColumn("Type", 2, cegui_reldim(0.2f));
        mcl->addColumn("Player (White)", 3, cegui_reldim(0.325));
//        mcl->addColumn("Rating", 3, cegui_reldim(0.07f));
        mcl->addColumn("Player (Black)", 4, cegui_reldim(0.325));
//        mcl->addColumn("Rating", 5, cegui_reldim(0.07f));
//        mcl->addColumn("Rated", 7, cegui_reldim(0.05f));
        mcl->subscribeEvent(MultiColumnList::EventMouseClick, [&](const EventArgs& e) -> bool {
          MultiColumnList* mcl = static_cast<MultiColumnList*>(rootWindow->getChild("Tabs/Games/MultiColumnList"));
          const MouseEventArgs & me = (const MouseEventArgs &)e;
          auto selMode = mcl->getSelectionMode();
          ListboxItem * li = mcl->getFirstSelectedItem();
          const Fics::GameSummary & summary = games.at(li->getID());
          if (-1 != activeGame) {
            ficsClient->unobserveGame(activeGame);
          }
          activeGame = summary.id;
          ficsClient->observeGame(activeGame);
          //ficsClient->listGames();
          return false;
        });

      }
      showLoginUi();
    }
  }

  static void readLogin(std::string & username, std::string & password) {
    string homeDir = getenv("HOME");
    istringstream prefs(Files::read(homeDir + "/.ficsLogin"));
    prefs >> username;
    prefs >> password;
  }

  void hideAll() {
    for (int i = 0; i < rootWindow->getChildCount(); ++i) {
      rootWindow->getChildAtIdx(i)->hide();
    }
  }

  void showLoginUi() {
    hideAll();
//    clearRootWindow();
    WindowManager & wmgr = WindowManager::getSingleton();
    rootWindow->getChild("LoginWindow")->show();
    {
      string username, password;
      readLogin(username, password);
      rootWindow->getChild("LoginWindow/Username")->setText(username);
      rootWindow->getChild("LoginWindow/Password")->setText(password);
    }
  }

  void showGameUi() {
    hideAll();
    TabControl* tc = static_cast<TabControl*>(rootWindow->getChild("Tabs"));
    tc->show();
  }

  virtual void reloadDocument() {
    taskQueue.add([&]{
      MultiColumnList* mcl = static_cast<MultiColumnList*>(rootWindow->getChild("Tabs/Games/MultiColumnList"));
      mcl->resetList();
    });
    fillRows();
  }

  // Sample sub-class for ListboxTextItem that auto-sets the selection brush
  // image.  This saves doing it manually every time in the code.
  class MyListItem : public ListboxTextItem
  {
  public:
    MyListItem(const CEGUI::String& text, CEGUI::uint item_id = 0) :
      ListboxTextItem(text, item_id)
    {
      setSelectionBrushImage("TaharezLook/MultiListSelectionBrush");
    }

    MyListItem(int text, CEGUI::uint item_id = 0) :
      ListboxTextItem(Platform::format("%d", text), item_id)
    {
      setSelectionBrushImage("TaharezLook/MultiListSelectionBrush");
    }
  };

  virtual void fillRows() {
    taskQueue.add([&]{
      long start = Platform::elapsedMillis();
      MultiColumnList* mcl = static_cast<MultiColumnList*>(rootWindow->getChild("Tabs/Games/MultiColumnList"));
      for (int i = mcl->getRowCount(); i < games.size(); ++i) {
        Fics::GameSummary & g = games[i];
        mcl->addRow();

        mcl->setItem(new MyListItem(g.id, i), 0, i);
        mcl->setItem(new MyListItem(g.private_ ? "Y" : "N", i), 1, i);
//        mcl->setItem(new ListboxTextItem(g.rated ? "Y" : "N"), 7, i);
        mcl->setItem(new MyListItem(Chess::getTypeName(g.type), i), 2, i);
        mcl->setItem(new MyListItem(g.players[0], i), 3, i);
        mcl->setItem(new MyListItem(g.players[1], i), 4, i);
        //mcl->setItem(new ListboxTextItem(Platform::format("%d", g.ratings[j])), 3 + (j * 2), i);
        long now = Platform::elapsedMillis();
        if ((now - start) > 1) {
          fillRows();
          return;
        }
      }
    });
  }

  virtual void onFicsEvent(const Fics::Event & event) {
    switch (event.type) {

      case Fics::EventType::NETWORK: {
        loggedIn = true;
        ficsClient->listGames();
        showGameUi();
        reloadDocument();
      } return;

      case Fics::EventType::GAME_LIST: {
        games = *event.gameList.list;
        reloadDocument();
      } return;

      case Fics::EventType::GAME_STATE: {
        auto gameState = *event.gameState.state;
        if (gameState.id == activeGame) {
          board = gameState.board;
        } else {
          SAY("Dropping game state event from inactive game %d (active game is %d)", gameState.id, activeGame);
        }
      } return;

      case Fics::EventType::CHAT: {
        string message(event.chat.message);
        Listbox * mle = static_cast<Listbox*>(rootWindow->getChild("Tabs/Chat/Lines"));
        mle->addItem(new ListboxTextItem(message));
      } return;
    }
    SAY("Unhandled FICS event");
  }

  virtual bool isDone() {
    return quit;
  }

#define MAX_MILLIS 1

  bool handleSdlEvent(const SDL_Event & event) {
    switch (event.type) {
      case SDL_KEYDOWN: {
        switch (event.key.keysym.sym) {
          case SDLK_F6: {
            ovrHmd_RecenterPose(hmd);
          } return true;
    
          case SDLK_F5: {
            static bool lowPersistence = false;
            if (lowPersistence) {
              ovrHmd_SetEnabledCaps(hmd, ovrHmd_GetEnabledCaps(hmd) & ~ovrHmdCap_LowPersistence);
              lowPersistence = false;
            }
            else {
              ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_LowPersistence);
              lowPersistence = true;
            }
          } return true;

          case SDLK_F4: {
            static bool dynamic = false;
            if (dynamic) {
              ovrHmd_SetEnabledCaps(hmd, ovrHmd_GetEnabledCaps(hmd) & ~ovrHmdCap_DynamicPrediction);
              dynamic = false;
            }
            else {
              ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_DynamicPrediction);
              dynamic = true;
            }
          } return true;

          case SDLK_q: {
            if (event.key.keysym.mod & KMOD_CTRL) {
              quit = true;
              return true;
            }
          }
          default:
            break;
        } // switch (event.key.keysym.sym)
      } // case SDL_KEYDOWN
    } // switch (event.type) 
    return false;
  }

  void updateState() {
    ovrHSWDisplayState hsw;
    ovrHmd_GetHSWDisplayState(hmd, &hsw);
    if (hsw.Displayed) {
      ovrHmd_DismissHSWDisplay(hmd);
    }
    // Process any background tasks queued up
    taskQueue.drain(MAX_MILLIS);

    static const vec2 windowScaleFactor = vec2(UI_SIZE) / vec2(windowSize);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (handleSdlEvent(event)) {
        continue;
      }
      if (Gui::handleSdlEvent(event, windowScaleFactor)) {
        continue;
      }

      if (CameraControl::instance().onEvent(event)) {
        continue;
      }
    }

    ui.withFbo([]{
      System::getSingleton().renderAllGUIContexts();
      glDisable(GL_SCISSOR_TEST);
    });

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
    gl.Clear().ColorBuffer().DepthBuffer();
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    Render::renderProceduralSkybox(Resource::SHADERS_MOVINGTHROUGHSPEHERESPACE_FS);
//    Render::renderSkybox(Resource::IMAGES_SKY_CITY_XNEG_PNG);
    return;
    MatrixStack & mv = Stacks::modelview();
    mv.withPush([&]{
      mv.translate(vec3(0, 0.35, -0.35f));
      ui.tex.Bind(oglplus::Texture::Target::_2D);
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


