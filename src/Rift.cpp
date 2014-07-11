#include "Common.h"

RiftApp::RiftApp(bool fullscreen) :  RiftGlfwApp(fullscreen) {
  Platform::sleepMillis(200);
  if (!ovrHmd_StartSensor(hmd, 0, 0)) {
    SAY_ERR("Could not attach to sensor device");
  }
}

RiftApp::~RiftApp() {
  delete[] eyesArgs;
  ovrHmd_StopSensor(hmd);
}

void RiftApp::finishFrame() {
}

void RiftApp::initGl() {
  RiftGlfwApp::initGl();
  GL_CHECK_ERROR;
  eyesArgs = new PerEyeArgs[2];

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

  ovrFovPort * fovs = hmdDesc.DefaultEyeFov;
  ovrEyeRenderDesc eyeRenderDescs[2];
  int configResult = ovrHmd_ConfigureRendering(hmd, &cfg.Config,
    distortionCaps, fovs, eyeRenderDescs);

  float    orthoDistance = 0.8f; // 2D is 0.8 meter from camera

  using namespace oglplus;
  for_each_eye([&](ovrEyeType eye){
    PerEyeArgs & eyeArgs = eyesArgs[eye];
    const ovrFovPort & fov = fovs[eye];

    ovrGLTexture & ovrTexture = eyeArgs.ovrTexture;
    memset(&ovrTexture, 0, sizeof(ovrGLTexture));

    ovrTextureHeader & eyeTextureHeader = ovrTexture.Texture.Header;
    eyeTextureHeader.TextureSize = ovrHmd_GetFovTextureSize(hmd, eye, fov, 1.0f);
    eyeTextureHeader.RenderViewport.Size = eyeTextureHeader.TextureSize;
    eyeTextureHeader.API = ovrRenderAPI_OpenGL;

    ovrMatrix4f ovrPerspectiveProjection = ovrMatrix4f_Projection(fov, 0.01f, 100.0f, true);
    eyeArgs.projection = Rift::fromOvr(ovrPerspectiveProjection);

    // Allocate the frameBuffer that will hold the scene, and then be
    // re-rendered to the screen with distortion
    uvec2 frameBufferSize = Rift::fromOvr(eyeTextureHeader.TextureSize);

    gl.Bound(Texture::Target::_2D, eyeArgs.color_tex)
      .MinFilter(TextureMinFilter::Linear)
      .MagFilter(TextureMagFilter::Linear)
      .WrapS(TextureWrap::ClampToEdge)
      .WrapT(TextureWrap::ClampToEdge)
      .Image2D(0, PixelDataInternalFormat::RGBA8,
        frameBufferSize.x, frameBufferSize.y,
        0, PixelDataFormat::RGB, PixelDataType::UnsignedByte, nullptr
      );

    gl.Bound(Renderbuffer::Target::Renderbuffer, eyeArgs.depth_rbo)
      .Storage(
        PixelDataInternalFormat::DepthComponent,
        frameBufferSize.x,
        frameBufferSize.y
      );

    gl.Bound(Framebuffer::Target::Draw, eyeArgs.fbo)
      .AttachTexture(FramebufferAttachment::Color, eyeArgs.color_tex, 0)
      .AttachRenderbuffer(FramebufferAttachment::Depth, eyeArgs.depth_rbo)
      .Complete();
    eyeArgs.ovrTexture.OGL.TexId = GetName(eyeArgs.color_tex);
  });

  ///////////////////////////////////////////////////////////////////////////
  // Initialize OpenGL settings and variables
  // Anti-alias lines (hopefully)
  glEnable(GL_BLEND);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
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
}

void RiftApp::draw() {
  static int frameIndex = 0;
  ovrHmd_BeginFrame(hmd, frameIndex++);
  gl::MatrixStack & mv = gl::Stacks::modelview();
  gl::MatrixStack & pr = gl::Stacks::projection();
  for (int i = 0; i < 2; ++i) {
    ovrEyeType eye = currentEye = hmdDesc.EyeRenderOrder[i];
    PerEyeArgs & eyeArgs = eyesArgs[eye];

    pr.top() = eyeArgs.projection;
    gl::Stacks::with_push(pr, mv, [&]{
      // Set up the per-eye projection matrix
      ovrPosef renderPose = ovrHmd_BeginEyeRender(hmd, eye);

      // Apply the head pose
      mv.preMultiply(glm::inverse(Rift::fromOvr(renderPose)));

      // Apply the per-eye offset
      mv.preMultiply(eyeArgs.viewAdjust);

      //frameBuffers[eye].activate();
      eyeArgs.fbo.Bind(oglplus::Framebuffer::Target::Draw);
      gl.Viewport(eyeArgs.ovrTexture.Texture.Header.TextureSize.w,
        eyeArgs.ovrTexture.Texture.Header.TextureSize.h);
      // Render the scene to an offscreen buffer
      renderScene();
      //frameBuffers[eye].deactivate();
      oglplus::DefaultFramebuffer().Bind(oglplus::Framebuffer::Target::Draw);


      ovrHmd_EndEyeRender(hmd, eye, renderPose, &(eyeArgs.ovrTexture.Texture));
    });
    GL_CHECK_ERROR;
  }
  postDraw();
  ovrHmd_EndFrame(hmd);
  GL_CHECK_ERROR;
}

