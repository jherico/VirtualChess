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

#pragma once


void GLAPIENTRY myGlDebugCallback(GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar * message,
  void * userParam);

int getSdlDisplayAtPosition(const ivec2 & pos, uvec2 & displaySize);

template <class T, class ARGS>
class SdlWrapperApp {
protected:
  SDL_Window*   window;
  uvec2         windowSize;
  ivec2         windowPosition;

public:
  SdlWrapperApp() {
    if (SDL_Init(SDL_INIT_EVERYTHING)<0){
      FAIL("Error initializing SDL");
    }
  }

  virtual ~SdlWrapperApp() {
    SDL_Quit();
  }

  virtual SDL_Window * createWindow() = 0;

  virtual ARGS getArgs() = 0;

  virtual int run() {
    window = createWindow();
    onCreate();
    SDL_GLContext context = SDL_GL_CreateContext(window);
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    glewExperimental = true;
    glewInit();
    SDL_GL_SetSwapInterval(1);

    T wrapped(getArgs());
    while (!wrapped.isDone()) {
      wrapped.onTick();
    }
    return 0;
  }

  virtual void onCreate() {
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    GLuint unusedIds = 0;
    if (glDebugMessageCallback) {
      glDebugMessageCallback(myGlDebugCallback, this);
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
        0, &unusedIds, true);
    }
    else if (glDebugMessageCallbackARB) {
      glDebugMessageCallbackARB(myGlDebugCallback, this);
      glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
        0, &unusedIds, true);
    }
    GL_CHECK_ERROR;
  }

};
