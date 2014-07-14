#pragma once

class Render {
private:
  Render() {}

public:

  static void renderGeometry(oglplus::Program & program, Geometry & geometry) {
    using namespace oglplus;
    program.Use();
    Uniform<mat4>(program, Layout::Uniform::Projection)
      .Set(Stacks::projection().top());
    Uniform<mat4>(program, Layout::Uniform::ModelView)
      .Set(Stacks::modelview().top());
    geometry.bind();
    geometry.draw();
    NoVertexArray().Bind();
    NoProgram().Use();
  }

  static void renderSkybox(Resource firstResource) {
    using namespace oglplus;
    static Program prog = GlUtils::getProgram(
        Resource::SHADERS_CUBEMAP_VS,
        Resource::SHADERS_CUBEMAP_FS);

    // Skybox texture
    static Geometry cube(
        [&](Buffer & vertexBuffer, Buffer & indexBuffer, VertexArray & vao){
      GlUtils::getCubeVertices(vertexBuffer);
      GlUtils::getCubeIndices(indexBuffer);
      vao.Bind();
      vertexBuffer.Bind(Buffer::Target::Array);
      indexBuffer.Bind(Buffer::Target::ElementArray);
      VertexArrayAttrib(Layout::Attribute::Position).
        Pointer(3, DataType::Float, false, sizeof(vec4), nullptr).
        Enable();
      NoVertexArray().Bind();
      NoBuffer().Bind(Buffer::Target::Array);
      NoBuffer().Bind(Buffer::Target::ElementArray);
    }, 36);

    Context & gl = GlUtils::context();
    gl.Disable(Capability::DepthTest);
    gl.CullFace(Face::Front);

    MatrixStack & mv = Stacks::modelview();
    mv.withPush([&]{
      mv.untranslate();
      auto boundTexture = gl.Bound(Texture::Target::CubeMap,
        GlUtils::getCubemapTexture(firstResource));
  //    Uniform<mat4>(prog, Layout::Uniform::Projection).Set(Stacks::projection().top());
  //    Uniform<mat4>(prog, Layout::Uniform::ModelView).Set(Stacks::modelview().top());
  //    prog.Bind();
  //    cube.draw();
      renderGeometry(prog, cube);
    });

    NoVertexArray().Bind();
    NoProgram().Bind();
    gl.CullFace(Face::Back);
    gl.Enable(Capability::DepthTest);
  }

};
