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

static uvec2 UI_SIZE(1920, 1080);

template <class T>
class UiTestWrapperApp : public SdlWrapperApp<T, uvec2> {
  typedef SdlWrapperApp<T, int> Super;

  virtual SDL_Window * createWindow() {
    int numDisplays = SDL_GetNumVideoDisplays();
    for (int i = 0; i < numDisplays; ++i) {
      SDL_Rect bounds;
      SDL_GetDisplayBounds(i, &bounds);
      if (bounds.x == 0 && bounds.y == 0) {
        continue;
      }
      windowSize.x = bounds.w;
      windowSize.y = bounds.h;
      windowPosition.x = bounds.x;
      windowPosition.y = bounds.y;
      break;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_Window * result = SDL_CreateWindow("SDL",
      windowPosition.x, windowPosition.y,
      windowSize.x, windowSize.y,
      SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);
    return result;
  }

  virtual uvec2 getArgs() {
    return windowSize;
  }
};

static class GameListSource : public Rocket::Controls::DataSource
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
      if (columns[i] == "GameId") {
        Rocket::Core::String str;
        str.FormatString(16, "%d", game.id);
        row.push_back(str);
      } else if (columns[i] == "Player1") {
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

class UiTest : public Rocket::Core::EventListener {
  bool quit{ false };
  // Task queue
  boost::mutex taskQueueMutex;
  std::queue<boost::function<void()> > taskQueue;
  uvec2       windowSize;

  // FICS state and interaction
  Fics::GameList games;

  // UI rendering
  GameListSource listSource;
  RocketSurface uiSurface;
  Geometry uiGeometry;
  oglplus::Context gl;
  Rocket::Core::ElementDocument* document;

public:

  UiTest(const uvec2 & windowSize) :
    uiSurface(UI_SIZE),
    windowSize(windowSize),
    listSource(games)
  {
    // We handle our own mouse cursor
    SDL_ShowCursor(0);
    // We want text input
    SDL_StartTextInput();

    // Load the rocket UI
    uiSurface.ctx->LoadMouseCursor("/users/bdavis/git/VirtualChess/resources/rocket/cursor.rml");

    {
      Mesh uiMesh;
      uiMesh.addTexturedQuad(1.5f, 1.5f);
      uiGeometry.loadMesh(uiMesh);
    }
    games = Fics::GameSummary::parseList(
      Platform::getResourceString(Resource::MISC_GAMELIST_TXT));

    reloadDocument();
  }

  template <typename Function>
  void addTask(Function f) {
    withScopedLock(taskQueueMutex, [&](const boost::mutex::scoped_lock &){
      taskQueue.push(f);
    });
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
    addTask([&]{
      if (nullptr != document) {
        uiSurface.ctx->UnloadAllDocuments();
        document = nullptr;
      }

      document = uiSurface.ctx->LoadDocument("/users/bdavis/git/VirtualChess/resources/rocket/tutorial.rml");
      if (nullptr != document)
      {
        document->Show(Rocket::Core::ElementDocument::MODAL);
        //string username, password; readLogin(username, password);
        //Rocket::Controls::ElementFormControlInput * uel = (Rocket::Controls::ElementFormControlInput *)document->GetElementById("username");
        //uel->SetValue(username.c_str());
        //Rocket::Controls::ElementFormControlInput * pel = (Rocket::Controls::ElementFormControlInput *)document->GetElementById("password");
        //pel->SetValue(password.c_str());
        //Rocket::Controls::ElementFormControlInput * cel = (Rocket::Controls::ElementFormControlInput *)document->GetElementById("connect");
        document->AddEventListener("click", this);
        document->RemoveReference();
      }
    });
    listSource.Update();
  }

  /// Process the incoming Rocket
  virtual void ProcessEvent(Rocket::Core::Event& event) {
    Rocket::Core::Element * el = event.GetTargetElement();
    string elId = el->GetId().CString();
    string elTag = el->GetTagName().CString();
    if (string("connect") == elId) {
      games = Fics::GameSummary::parseList(
        Platform::getResourceString(Resource::MISC_GAMELIST_TXT));
      reloadDocument();
      listSource.Update();
      return;
    } else if (string("reload") == elId) {
      reloadDocument();
      return;
    } else if (string("datagridcell") == elTag) {
      Rocket::Controls::ElementDataGridRow * row = (Rocket::Controls::ElementDataGridRow *)el->GetParentNode();
      SAY("Row %d", row->GetTableRelativeIndex());

    }
  }

  virtual bool isDone() {
    return quit;
  }

  void updateState() {
    // Process any background tasks queued up
    withScopedLock(taskQueueMutex, [&](const boost::mutex::scoped_lock &){
      while (!taskQueue.empty()) {
        boost::function<void()> f = taskQueue.front();
        taskQueue.pop();
        f();
      }
    });

    uiSurface.render();

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
          vec2 mpos(event.motion.x, event.motion.y);
          mpos.x /= windowSize.x;
          mpos.y /= windowSize.y;
          mpos *= UI_SIZE;
          uiSurface.ctx->ProcessMouseMove(mpos.x, mpos.y, RocketBridge::GetKeyModifiers());
        } return;

      case SDL_MOUSEWHEEL:
        uiSurface.ctx->ProcessMouseWheel(-event.wheel.y, RocketBridge::GetKeyModifiers());
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
      }
    }
  }

  void drawScene() {
    gl.Clear().DepthBuffer();
    gl.Viewport(windowSize.x, windowSize.y);
    uiSurface.tex.Bind(Texture::Target::_2D);
    MatrixStack & mv = Stacks::modelview();
    mv.withPush([&]{
      Render::renderGeometry(
        GlUtils::getProgram(Resource::SHADERS_TEXTURED_VS, Resource::SHADERS_TEXTURED_FS),
        uiGeometry
      );
    });
  }

  void onTick() {
    updateState();
    drawScene();
  }

};

///RUN_APP(UiTestWrapperApp<UiTest>);


