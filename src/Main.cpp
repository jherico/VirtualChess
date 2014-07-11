#include "Common.h"
#include <oglplus/shapes/cube.hpp>

template <class T>
class RiftWrapperApp : public RiftApp {
public:
  T * deferredApp{ nullptr };

  virtual ~RiftWrapperApp() {
    delete deferredApp;
  }

  virtual void initGl() {
    RiftApp::initGl();
    deferredApp = new T();
    deferredApp->initGl();
  }

  virtual void onKey(int key, int scancode, int action, int mods) {
    deferredApp->onKey(key, scancode, action, mods);
  }

  virtual void update() {
    deferredApp->update();
  }

  virtual void renderScene() {
    deferredApp->renderScene();
  }
};


using namespace oglplus;
class BaseApp {
public:
  virtual void onKey(int key, int scancode, int action, int mods) {
  }

  virtual void update() {
  }

  virtual void initGl() {
  }

  virtual void renderScene() = 0;
};

class VirtualChess : public BaseApp{
  Context gl;
  Program prog;

  // helper object building cube vertex attributes
  shapes::Cube make_cube;
  // helper object encapsulating cube drawing instructions
  shapes::DrawingInstructions cube_instr;
  // indices pointing to cube primitive elements
  shapes::Cube::IndexArray cube_indices;

  // Uniforms
  Uniform<mat4> uProjection;
  Uniform<mat4> uModelView;

  // A vertex array object for the rendered cube
  VertexArray cube;
  // VBOs for the cube's vertices
  Buffer verts;
  
public:
  VirtualChess() : 
    uProjection(prog, Layout::Uniform::Projection), 
    uModelView(prog, Layout::Uniform::ModelView),
    cube_instr(make_cube.Instructions()), 
    cube_indices(make_cube.Indices()) {
    gl::Stacks::modelview().top() = glm::lookAt(vec3(0, 0, 0.5), vec3(0, 0, 0), vec3(0, 1, 0));
  }

  virtual void initGl() {
    prog = GlUtils::getProgram(Resource::SHADERS_SIMPLE_VS, Resource::SHADERS_COLORED_FS);
    cube.Bind();
    // bind the VBO for the cube vertices
    verts.Bind(Buffer::Target::Array);
    {
      std::vector<GLfloat> data;
      GLuint n_per_vertex = make_cube.Positions(data);
      // upload the data
      Buffer::Data(Buffer::Target::Array, data);
      // setup the vertex attribs array for the vertices
      VertexArrayAttrib attr(prog, "Position");
      attr.Setup<GLfloat>(n_per_vertex);
      attr.Enable();
      NoVertexArray().Bind();
      attr.Disable();
    }
  }

  void renderScene() {
    glClearColor(0.4, 0.4, 0.4, 1);
    gl.Clear().ColorBuffer().DepthBuffer();
    GlUtils::renderSkybox(Resource::IMAGES_SKY_CITY_XNEG_PNG);
    prog.Use();
    uProjection.Set(gl::Stacks::projection().top());
    gl::MatrixStack & mv = gl::Stacks::modelview();
    mv.withPush([&]{
      mv.scale(0.04f);
      uModelView.Set(mv.top());
      cube.Bind();
      cube_instr.Draw(cube_indices);
      NoVertexArray().Bind();
    });
  }
};

RUN_OVR_APP(RiftWrapperApp<VirtualChess>);
