#pragma once

#include <Rocket/Core.h>
#include <Rocket/Controls.h>
#include <Rocket/Debugger.h>


struct RocketSurface {
  typedef std::shared_ptr<Rocket::Core::Context> RocketContextPtr;

  RocketContextPtr ctx;
  uvec2 size;
  oglplus::Texture      tex;
  oglplus::Framebuffer  fbo;
private:
  oglplus::Context gl;
public:
  RocketSurface(const uvec2 & size);
  void render();
};


class RocketBridge :
    public Rocket::Core::SystemInterface,
    public Rocket::Core::FileInterface,
    public Rocket::Core::RenderInterface
{
  RocketBridge();

public:
  static void init();
  static void shutdown();

  virtual ~RocketBridge();
  /// Opens a file.
  virtual Rocket::Core::FileHandle Open(const Rocket::Core::String& path);

  /// Closes a previously opened file.
  virtual void Close(Rocket::Core::FileHandle file);

  /// Reads data from a previously opened file.
  virtual size_t Read(void* buffer, size_t size, Rocket::Core::FileHandle file);

  /// Seeks to a point in a previously opened file.
  virtual bool Seek(Rocket::Core::FileHandle file, long offset, int origin);

  /// Returns the current position of the file pointer.
  virtual size_t Tell(Rocket::Core::FileHandle file);

  /// Get the number of seconds elapsed since the start of the application
  /// @returns Seconds elapsed
  virtual float GetElapsedTime();

  /// Called by Rocket when it wants to render geometry that it does not wish to optimise.
  virtual void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation);

  /// Called by Rocket when it wants to compile geometry it believes will be static for the forseeable future.
  virtual Rocket::Core::CompiledGeometryHandle CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture);

  /// Called by Rocket when it wants to render application-compiled geometry.
  virtual void RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation);
  /// Called by Rocket when it wants to release application-compiled geometry.
  virtual void ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry);

  /// Called by Rocket when it wants to enable or disable scissoring to clip content.
  virtual void EnableScissorRegion(bool enable);
  /// Called by Rocket when it wants to change the scissor region.
  virtual void SetScissorRegion(int x, int y, int width, int height);

  /// Called by Rocket when a texture is required by the library.
  virtual bool LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions, const Rocket::Core::String& source);
  /// Called by Rocket when a texture is required to be built from an internally-generated sequence of pixels.
  virtual bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source, const Rocket::Core::Vector2i& source_dimensions);
  /// Called by Rocket when a loaded texture is no longer required.
  virtual void ReleaseTexture(Rocket::Core::TextureHandle texture_handle);

  static int TranslateMouseButton(Uint8 button);
  static int GetKeyModifiers();
  static Rocket::Core::Input::KeyIdentifier TranslateKey(SDL_Keycode sdlkey);
};

