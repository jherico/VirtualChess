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


#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#if !defined(_DEBUG)
#undef RIFT_DEBUG
#endif
#endif

#define __STDC_FORMAT_MACROS 1

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdarg>
#include <cstdio>

#include <algorithm>
#include <array>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <stack>
#include <sstream>
#include <unordered_map>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/circular_buffer.hpp>


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/norm.hpp>

using glm::ivec3;
using glm::ivec2;
using glm::uvec2;

using glm::mat3;
using glm::mat4;

using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::quat;

#include <GL/glew.h>

#pragma warning( disable : 4068 4244)
#include <oglplus/all.hpp>
#include <oglplus/interop/glm.hpp>
#include <oglplus/bound/texture.hpp>
#include <oglplus/bound/framebuffer.hpp>
#include <oglplus/bound/renderbuffer.hpp>
#include <oglplus/bound/buffer.hpp>
#pragma warning( default : 4068 4244)

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#include <SDL.h>

#include <Resources.h>

typedef std::shared_ptr<void*> VoidPtr;

template<class T>
class circular_buffer : public std::list<T>{
  size_t max;
public:
  circular_buffer(size_t max) : max(max) {
  }
  void push_back(const T & t) {
    std::list<T>::push_back(t);
    while (std::list<T>::size() > max) {
      std::list<T>::pop_front();
    }
  }
};


class Platform {
public:
    static void sleepMillis(int millis);
    static long elapsedMillis();
    static float elapsedSeconds();
    static void fail(const char * file, int line, const char * message, ...);
    static void say(std::ostream & out, const char * message, ...);
    static std::string format(const char * formatString, ...);
    static std::string getResourceString(Resource resource);
    static std::vector<uint8_t> getResourceVector(Resource resource);
    static std::string replaceAll(const std::string & in, const std::string & from, const std::string & to);
};

#ifndef PI
#define PI 3.14159265f
#endif

#ifndef HALF_PI
#define HALF_PI (PI / 2.0f)
#endif

#ifndef TWO_PI
#define TWO_PI (PI * 2.0f)
#endif

#ifndef RADIANS_TO_DEGREES
#define RADIANS_TO_DEGREES (180.0f / PI)
#endif

#ifndef DEGREES_TO_RADIANS
#define DEGREES_TO_RADIANS (PI / 180.0f)
#endif

// Windows has a non-standard main function prototype
#ifdef WIN32
    #define MAIN_DECL int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
    #define MAIN_DECL int main(int argc, char ** argv)
#endif

#define FAIL(...) Platform::fail(__FILE__, __LINE__, __VA_ARGS__)
#define SAY(...) Platform::say(std::cout, __VA_ARGS__)
#define SAY_ERR(...) Platform::say(std::cerr, __VA_ARGS__)

// Combine some macros together to create a single macro
// to run an example app
#define RUN_APP(AppClass) \
    MAIN_DECL { \
        try { \
            return AppClass().run(); \
        } catch (std::exception & error) { \
            SAY_ERR(error.what()); \
        } catch (const std::string & error) { \
            SAY_ERR(error.c_str()); \
        } \
        return -1; \
    }

template <typename T>
inline float aspect(T const & size) {
  return (float)size.x / (float)size.y;
}

// Platform config
#include "Config.h"

// First order dependencies
#include "Colors.h"
#include "Files.h"
#include "Interaction.h"
#include "Stacks.h"
#include "SocketClient.h"
#include "GlDebug.h"

// Second order dependencies
#include "Mesh.h"

// Thord order
#include "GlUtils.h"
#include "RenderUtils.h"
#include "SdlWrapperApp.h"
#include "Rift.h"

