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

#include <regex>

#ifdef __APPLE__
#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreGraphics/CGDisplayConfiguration.h>
#endif

void  APIENTRY myGlDebugCallback(
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

#if 0
void GlfwApp::onCreate() {
  windowAspect = aspect(windowSize);
  windowAspectInverse = 1.0f / windowAspect;
  glfwSetWindowUserPointer(window, this);
  glfwSetKeyCallback(window, glfwKeyCallback);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

// Initialize the OpenGL bindings
// For some reason we have to set this experminetal flag to properly
// init GLEW if we use a core context.
  glewExperimental = GL_TRUE;
  if (0 != glewInit()) {
    FAIL("Failed to initialize GL3W");
  }
  glGetError();
  GL_CHECK_ERROR;
}

void GlfwApp::preCreate() {
  glfwWindowHint(GLFW_DEPTH_BITS, 16);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
#ifdef RIFT_DEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
}

void GlfwApp::createWindow(const uvec2 & size, const ivec2 & position) {
  windowSize = size;
  windowPosition = position;
  preCreate();
  window = glfwCreateWindow(size.x, size.y, "glfw", nullptr, nullptr);
  if (!window) {
    FAIL("Unable to create rendering window");
  }
  if ((position.x > INT_MIN) && (position.y > INT_MIN)) {
    glfwSetWindowPos(window, position.x, position.y);
  }
  onCreate();
}

void GlfwApp::createFullscreenWindow(const uvec2 & size, GLFWmonitor * monitor) {
  windowSize = size;
  preCreate();
  const GLFWvidmode * currentMode = glfwGetVideoMode(monitor);
  window = glfwCreateWindow(windowSize.x, windowSize.y, "glfw", monitor, nullptr);
  assert(window != 0);
  onCreate();
}

void GlfwApp::initGl() {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glDisable(GL_DITHER);
  glEnable(GL_DEPTH_TEST);
  GL_CHECK_ERROR;
}

void GlfwApp::finishFrame() {
  glfwSwapBuffers(window);
}

void GlfwApp::destroyWindow() {
  glfwSetKeyCallback(window, nullptr);
  glfwDestroyWindow(window);
}

GlfwApp::~GlfwApp() {
  glfwTerminate();
}

void GlfwApp::screenshot() {

#ifdef HAVE_OPENCV
  //use fast 4-byte alignment (default anyway) if possible
  glFlush();
  cv::Mat img(windowSize.y, windowSize.x, CV_8UC3);
  glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);
  glPixelStorei(GL_PACK_ROW_LENGTH, img.step / img.elemSize());
  glReadPixels(0, 0, img.cols, img.rows, GL_BGR, GL_UNSIGNED_BYTE, img.data);
  cv::flip(img, img, 0);

  static int counter = 0;
  static char buffer[128];
  sprintf(buffer, "screenshot%05i.png", counter++);
  bool success = cv::imwrite(buffer, img);
  if (!success) {
    throw std::runtime_error("Failed to write image");
  }
#endif
}

int GlfwApp::run() {
  createRenderingTarget();

  if (!window) {
    FAIL("Unable to create OpenGL window");
  }

  initGl();

  int framecount = 0;
  long start = Platform::elapsedMillis();
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    update();
    draw();
    finishFrame();
    long now = Platform::elapsedMillis();
    ++framecount;
    if ((now - start) >= 2000) {
      float elapsed = (now - start) / 1000.f;
      fps = (float) framecount / elapsed;
      SAY("FPS: %0.2f", fps);
      start = now;
      framecount = 0;
    }
  }
  glfwDestroyWindow(window);
  return 0;
}

void GlfwApp::onKey(int key, int scancode, int action, int mods) {
  if (GLFW_PRESS != action) {
    return;
  }

  switch (key) {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, 1);
    return;

#ifdef HAVE_OPENCV
    case GLFW_KEY_S:
    if (mods & GLFW_MOD_SHIFT) {
      screenshot();
    }
    return;
#endif
  }
}

GLFWmonitor * GlfwApp::getMonitorAtPosition(const ivec2 & position) {
  int count;
  GLFWmonitor ** monitors = glfwGetMonitors(&count);
  for (int i = 0; i < count; ++i) {
    ivec2 candidatePosition;
    glfwGetMonitorPos(monitors[i], &candidatePosition.x, &candidatePosition.y);
    if (candidatePosition == position) {
      return monitors[i];
    }
  }
  return nullptr;
}
#endif

/*
#define MK(X, Y, Z) \
X Y; memset(&Y, 0, sizeof(X)); Y.Z = sizeof(X);
bool GlfwApp::getCurrentResolution(const std::string & displayName, long displayId, ivec2 & size, ivec2 & position) {
  int count;
  // Try a cross-platform mechanism for fetching the information
  GLFWmonitor ** monitors = glfwGetMonitors(&count);
  for (int i = 0; i < count; ++i) {
    std::string monitorName = glfwGetMonitorName(monitors[i]);
//    std::wstring deviceName = glfwGetWin32DeviceName(monitors[i]);
    if (monitorName == displayName) {
      glfwGetMonitorPos(monitors[i], &position.x, &position.y);
      const GLFWvidmode * mode = glfwGetVideoMode(monitors[i]);
      size.x = mode->width;
      size.y = mode->height;
      return true;
    }
  }

#ifdef WIN32
  MK(DISPLAY_DEVICEA, dd, cb);
  DWORD deviceNum = 0;
  while (EnumDisplayDevicesA(NULL, deviceNum, &dd, 0)){
    MK(DEVMODEA, deviceMode, dmSize);
    EnumDisplaySettingsExA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode, 0);

    if (displayName == std::string(dd.DeviceName)) {
      size.x = deviceMode.dmPelsWidth;
      size.y = deviceMode.dmPelsHeight;
      position.x = deviceMode.dmPosition.x;
      position.y = deviceMode.dmPosition.y;
      return true;
    }

    MK(DISPLAY_DEVICEA, dd2, cb);
    DWORD monitorNum = 0;
    while (EnumDisplayDevicesA(dd.DeviceName, monitorNum, &dd2, 0)) {
      if (displayName == std::string(dd2.DeviceName)) {
        size.x = deviceMode.dmPelsWidth;
        size.y = deviceMode.dmPelsHeight;
        position.x = deviceMode.dmPosition.x;
        position.y = deviceMode.dmPosition.y;
        return true;
      }
      monitorNum++;
    }
    deviceNum++;
  }
#elif __APPLE__
    uint32_t monitorCount;
    CGGetOnlineDisplayList(0, NULL, &monitorCount);
    std::vector<CGDirectDisplayID> displays(monitorCount);
    CGGetOnlineDisplayList(monitorCount, &displays[0], &monitorCount);
    for (int i = 0;  i < monitorCount;  i++) {
        if (displayId == displays[i]) {
            CGRect rect = CGDisplayBounds (displays[i]);
            size.x = rect.size.width;
            size.y = rect.size.height;
            position.x = rect.origin.x;
            position.y = rect.origin.y;
            return true;
        }
    }
#else
#endif
  return false;
}
*/
