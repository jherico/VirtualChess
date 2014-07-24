#include "Common.h"
#include <CEGUI/RendererModules/OpenGL/GL3Renderer.h>

using namespace CEGUI;

MouseButton TranslateMouseButton(Uint8 button)
{
  switch (button)
  {
  case SDL_BUTTON_LEFT:
    return MouseButton::LeftButton;
  case SDL_BUTTON_RIGHT:
    return MouseButton::RightButton;
  case SDL_BUTTON_MIDDLE:
    return MouseButton::MiddleButton;
  default:
    return MouseButton::NoButton;
  }
}


CEGUI::Key::Scan TranslateKey(const SDL_Keysym & sdlkey)
{
  switch (sdlkey.scancode) {
  case SDL_SCANCODE_A: return Key::A;
  case SDL_SCANCODE_B: return Key::B;
  case SDL_SCANCODE_C: return Key::C;
  case SDL_SCANCODE_D: return Key::D;
  case SDL_SCANCODE_E: return Key::E;
  case SDL_SCANCODE_F: return Key::F;
  case SDL_SCANCODE_G: return Key::G;
  case SDL_SCANCODE_H: return Key::H;
  case SDL_SCANCODE_I: return Key::I;
  case SDL_SCANCODE_J: return Key::J;
  case SDL_SCANCODE_K: return Key::K;
  case SDL_SCANCODE_L: return Key::L;
  case SDL_SCANCODE_M: return Key::M;
  case SDL_SCANCODE_N: return Key::N;
  case SDL_SCANCODE_O: return Key::O;
  case SDL_SCANCODE_P: return Key::P;
  case SDL_SCANCODE_Q: return Key::Q;
  case SDL_SCANCODE_R: return Key::R;
  case SDL_SCANCODE_S: return Key::S;
  case SDL_SCANCODE_T: return Key::T;
  case SDL_SCANCODE_U: return Key::U;
  case SDL_SCANCODE_V: return Key::V;
  case SDL_SCANCODE_W: return Key::W;
  case SDL_SCANCODE_X: return Key::X;
  case SDL_SCANCODE_Y: return Key::Y;
  case SDL_SCANCODE_Z: return Key::Z;
  case SDL_SCANCODE_1: return Key::One;
  case SDL_SCANCODE_2: return Key::Two;
  case SDL_SCANCODE_3: return Key::Three;
  case SDL_SCANCODE_4: return Key::Four;
  case SDL_SCANCODE_5: return Key::Five;
  case SDL_SCANCODE_6: return Key::Six;
  case SDL_SCANCODE_7: return Key::Seven;
  case SDL_SCANCODE_8: return Key::Eight;
  case SDL_SCANCODE_9: return Key::Nine;
  case SDL_SCANCODE_0: return Key::Zero;
  case SDL_SCANCODE_RETURN: return Key::Return;
  case SDL_SCANCODE_ESCAPE: return Key::Escape;
  case SDL_SCANCODE_BACKSPACE: return Key::Backspace;
  case SDL_SCANCODE_TAB: return Key::Tab;
  case SDL_SCANCODE_SPACE: return Key::Space;
  case SDL_SCANCODE_MINUS: return Key::Minus;
  case SDL_SCANCODE_EQUALS: return Key::Equals;
  case SDL_SCANCODE_LEFTBRACKET: return Key::LeftBracket;
  case SDL_SCANCODE_RIGHTBRACKET: return Key::RightBracket;
  case SDL_SCANCODE_BACKSLASH: return Key::Backslash;
    //    case SDL_SCANCODE_NONUSHASH: return Key::Non;
  case SDL_SCANCODE_SEMICOLON: return Key::Semicolon;
  case SDL_SCANCODE_APOSTROPHE: return Key::Apostrophe;
  case SDL_SCANCODE_GRAVE: return Key::Grave;
  case SDL_SCANCODE_COMMA: return Key::Comma;
  case SDL_SCANCODE_PERIOD: return Key::Period;
  case SDL_SCANCODE_SLASH: return Key::Slash;
    //    case SDL_SCANCODE_CAPSLOCK: return Key::;
  case SDL_SCANCODE_F1: return Key::F1;
  case SDL_SCANCODE_F2: return Key::F2;
  case SDL_SCANCODE_F3: return Key::F3;
  case SDL_SCANCODE_F4: return Key::F4;
  case SDL_SCANCODE_F5: return Key::F5;
  case SDL_SCANCODE_F6: return Key::F6;
  case SDL_SCANCODE_F7: return Key::F7;
  case SDL_SCANCODE_F8: return Key::F8;
  case SDL_SCANCODE_F9: return Key::F9;
  case SDL_SCANCODE_F10: return Key::F10;
  case SDL_SCANCODE_F11: return Key::F11;
  case SDL_SCANCODE_F12: return Key::F12;
    //case SDL_SCANCODE_PRINTSCREEN: return Key::;
  case SDL_SCANCODE_SCROLLLOCK: return Key::ScrollLock;
  case SDL_SCANCODE_PAUSE: return Key::Pause;
  case SDL_SCANCODE_INSERT: return Key::Insert;
  case SDL_SCANCODE_HOME: return Key::Home;
  case SDL_SCANCODE_PAGEUP: return Key::PageUp;
  case SDL_SCANCODE_DELETE: return Key::Delete;
  case SDL_SCANCODE_END: return Key::End;
  case SDL_SCANCODE_PAGEDOWN: return Key::PageDown;
  case SDL_SCANCODE_RIGHT: return Key::ArrowRight;
  case SDL_SCANCODE_LEFT: return Key::ArrowLeft;
  case SDL_SCANCODE_DOWN: return Key::ArrowDown;
  case SDL_SCANCODE_UP: return Key::ArrowUp;
  case SDL_SCANCODE_NUMLOCKCLEAR: return Key::NumLock;
  case SDL_SCANCODE_KP_DIVIDE: return Key::Divide;
  case SDL_SCANCODE_KP_MULTIPLY: return Key::Multiply;
  case SDL_SCANCODE_KP_MINUS: return Key::Minus;
  case SDL_SCANCODE_KP_PLUS: return Key::Add;
  case SDL_SCANCODE_KP_ENTER: return Key::NumpadEnter;
  case SDL_SCANCODE_KP_1: return Key::Numpad1;
  case SDL_SCANCODE_KP_2: return Key::Numpad2;
  case SDL_SCANCODE_KP_3: return Key::Numpad3;
  case SDL_SCANCODE_KP_4: return Key::Numpad4;
  case SDL_SCANCODE_KP_5: return Key::Numpad5;
  case SDL_SCANCODE_KP_6: return Key::Numpad6;
  case SDL_SCANCODE_KP_7: return Key::Numpad7;
  case SDL_SCANCODE_KP_8: return Key::Numpad8;
  case SDL_SCANCODE_KP_9: return Key::Numpad9;
  case SDL_SCANCODE_KP_0: return Key::Numpad0;
  case SDL_SCANCODE_KP_PERIOD: return Key::Period;
    //case SDL_SCANCODE_NONUSBACKSLASH: return Key::;
  case SDL_SCANCODE_APPLICATION: return Key::AppMenu;
  case SDL_SCANCODE_LCTRL: return Key::LeftControl;
  case SDL_SCANCODE_LSHIFT: return Key::LeftShift;
  case SDL_SCANCODE_LALT: return Key::LeftAlt;
  case SDL_SCANCODE_LGUI: return Key::LeftWindows;
  case SDL_SCANCODE_RCTRL: return Key::RightControl;
  case SDL_SCANCODE_RSHIFT: return Key::RightShift;
  case SDL_SCANCODE_RALT: return Key::RightAlt;
  case SDL_SCANCODE_RGUI: return Key::RightWindows;

  case SDL_SCANCODE_UNKNOWN: // fallthrough
  default:
    return Key::Unknown;
  }
}

void Gui::init(const uvec2 & size) {
  static CEGUI::OpenGL3Renderer & myRenderer =
    CEGUI::OpenGL3Renderer::create();
  myRenderer.setDisplaySize(CEGUI::Sizef(size.x, size.y));
  CEGUI::System::create(myRenderer);
//  OpenGL3Renderer::bootstrapSystem();


  {
    DefaultResourceProvider* rp = static_cast<CEGUI::DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());
    rp->setResourceGroupDirectory("schemes", "/Users/bdavis/Git/VirtualChess/resources/cegui/schemes/");
    rp->setResourceGroupDirectory("imagesets", "/Users/bdavis/Git/VirtualChess/resources/cegui/imagesets/");
    rp->setResourceGroupDirectory("fonts", "/Users/bdavis/Git/VirtualChess/resources/cegui/fonts/");
    rp->setResourceGroupDirectory("layouts", "/Users/bdavis/Git/VirtualChess/resources/cegui/layouts/");
    rp->setResourceGroupDirectory("looknfeels", "/Users/bdavis/Git/VirtualChess/resources/cegui/looknfeel/");
    rp->setResourceGroupDirectory("lua_scripts", "/Users/bdavis/Git/VirtualChess/resources/cegui/lua_scripts/");
    //rp->setDefaultResourceGroup()
  }

  // set the default resource groups to be used
  ImageManager::setImagesetDefaultResourceGroup("imagesets");
  Font::setDefaultResourceGroup("fonts");
  Scheme::setDefaultResourceGroup("schemes");
  WidgetLookManager::setDefaultResourceGroup("looknfeels");
  WindowManager::setDefaultResourceGroup("layouts");
  // ScriptModule::setDefaultResourceGroup("lua_scripts");
  // AnimationManager::setDefaultResourceGroup("animations");

  SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
  FontManager::getSingleton().createFromFile("DejaVuSans-14.font");

  System::getSingleton().getDefaultGUIContext().setDefaultFont("DejaVuSans-14");
  System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");
}

bool Gui::handleSdlEvent(const SDL_Event & event, const vec2 & windowScaleFactor) {
  switch (event.type) {
  case SDL_MOUSEMOTION:
    System::getSingleton().getDefaultGUIContext().injectMousePosition(
      windowScaleFactor.x * event.motion.x,
      windowScaleFactor.y * event.motion.y);
    return true;

  case SDL_MOUSEWHEEL:
    System::getSingleton().getDefaultGUIContext().injectMouseWheelChange(event.wheel.y);
    return true;

  case SDL_MOUSEBUTTONDOWN:
    System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(TranslateMouseButton(event.button.button));
    return true;

  case SDL_MOUSEBUTTONUP:
    System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(TranslateMouseButton(event.button.button));
    return true;

  case SDL_KEYDOWN:
    System::getSingleton().getDefaultGUIContext().injectKeyDown(TranslateKey(event.key.keysym));
    return true;

  case SDL_KEYUP:
    System::getSingleton().getDefaultGUIContext().injectKeyUp(TranslateKey(event.key.keysym));
    return true;

  case SDL_TEXTINPUT:
    for (int i = 0; event.text.text[i] && i < sizeof(event.text.text); ++i) {
      System::getSingleton().getDefaultGUIContext().injectChar(event.text.text[i]);
    }
    return true;
  }
}