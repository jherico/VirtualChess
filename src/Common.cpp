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

#include <stdio.h>
#include <stdarg.h>
#include "Common.h"
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

void Platform::sleepMillis(int millis) {
#ifdef WIN32
  Sleep(millis);
#else
  usleep(millis * 1000);
#endif
}

long Platform::elapsedMillis() {
#ifdef WIN32
  static long start = GetTickCount();
  return GetTickCount() - start;
#else
  timeval time;
  gettimeofday(&time, NULL);
  long millis = (time.tv_sec * 1000) + (time.tv_usec / 1000);
  static long start = millis;
  return millis - start;
#endif
}

#ifndef WIN32
#define MAX_PATH 4096
#endif

std::string Platform::getResourcePath(Resource resource) {
  char path[MAX_PATH + 1];
  Resources::getResourcePath(resource, path, MAX_PATH);
  std::string result = path;
  return result;
}

float Platform::elapsedSeconds() {
  return (float)elapsedMillis() / 1000.0f;
}

static const size_t BUFFER_SIZE = 8192;


std::string formatVarargs(const char * fmt_str, va_list ap) {
  va_list argcopy;
  va_copy(argcopy, ap);
  size_t size = vsnprintf(nullptr, 0, fmt_str, argcopy);
  va_end(argcopy);

  char * output = new char[size * 2];
  size = vsnprintf(output, size * 2, fmt_str, ap);
  std::string result;
  result.assign(output, size);
  delete [] output;
  return result;
}

// If you got here, something's pretty wrong
void Platform::fail(const char * file, int line, const char * message, ...) {
  va_list arg;
  va_start(arg, message);
  std::string string = formatVarargs(message, arg);
  va_end(arg);
  std::string error = format("FATAL %s (%d): %s", file, line, string.c_str());
  std::cerr << error << std::endl;
#ifdef WIN32
  if (NULL == GetConsoleWindow()) {
    MessageBoxA(NULL, error.c_str(), "Message", IDOK | MB_ICONERROR);
  }
  DebugBreak();
#endif
  // assert(0);
  throw std::runtime_error(error.c_str());
}

void Platform::say(std::ostream & out, const char * message, ...) {
  va_list arg;
  va_start(arg, message);
  std::string formatted = formatVarargs(message, arg);
  va_end(arg);
#ifdef WIN32
  OutputDebugStringA(formatted.c_str());
  OutputDebugStringA("\n");
#endif
  out << formatted << std::endl;
}

std::string Platform::format(const char * fmt_str, ...) {
    va_list ap;
    va_start(ap, fmt_str);
    std::string result = formatVarargs(fmt_str, ap);
    va_end(ap);
    return result;
}

std::string Platform::getResourceString(Resource resource) {
  size_t size = Resources::getResourceSize(resource);
  char * data = new char[size];
  Resources::getResourceData(resource, data);
  std::string dataStr(data, size);
  delete[] data;
  return dataStr;
}

time_t Platform::getResourceModified(Resource resource) {
  return Resources::getResourceModified(resource);
}

std::vector<uint8_t> Platform::getResourceVector(Resource resource) {
  size_t size = Resources::getResourceSize(resource);
  std::vector<uint8_t> result;
  result.resize(size);
  Resources::getResourceData(resource, &(result[0]));
  return result;
}


