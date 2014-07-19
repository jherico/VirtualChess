#include "Common.h"
#include "Rocket.h"

#include <Rocket/Core/FontDatabase.h>

typedef std::shared_ptr<Rocket::Core::Context> RocketContextPtr;
using namespace oglplus;

struct RocketSurface {
  RocketContextPtr ctx;
  uvec2 size;
  oglplus::Texture      tex;
  oglplus::Framebuffer  fbo;
private:
  oglplus::Context gl;
public:

  RocketSurface(const uvec2 & size) : size(size) {
    ctx.reset(Rocket::Core::CreateContext("main",
      Rocket::Core::Vector2i(size.x, size.y)));

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


  void render() {
    fbo.Bind(Framebuffer::Target::Draw);
    gl.Viewport(size.x, size.y);
    gl.Clear().ColorBuffer();
    ctx->Update();
    ctx->Render();
    DefaultFramebuffer().Bind(Framebuffer::Target::Draw);
  }

//  // Load and show the tutorial document.
//  Rocket::Core::ElementDocument* document = ctx->LoadDocument("data/tutorial.rml");
//  if (document != NULL) {
//    document->Show();
//    document->RemoveReference();
//  }

};


using namespace oglplus;
struct CompiledGeometry {
  oglplus::VertexArray vao;
  oglplus::Buffer buffer;
  int count;
  int vertexCount;
  int indexOffset;
  Rocket::Core::TextureHandle texture;
};

Resource FONT_NAMES[] = {
  Resource::FONTS_DELICIOUS_ROMAN_OTF,
  Resource::FONTS_DELICIOUS_ITALIC_OTF,
  Resource::FONTS_DELICIOUS_BOLD_OTF,
  Resource::FONTS_DELICIOUS_BOLDITALIC_OTF
};

static std::shared_ptr<RocketBridge> INSTANCE;

void RocketBridge::init() {
  INSTANCE.reset(new RocketBridge());
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

  Rocket::Core::String font_names[4];
  for (int i = 0; i < sizeof(font_names) / sizeof(Rocket::Core::String); i++) {
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
  cg.vertexCount = num_vertices;
  cg.indexOffset = sizeof(Rocket::Core::Vertex) * num_vertices;
  size_t bufferSize = cg.indexOffset + sizeof(int) * num_indices;

  cg.buffer.Bind(Buffer::Target::Array);
  Buffer::Data<void*>(Buffer::Target::Array, bufferSize, nullptr, usage);
  Buffer::SubData(BufferTarget::Array, 0, num_vertices, vertices);
  Buffer::SubData(BufferTarget::Array, cg.indexOffset, num_indices, indices);
  glBufferSubData(GL_ARRAY_BUFFER, cg.indexOffset, sizeof(int) * num_indices, indices);
  NoBuffer().Bind(Buffer::Target::Array);

//  cg.vao.Bind();
//  /// Two-dimensional position of the vertex (usually in pixels).
//  Vector2f position;
//  /// RGBA-ordered 8-bit / channel colour.
//  Colourb colour;
//  /// Texture coordinate for any associated texture.
//  Vector2f tex_coord;
//  VertexArrayAttrib(Layout::Attribute::POSITION).
//    Pointer(2, DataType::Float, false, VERTEX_STRIDE, 0).
//    Enable();
//  VertexArrayAttrib(Layout::Attribute::COLOR).
//    Pointer(4, DataType::Byte, false, VERTEX_STRIDE, VERTEX_COLOR_OFFSET).
//    Enable();
//  VertexArrayAttrib(Layout::Attribute::TEX_COORD).
//    Pointer(2, DataType::Float, false, VERTEX_STRIDE, VERTEX_TEX_OFFSET).
//    Enable();
//  NoVertexArray().Bind();
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
  Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "Compiled Geometry.");
  CompiledGeometry * cg = new CompiledGeometry();
  compileGeometry(*cg, BufferUsage::StaticDraw, vertices, num_vertices, indices, num_indices, texture);
  return (Rocket::Core::CompiledGeometryHandle)cg;
}


// Called by Rocket when it wants to render application-compiled geometry.
void RocketBridge::RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation)
{
  CompiledGeometry & cg = *(CompiledGeometry*)geometry;
  glPushMatrix();
  glTranslatef(translation.x, translation.y, 0);
//  cg.vao.Bind();
  cg.buffer.Bind(Buffer::Target::Array);
  cg.buffer.Bind(Buffer::Target::ElementArray);
  glVertexPointer(2, GL_FLOAT, VERTEX_STRIDE, 0);
  glEnableClientState(GL_COLOR_ARRAY);
  glColorPointer(4, GL_UNSIGNED_BYTE, VERTEX_STRIDE, VERTEX_COLOR_OFFSET);
  if (!cg.texture) {
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  } else {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, (GLuint) cg.texture);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, VERTEX_STRIDE, VERTEX_TEX_OFFSET);
  }
  glDrawElements(GL_TRIANGLES, cg.count, GL_UNSIGNED_INT, (void*)cg.indexOffset);
  glPopMatrix();
  NoBuffer().Bind(Buffer::Target::ElementArray);
  NoBuffer().Bind(Buffer::Target::Array);
//  NoVertexArray().Bind();
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
            image_dest[write_index] = image_src[read_index+2];
            image_dest[write_index+1] = image_src[read_index+1];
            image_dest[write_index+2] = image_src[read_index];
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

    return true;
}

// Called by Rocket when a loaded texture is no longer required.
void RocketBridge::ReleaseTexture(Rocket::Core::TextureHandle texture_handle)
{
    glDeleteTextures(1, (GLuint*) &texture_handle);
}


