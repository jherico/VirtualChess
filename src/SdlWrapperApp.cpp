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

int getSdlDisplayAtPosition(const ivec2 & pos, uvec2 & displaySize) {
  int numDisplays = SDL_GetNumVideoDisplays();
  for (int i = 0; i < numDisplays; ++i) {
    SDL_Rect bounds;
    SDL_GetDisplayBounds(i, &bounds);
    if (bounds.x == pos.x && bounds.y == pos.y) {
      displaySize.x = bounds.w;
      displaySize.y = bounds.h;
      return i;
    }
  }
  return -1;
}

void  myGlDebugCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar * message,
    void * userParam) {
  const char * typeStr = "?";
  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
    typeStr = "ERROR";
    break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    typeStr = "DEPRECATED_BEHAVIOR";
    break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    typeStr = "UNDEFINED_BEHAVIOR";
    break;
  case GL_DEBUG_TYPE_PORTABILITY:
    typeStr = "PORTABILITY";
    break;
  case GL_DEBUG_TYPE_PERFORMANCE:
    typeStr = "PERFORMANCE";
    break;
  case GL_DEBUG_TYPE_OTHER:
    typeStr = "OTHER";
    break;
  }

  const char * severityStr = "?";
  switch (severity) {
  case GL_DEBUG_SEVERITY_LOW:
    severityStr = "LOW";
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    severityStr = "MEDIUM";
    break;
  case GL_DEBUG_SEVERITY_HIGH:
    severityStr = "HIGH";
    break;
  }
  SAY("--- OpenGL Callback Message ---");
  SAY("type: %s\nseverity: %-8s\nid: %d\nmsg: %s", typeStr, severityStr, id,
      message);
  SAY("--- OpenGL Callback Message ---");
}
