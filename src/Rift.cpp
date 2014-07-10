#include "Common.h"
#include <OVR_CAPI_GL.h>

RiftApp::RiftApp(bool fullscreen) :  RiftGlfwApp(fullscreen) {
  Platform::sleepMillis(200);
  if (!ovrHmd_StartSensor(hmd, 0, 0)) {
    SAY_ERR("Could not attach to sensor device");
  }

  memset(eyeTextures, 0, 2 * sizeof(ovrGLTexture));
  float eyeHeight = 1.5f;
  player = glm::inverse(glm::lookAt(
    vec3(0, eyeHeight, 4),
    vec3(0, eyeHeight, 0),
    vec3(0, 1, 0)));

  for_each_eye([&](ovrEyeType eye){
    ovrSizei eyeTextureSize = ovrHmd_GetFovTextureSize(hmd, eye, hmdDesc.MaxEyeFov[eye], 1.0f);

    ovrTextureHeader & eyeTextureHeader = eyeTextures[eye].OGL.Header;
    eyeTextureHeader.TextureSize = eyeTextureSize;
    eyeTextureHeader.RenderViewport.Size = eyeTextureSize;
    eyeTextureHeader.API = ovrRenderAPI_OpenGL;
  });
}

RiftApp::~RiftApp() {
  ovrHmd_StopSensor(hmd);
}

void RiftApp::finishFrame() {

}

void RiftApp::initGl() {
  RiftGlfwApp::initGl();
  query = gl::TimeQueryPtr(new gl::TimeQuery());
  GL_CHECK_ERROR;

  int samples;
  glGetIntegerv(GL_SAMPLES, &samples);

  ovrGLConfig cfg;
  memset(&cfg, 0, sizeof(cfg));
  cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
  cfg.OGL.Header.RTSize = Rift::toOvr(windowSize);
  cfg.OGL.Header.Multisample = 1;

  int distortionCaps = 0 
    | ovrDistortionCap_Vignette
    | ovrDistortionCap_Chromatic
    | ovrDistortionCap_TimeWarp
    ;

  int configResult = ovrHmd_ConfigureRendering(hmd, &cfg.Config,
    distortionCaps, hmdDesc.MaxEyeFov, eyeRenderDescs);

  float    orthoDistance = 0.8f; // 2D is 0.8 meter from camera
  for_each_eye([&](ovrEyeType eye){
    const ovrEyeRenderDesc & erd = eyeRenderDescs[eye];
    ovrMatrix4f ovrPerspectiveProjection = ovrMatrix4f_Projection(erd.Fov, 0.01f, 100000.0f, true);
    projections[eye] = Rift::fromOvr(ovrPerspectiveProjection);
    vec2 orthoScale = vec2(1.0f) / Rift::fromOvr(erd.PixelsPerTanAngleAtCenter);
    orthoProjections[eye] = Rift::fromOvr(
        ovrMatrix4f_OrthoSubProjection(
            ovrPerspectiveProjection, Rift::toOvr(orthoScale), orthoDistance, erd.ViewAdjust.x));
  });

  ///////////////////////////////////////////////////////////////////////////
  // Initialize OpenGL settings and variables
  // Anti-alias lines (hopefully)
  glEnable(GL_BLEND);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  GL_CHECK_ERROR;

  // Allocate the frameBuffer that will hold the scene, and then be
  // re-rendered to the screen with distortion
  uvec2 frameBufferSize = Rift::fromOvr(eyeTextures[0].OGL.Header.TextureSize);
  for_each_eye([&](ovrEyeType eye) {
    frameBuffers[eye].init(frameBufferSize);
    eyeTextures[eye].OGL.TexId = frameBuffers[eye].color->texture;
  });
  GL_CHECK_ERROR;
}

void RiftApp::onKey(int key, int scancode, int action, int mods) {
  if (GLFW_PRESS == action) switch (key) {
  case GLFW_KEY_R:
    ovrHmd_ResetSensor(hmd);
    return;
  }

  // Allow the camera controller to intercept the input
  if (CameraControl::instance().onKey(key, scancode, action, mods)) {
    return;
  }
  RiftGlfwApp::onKey(key, scancode, action, mods);
}


void RiftApp::update() {
  RiftGlfwApp::update();
  CameraControl::instance().applyInteraction(player);
//  gl::Stacks::modelview().top() = glm::lookAt(vec3(0, 0, 0.4f), vec3(0), vec3(0, 1, 0));
}

void RiftApp::draw() {
  static int frameIndex = 0;
  ovrHmd_BeginFrame(hmd, frameIndex++);
  gl::MatrixStack & mv = gl::Stacks::modelview();
  gl::MatrixStack & pr = gl::Stacks::projection();
  for (int i = 0; i < 2; ++i) {
    ovrEyeType eye = currentEye = hmdDesc.EyeRenderOrder[i];
    gl::Stacks::with_push(pr, mv, [&]{
      const ovrEyeRenderDesc & erd = eyeRenderDescs[eye];
      // Set up the per-eye projection matrix
      {
        ovrMatrix4f eyeProjection = ovrMatrix4f_Projection(erd.Fov, 0.01f, 100000.0f, true);
        mat4 ovrProj = Rift::fromOvr(eyeProjection);
        pr.top() = ovrProj;
      }

      ovrPosef renderPose = ovrHmd_BeginEyeRender(hmd, eye);
      // Set up the per-eye modelview matrix
      {
        // Apply the head pose
        mat4 m = Rift::fromOvr(renderPose);
        mv.preMultiply(glm::inverse(m));
        // Apply the per-eye offset
        vec3 eyeOffset = Rift::fromOvr(erd.ViewAdjust);
        mv.preMultiply(glm::translate(mat4(), eyeOffset));
      }

      // Render the scene to an offscreen buffer
      frameBuffers[eye].activate();
      //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      //glEnable(GL_DEPTH_TEST);
      renderScene();
      frameBuffers[eye].deactivate();

      ovrHmd_EndEyeRender(hmd, eye, renderPose, &(eyeTextures[eye].Texture));
    });
    GL_CHECK_ERROR;
  }
  query->begin();
  postDraw();
#if 1
  ovrHmd_EndFrame(hmd);
#else
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  static gl::GeometryPtr geometry = GlUtils::getQuadGeometry(1.0, 1.5f);
  static gl::ProgramPtr program = GlUtils::getProgram(Resource::SHADERS_TEXTURED_VS, Resource::SHADERS_TEXTURED_FS);
  program->use();
  geometry->bindVertexArray();
  gl::Stacks::with_push(pr, mv, [&]{
    pr.identity(); mv.identity();
    frameBuffers[0].color->bind();
    glViewport(0, 0, 640, 800);
    geometry->draw();
    frameBuffers[1].color->bind();
    glViewport(640, 0, 640, 800);
    geometry->draw();
  });
  gl::Program::clear();
  gl::VertexArray::unbind();
  glfwSwapBuffers(window);
#endif
  query->end();
  int result = query->getResult();
  GL_CHECK_ERROR;
}

void RiftApp::renderStringAt(const std::string & str, float x, float y) {
  gl::MatrixStack & mv = gl::Stacks::modelview();
  gl::MatrixStack & pr = gl::Stacks::projection();
  gl::Stacks::with_push(mv, pr, [&]{
    mv.identity();
    pr.top() = 1.0f * glm::ortho(
      -1.0f, 1.0f,
      -windowAspectInverse * 2.0f, windowAspectInverse * 2.0f,
      -100.0f, 100.0f);
    vec2 cursor(x, windowAspectInverse * y);
    GlUtils::renderString(str, cursor, 18.0f);
  });
}
