#include "Common.h"
#include "Rocket.h"

#include <Rocket/Core/FontDatabase.h>

using namespace oglplus;


using namespace oglplus;
struct CompiledGeometry {
  oglplus::VertexArray vao;
  oglplus::Buffer buffer;
  oglplus::Buffer indexBuffer;
  int count;
  Rocket::Core::TextureHandle texture;
};

Resource FONT_NAMES[] = {
  Resource::FONTS_DELICIOUS_ROMAN_OTF,
  Resource::FONTS_DELICIOUS_BOLD_OTF,
  Resource::FONTS_DELICIOUS_ITALIC_OTF,
  Resource::FONTS_DELICIOUS_BOLDITALIC_OTF,
  Resource::FONTS_BITWISE_OTF,
  Resource::FONTS_MODER_DOS_437_OTF,
};

static std::shared_ptr<RocketBridge> INSTANCE;

RocketSurface::RocketSurface(const uvec2 & size) : size(size) {
  RocketBridge::init();

  ctx.reset(Rocket::Core::CreateContext("main",
    Rocket::Core::Vector2i(size.x, size.y)));
  Rocket::Debugger::Initialise(ctx.get());

  using namespace oglplus;
  gl.Bound(Texture::Target::_2D, tex)
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


void RocketSurface::render() {
  using namespace oglplus;
  fbo.Bind(Framebuffer::Target::Draw);
  gl.ClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  gl.Clear().ColorBuffer();
  gl.Viewport(size.x, size.y);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  Stacks::withPush([&]{
    MatrixStack & pr = Stacks::projection();
    pr.top() = glm::ortho((float)0, (float)size.x, (float)size.y, (float)0);
    ctx->Update();
    ctx->Render();
  });
  DefaultFramebuffer().Bind(Framebuffer::Target::Draw);
}

void RocketBridge::init() {
  if (nullptr == INSTANCE) {
    INSTANCE.reset(new RocketBridge());
  }
}

void RocketBridge::shutdown() {
  INSTANCE = nullptr;
}

/// Loads the default fonts from the given path.
RocketBridge::RocketBridge() {
  Rocket::Core::SetSystemInterface(this);
  Rocket::Core::SetFileInterface(this);
  Rocket::Core::SetRenderInterface(this);
  Rocket::Core::Initialise();
  Rocket::Controls::Initialise();
  

  for (int i = 0; i < sizeof(FONT_NAMES) / sizeof(Resource); i++) {
    std::string fontPath = Platform::format("+%d", FONT_NAMES[i]);
    Rocket::Core::FontDatabase::LoadFontFace(fontPath.c_str());
  }
}

RocketBridge::~RocketBridge() {
  Rocket::Core::Shutdown();
  Rocket::Core::SetRenderInterface(nullptr);
  Rocket::Core::SetFileInterface(nullptr);
  Rocket::Core::SetSystemInterface(nullptr);
}

// Opens a file.
Rocket::Core::FileHandle RocketBridge::Open(const Rocket::Core::String& path)
{
  if (!path.Empty() && '+' == path[0]) {
    Resource res = static_cast<Resource>(atoi(path.Substring(0).CString()));
    return (Rocket::Core::FileHandle)new std::istringstream(Platform::getResourceString(res));
  } else {
    return (Rocket::Core::FileHandle)new std::ifstream(path.CString());
  }
  return -1;
}

// Closes a previously opened file.
void RocketBridge::Close(Rocket::Core::FileHandle file)
{
  std::istream * stream = (std::istream*)file;
  delete stream;
}

// Reads data from a previously opened file.
size_t RocketBridge::Read(void* buffer, size_t size, Rocket::Core::FileHandle file)
{
  std::istream & stream = *(std::istream*)file;
  size_t result = 0;
  char * charBuffer = (char*)buffer;
  while (stream && !stream.eof()) {
    stream.read(charBuffer++, 1);
    result++;
  }
  return result;
}

// Seeks to a point in a previously opened file.
bool RocketBridge::Seek(Rocket::Core::FileHandle file, long offset, int origin)
{
  std::istream & stream = *(std::istream*)file;
  std::ios_base::seekdir way;
  switch (origin) {
  case SEEK_CUR: way = std::ios_base::cur; break;
  case SEEK_END: way = std::ios_base::end; break;
  case SEEK_SET: way = std::ios_base::beg; break;
  }
  stream.seekg(offset, way);
  return true;
}

// Returns the current position of the file pointer.
size_t RocketBridge::Tell(Rocket::Core::FileHandle file)
{
  std::istream & stream = *(std::istream*)file;
  return stream.tellg();
}

// Get the number of seconds elapsed since the start of the application
float RocketBridge::GetElapsedTime()
{
  return (float)Platform::elapsedMillis() / 1000.0f;
}



static const size_t VERTEX_STRIDE = sizeof(Rocket::Core::Vertex);
static const void* VERTEX_COLOR_OFFSET = (void*)offsetof(Rocket::Core::Vertex, colour);
static const void* VERTEX_TEX_OFFSET = (void*)offsetof(Rocket::Core::Vertex, tex_coord);

//static const size_t VERTEX_STRIDE = sizeof(vec4) * 3;
//static const void* VERTEX_COLOR_OFFSET = (void*)sizeof(vec4);
//static const void* VERTEX_TEX_OFFSET = (void*)(sizeof(vec4) * 2);

void compileGeometry(CompiledGeometry & cg,
    BufferUsage usage,
    Rocket::Core::Vertex* vertices,
    int num_vertices,
    int* indices,
    int num_indices,
    const Rocket::Core::TextureHandle texture
) {
  cg.texture = texture;
  cg.count = num_indices;

  cg.buffer.Bind(Buffer::Target::Array);
  //Buffer::Data(Buffer::Target::Array, num_vertices, &data[0], usage);
  Buffer::Data(Buffer::Target::Array, num_vertices, vertices, usage);
  cg.indexBuffer.Bind(Buffer::Target::ElementArray);
  Buffer::Data(Buffer::Target::ElementArray, num_indices, indices, usage);

  cg.vao.Bind();
  VertexArrayAttrib(Layout::Attribute::Position).
    Pointer(2, DataType::Float, false, VERTEX_STRIDE, 0).
    Enable();
  VertexArrayAttrib(Layout::Attribute::Color).
    Pointer(4, DataType::Byte, false, VERTEX_STRIDE, VERTEX_COLOR_OFFSET).
    Enable();
  VertexArrayAttrib(Layout::Attribute::TexCoord0).
    Pointer(2, DataType::Float, false, VERTEX_STRIDE, VERTEX_TEX_OFFSET).
    Enable();
  NoVertexArray().Bind();
  NoBuffer().Bind(Buffer::Target::Array);
  NoBuffer().Bind(Buffer::Target::ElementArray);
}


//#define GL_CLAMP_TO_EDGE 0x812F


// Called by Rocket when it wants to render geometry that it does not wish to optimise.
void RocketBridge::RenderGeometry(
    Rocket::Core::Vertex* vertices,
    int num_vertices,
    int* indices,
    int num_indices,
    const Rocket::Core::TextureHandle texture,
    const Rocket::Core::Vector2f& translation)
{
  static CompiledGeometry cg;
  Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "Rendered Direct.");
  compileGeometry(cg, BufferUsage::StreamDraw, vertices, num_vertices, indices, num_indices, texture);
  RenderCompiledGeometry((Rocket::Core::CompiledGeometryHandle)&cg, translation);
}

// Called by Rocket when it wants to compile geometry it believes will be static for the forseeable future.
Rocket::Core::CompiledGeometryHandle RocketBridge::CompileGeometry(
    Rocket::Core::Vertex* vertices,
    int num_vertices,
    int* indices,
    int num_indices,
    const Rocket::Core::TextureHandle texture)
{
//  Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "Compiled Geometry.");
  CompiledGeometry * cg = new CompiledGeometry();
  compileGeometry(*cg, BufferUsage::StaticDraw, vertices, num_vertices, indices, num_indices, texture);
  return (Rocket::Core::CompiledGeometryHandle)cg;
}


// Called by Rocket when it wants to render application-compiled geometry.
void RocketBridge::RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation)
{
  CompiledGeometry & cg = *(CompiledGeometry*)geometry;
  Program & prog = GlUtils::getProgram(
    cg.texture ? Resource::SHADERS_TEXTURED_VS : Resource::SHADERS_COLORED_VS,
    cg.texture ? Resource::SHADERS_TEXTURED_FS : Resource::SHADERS_COLORED_FS
  );
  prog.Use();
  Stacks::withPush([&]{
    MatrixStack & mv = Stacks::modelview();
    mv.identity();
    mv.translate(vec3(translation.x, translation.y, 0));
    SET_MODELVIEW(prog);
    SET_PROJECTION(prog);
  });
  cg.vao.Bind();
  cg.indexBuffer.Bind(Buffer::Target::ElementArray);
  if (cg.texture) {
    glBindTexture(GL_TEXTURE_2D, cg.texture);
  }
  glDrawElements(GL_TRIANGLES, cg.count, GL_UNSIGNED_INT, 0);
  if (cg.texture) {
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  NoVertexArray().Bind();
  NoBuffer().Bind(Buffer::Target::ElementArray);
  NoProgram().Use();
}

// Called by Rocket when it wants to release application-compiled geometry.
void RocketBridge::ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry)
{
  delete (CompiledGeometry*)geometry;
}

// Called by Rocket when it wants to enable or disable scissoring to clip content.
void RocketBridge::EnableScissorRegion(bool enable)
{
    if (enable)
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);
}

// Called by Rocket when it wants to change the scissor region.
void RocketBridge::SetScissorRegion(int x, int y, int width, int height)
{
    //glScissor(x, m_height - (y + height), width, height);
}

// Set to byte packing, or the compiler will expand our struct, which means it won't read correctly from file
#pragma pack(1)
struct TGAHeader
{
    char  idLength;
    char  colourMapType;
    char  dataType;
    short int colourMapOrigin;
    short int colourMapLength;
    char  colourMapDepth;
    short int xOrigin;
    short int yOrigin;
    short int width;
    short int height;
    char  bitsPerPixel;
    char  imageDescriptor;
};
// Restore packing
#pragma pack()

// Called by Rocket when a texture is required by the library.
bool RocketBridge::LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions, const Rocket::Core::String& source)
{
    Rocket::Core::FileInterface* file_interface = Rocket::Core::GetFileInterface();
    Rocket::Core::FileHandle file_handle = file_interface->Open(source);
    if (!file_handle)
    {
        return false;
    }

    file_interface->Seek(file_handle, 0, SEEK_END);
    size_t buffer_size = file_interface->Tell(file_handle);
    file_interface->Seek(file_handle, 0, SEEK_SET);

    char* buffer = new char[buffer_size];
    file_interface->Read(buffer, buffer_size, file_handle);
    file_interface->Close(file_handle);

    TGAHeader header;
    memcpy(&header, buffer, sizeof(TGAHeader));

    int color_mode = header.bitsPerPixel / 8;
    int image_size = header.width * header.height * 4; // We always make 32bit textures

    if (header.dataType != 2)
    {
        Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "Only 24/32bit uncompressed TGAs are supported.");
        return false;
    }

    // Ensure we have at least 3 colors
    if (color_mode < 3)
    {
        Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "Only 24 and 32bit textures are supported");
        return false;
    }

    const char* image_src = buffer + sizeof(TGAHeader);
    unsigned char* image_dest = new unsigned char[image_size];

    // Targa is BGR, swap to RGB and flip Y axis
    for (long y = 0; y < header.height; y++)
    {
        long read_index = y * header.width * color_mode;
        long write_index = ((header.imageDescriptor & 32) != 0) ? read_index : (header.height - y - 1) * header.width * color_mode;
        for (long x = 0; x < header.width; x++)
        {
          image_dest[write_index] = image_src[read_index + 2];
          image_dest[write_index + 1] = image_src[read_index+1];
          image_dest[write_index + 2] = image_src[read_index];
          //image_dest[write_index] = x % 2 ? 255 : 0;// image_src[read_index + 2];
          //image_dest[write_index + 1] = y % 2 ? 255 : 0; //image_src[read_index+1];
          //image_dest[write_index + 2] = 0; // image_src[read_index];
            if (color_mode == 4)
                image_dest[write_index+3] = image_src[read_index+3];
            else
                image_dest[write_index+3] = 255;

            write_index += 4;
            read_index += color_mode;
        }
    }

    texture_dimensions.x = header.width;
    texture_dimensions.y = header.height;

    bool success = GenerateTexture(texture_handle, image_dest, texture_dimensions);

    delete [] image_dest;
    delete [] buffer;

    return success;
}

// Called by Rocket when a texture is required to be built from an internally-generated sequence of pixels.
bool RocketBridge::GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source, const Rocket::Core::Vector2i& source_dimensions)
{
    GLuint texture_id = 0;
    glGenTextures(1, &texture_id);
    if (texture_id == 0)
    {
        printf("Failed to generate textures\n");
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, source_dimensions.x, source_dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, source);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    texture_handle = (Rocket::Core::TextureHandle) texture_id;
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

// Called by Rocket when a loaded texture is no longer required.
void RocketBridge::ReleaseTexture(Rocket::Core::TextureHandle texture_handle)
{
    glDeleteTextures(1, (GLuint*) &texture_handle);
}

Rocket::Core::Input::KeyIdentifier RocketBridge::TranslateKey(SDL_Keycode sdlkey)
{
  using namespace Rocket::Core::Input;


  switch (sdlkey) {
  case SDLK_UNKNOWN:
    return KI_UNKNOWN;
    break;
  case SDLK_SPACE:
    return KI_SPACE;
    break;
  case SDLK_0:
    return KI_0;
    break;
  case SDLK_1:
    return KI_1;
    break;
  case SDLK_2:
    return KI_2;
    break;
  case SDLK_3:
    return KI_3;
    break;
  case SDLK_4:
    return KI_4;
    break;
  case SDLK_5:
    return KI_5;
    break;
  case SDLK_6:
    return KI_6;
    break;
  case SDLK_7:
    return KI_7;
    break;
  case SDLK_8:
    return KI_8;
    break;
  case SDLK_9:
    return KI_9;
    break;
  case SDLK_a:
    return KI_A;
    break;
  case SDLK_b:
    return KI_B;
    break;
  case SDLK_c:
    return KI_C;
    break;
  case SDLK_d:
    return KI_D;
    break;
  case SDLK_e:
    return KI_E;
    break;
  case SDLK_f:
    return KI_F;
    break;
  case SDLK_g:
    return KI_G;
    break;
  case SDLK_h:
    return KI_H;
    break;
  case SDLK_i:
    return KI_I;
    break;
  case SDLK_j:
    return KI_J;
    break;
  case SDLK_k:
    return KI_K;
    break;
  case SDLK_l:
    return KI_L;
    break;
  case SDLK_m:
    return KI_M;
    break;
  case SDLK_n:
    return KI_N;
    break;
  case SDLK_o:
    return KI_O;
    break;
  case SDLK_p:
    return KI_P;
    break;
  case SDLK_q:
    return KI_Q;
    break;
  case SDLK_r:
    return KI_R;
    break;
  case SDLK_s:
    return KI_S;
    break;
  case SDLK_t:
    return KI_T;
    break;
  case SDLK_u:
    return KI_U;
    break;
  case SDLK_v:
    return KI_V;
    break;
  case SDLK_w:
    return KI_W;
    break;
  case SDLK_x:
    return KI_X;
    break;
  case SDLK_y:
    return KI_Y;
    break;
  case SDLK_z:
    return KI_Z;
    break;
  case SDLK_SEMICOLON:
    return KI_OEM_1;
    break;
  case SDLK_PLUS:
    return KI_OEM_PLUS;
    break;
  case SDLK_COMMA:
    return KI_OEM_COMMA;
    break;
  case SDLK_MINUS:
    return KI_OEM_MINUS;
    break;
  case SDLK_PERIOD:
    return KI_OEM_PERIOD;
    break;
  case SDLK_SLASH:
    return KI_OEM_2;
    break;
  case SDLK_BACKQUOTE:
    return KI_OEM_3;
    break;
  case SDLK_LEFTBRACKET:
    return KI_OEM_4;
    break;
  case SDLK_BACKSLASH:
    return KI_OEM_5;
    break;
  case SDLK_RIGHTBRACKET:
    return KI_OEM_6;
    break;
  case SDLK_QUOTEDBL:
    return KI_OEM_7;
    break;
  case SDLK_KP_0:
    return KI_NUMPAD0;
    break;
  case SDLK_KP_1:
    return KI_NUMPAD1;
    break;
  case SDLK_KP_2:
    return KI_NUMPAD2;
    break;
  case SDLK_KP_3:
    return KI_NUMPAD3;
    break;
  case SDLK_KP_4:
    return KI_NUMPAD4;
    break;
  case SDLK_KP_5:
    return KI_NUMPAD5;
    break;
  case SDLK_KP_6:
    return KI_NUMPAD6;
    break;
  case SDLK_KP_7:
    return KI_NUMPAD7;
    break;
  case SDLK_KP_8:
    return KI_NUMPAD8;
    break;
  case SDLK_KP_9:
    return KI_NUMPAD9;
    break;
  case SDLK_KP_ENTER:
    return KI_NUMPADENTER;
    break;
  case SDLK_KP_MULTIPLY:
    return KI_MULTIPLY;
    break;
  case SDLK_KP_PLUS:
    return KI_ADD;
    break;
  case SDLK_KP_MINUS:
    return KI_SUBTRACT;
    break;
  case SDLK_KP_PERIOD:
    return KI_DECIMAL;
    break;
  case SDLK_KP_DIVIDE:
    return KI_DIVIDE;
    break;
  case SDLK_KP_EQUALS:
    return KI_OEM_NEC_EQUAL;
    break;
  case SDLK_BACKSPACE:
    return KI_BACK;
    break;
  case SDLK_TAB:
    return KI_TAB;
    break;
  case SDLK_CLEAR:
    return KI_CLEAR;
    break;
  case SDLK_RETURN:
    return KI_RETURN;
    break;
  case SDLK_PAUSE:
    return KI_PAUSE;
    break;
  case SDLK_CAPSLOCK:
    return KI_CAPITAL;
    break;
  case SDLK_PAGEUP:
    return KI_PRIOR;
    break;
  case SDLK_PAGEDOWN:
    return KI_NEXT;
    break;
  case SDLK_END:
    return KI_END;
    break;
  case SDLK_HOME:
    return KI_HOME;
    break;
  case SDLK_LEFT:
    return KI_LEFT;
    break;
  case SDLK_UP:
    return KI_UP;
    break;
  case SDLK_RIGHT:
    return KI_RIGHT;
    break;
  case SDLK_DOWN:
    return KI_DOWN;
    break;
  case SDLK_INSERT:
    return KI_INSERT;
    break;
  case SDLK_DELETE:
    return KI_DELETE;
    break;
  case SDLK_HELP:
    return KI_HELP;
    break;
  case SDLK_F1:
    return KI_F1;
    break;
  case SDLK_F2:
    return KI_F2;
    break;
  case SDLK_F3:
    return KI_F3;
    break;
  case SDLK_F4:
    return KI_F4;
    break;
  case SDLK_F5:
    return KI_F5;
    break;
  case SDLK_F6:
    return KI_F6;
    break;
  case SDLK_F7:
    return KI_F7;
    break;
  case SDLK_F8:
    return KI_F8;
    break;
  case SDLK_F9:
    return KI_F9;
    break;
  case SDLK_F10:
    return KI_F10;
    break;
  case SDLK_F11:
    return KI_F11;
    break;
  case SDLK_F12:
    return KI_F12;
    break;
  case SDLK_F13:
    return KI_F13;
    break;
  case SDLK_F14:
    return KI_F14;
    break;
  case SDLK_F15:
    return KI_F15;
    break;
  case SDLK_NUMLOCKCLEAR:
    return KI_NUMLOCK;
    break;
  case SDLK_SCROLLLOCK:
    return KI_SCROLL;
    break;
  case SDLK_LSHIFT:
    return KI_LSHIFT;
    break;
  case SDLK_RSHIFT:
    return KI_RSHIFT;
    break;
  case SDLK_LCTRL:
    return KI_LCONTROL;
    break;
  case SDLK_RCTRL:
    return KI_RCONTROL;
    break;
  case SDLK_LALT:
    return KI_LMENU;
    break;
  case SDLK_RALT:
    return KI_RMENU;
    break;
  case SDLK_LGUI:
    return KI_LMETA;
    break;
  case SDLK_RGUI:
    return KI_RMETA;
    break;
    /*case SDLK_LSUPER:
    return KI_LWIN;
    break;
    case SDLK_RSUPER:
    return KI_RWIN;
    break;*/
  default:
    return KI_UNKNOWN;
    break;
  }
}

int RocketBridge::TranslateMouseButton(Uint8 button)
{
  switch (button)
  {
  case SDL_BUTTON_LEFT:
    return 0;
  case SDL_BUTTON_RIGHT:
    return 1;
  case SDL_BUTTON_MIDDLE:
    return 2;
  default:
    return 3;
  }
}

int RocketBridge::GetKeyModifiers()
{
  SDL_Keymod sdlMods = SDL_GetModState();

  int retval = 0;

  if (sdlMods & KMOD_CTRL)
    retval |= Rocket::Core::Input::KM_CTRL;

  if (sdlMods & KMOD_SHIFT)
    retval |= Rocket::Core::Input::KM_SHIFT;

  if (sdlMods & KMOD_ALT)
    retval |= Rocket::Core::Input::KM_ALT;

  return retval;
}

