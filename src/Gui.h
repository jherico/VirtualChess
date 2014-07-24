#pragma once

class Gui {
  Gui();
public:
  static void init(const uvec2 & size);
  static bool handleSdlEvent(const SDL_Event & event, const vec2 & windowScaleFactor);

};
