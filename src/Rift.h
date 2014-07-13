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

class Rift {
public:
//  static void getDefaultDk1HmdValues(ovrHmd hmd, ovrHmdDesc & ovrHmdInfo);
  static void getRiftPositionAndSize(ovrHmd hmd,
      ivec2 & windowPosition, uvec2 & windowSize);
  static quat getStrabismusCorrection();
  static void setStrabismusCorrection(const quat & q);
  static void getHmdInfo(ovrHmd hmd, ovrHmdDesc & ovrHmdInfo);
  static mat4 getMat4(ovrHmd hmd);

  static inline mat4 fromOvr(const ovrFovPort & fovport, float nearPlane = 0.01f, float farPlane = 10000.0f) {
    return fromOvr(ovrMatrix4f_Projection(fovport, nearPlane, farPlane, true));
  }

  static inline mat4 fromOvr(const ovrMatrix4f & om) {
    return glm::transpose(glm::make_mat4(&om.M[0][0]));
  }

  static inline vec3 fromOvr(const ovrVector3f & ov) {
    return glm::make_vec3(&ov.x);
  }

  static inline vec2 fromOvr(const ovrVector2f & ov) {
    return glm::make_vec2(&ov.x);
  }

  static inline uvec2 fromOvr(const ovrSizei & ov) {
    return uvec2(ov.w, ov.h);
  }

  static inline quat fromOvr(const ovrQuatf & oq) {
    return glm::make_quat(&oq.x);
  }

  static inline mat4 fromOvr(const ovrPosef & op) {
    mat4 orientation = glm::mat4_cast(fromOvr(op.Orientation));
    mat4 translation = glm::translate(mat4(), Rift::fromOvr(op.Position));
    return translation * orientation;
    //  return mat4_cast(fromOvr(op.Orientation)) * glm::translate(mat4(), Rift::fromOvr(op.Position));
  }

  static inline ovrMatrix4f toOvr(const mat4 & m) {
    ovrMatrix4f result;
    mat4 transposed(glm::transpose(m));
    memcpy(result.M, &(transposed[0][0]), sizeof(float) * 16);
    return result;
  }

  static inline ovrVector3f toOvr(const vec3 & v) {
    ovrVector3f result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    return result;
  }

  static inline ovrVector2f toOvr(const vec2 & v) {
    ovrVector2f result;
    result.x = v.x;
    result.y = v.y;
    return result;
  }

  static inline ovrSizei toOvr(const uvec2 & v) {
    ovrSizei result;
    result.w = v.x;
    result.h = v.y;
    return result;
  }

  static inline ovrQuatf toOvr(const quat & q) {
    ovrQuatf result;
    result.x = q.x;
    result.y = q.y;
    result.z = q.z;
    result.w = q.w;
    return result;
  }

};

template <typename Function>
void for_each_eye(Function function) {
  for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
    eye < ovrEyeType::ovrEye_Count;
    eye = static_cast<ovrEyeType>(eye + 1)) {
    function(eye);
  }
}

class RiftManagerApp {
protected:
  ovrHmd hmd;
  ovrHmdDesc hmdDesc;

  uvec2 hmdNativeResolution;
  ivec2 hmdDesktopPosition;

public:
  RiftManagerApp(ovrHmdType defaultHmdType = ovrHmd_DK1) {
    hmd = ovrHmd_Create(0);
    if (NULL == hmd) {
      hmd = ovrHmd_CreateDebug(defaultHmdType);
    }
    ovrHmd_GetDesc(hmd, &hmdDesc);
    hmdNativeResolution = ivec2(hmdDesc.Resolution.w, hmdDesc.Resolution.h);
    hmdDesktopPosition = ivec2(hmdDesc.WindowsPos.x, hmdDesc.WindowsPos.y);
  }

  virtual ~RiftManagerApp() {
    ovrHmd_Destroy(hmd);
    hmd = nullptr;
  }
};


struct RiftWrapperArgs {
  uvec2       windowSize;
  ovrHmd      hmd;
  ovrHmdDesc  hmdDesc;
};
/**
A class that takes care of the basic duties of putting an OpenGL
window on the desktop in the correct position so that it's visible
through the Rift.
*/

void setupSdlGlAttributes();

template <class T>
class RiftWrapperApp : public SdlWrapperApp<T, RiftWrapperArgs>, public RiftManagerApp {
  int hmdDisplay;

public:
  RiftWrapperApp() {
    // Attempt to find the Rift monitor.
    windowPosition = hmdDesktopPosition;
    hmdDisplay = getSdlDisplayAtPosition(windowPosition, windowSize);
    if (-1 == hmdDisplay) {
      FAIL("No Rift display found.");
    }

    Platform::sleepMillis(200);
    if (!ovrHmd_StartSensor(hmd, 0, 0)) {
      SAY_ERR("Could not attach to sensor device");
    }
  }

  virtual ~RiftWrapperApp() {
    ovrHmd_StopSensor(hmd);
  }

  RiftWrapperArgs getArgs() {
    RiftWrapperArgs result;
    result.hmd = hmd;
    result.hmdDesc = hmdDesc;
    result.windowSize = windowSize;
    return result;
  }

  virtual SDL_Window * createWindow() {
    setupSdlGlAttributes();
    SDL_Window * result = SDL_CreateWindow("SDL", 
      windowPosition.x, windowPosition.y, 
      windowSize.x, windowSize.y,
      SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);
    return result;
  }
};


class RiftApp {
public:
  struct PerEyeArgs {
    ovrGLTexture ovrTexture;
    //ovrEyeRenderDesc ovrEyeDesc;
    mat4 projection;
    oglplus::Texture      color_tex;
    oglplus::Renderbuffer depth_rbo;
    oglplus::Framebuffer  fbo;
    mat4 viewAdjust;
  };

protected:
  oglplus::Context gl;
  uvec2       windowSize;
  ovrEyeType  currentEye{ ovrEye_Count };
  PerEyeArgs  eyesArgs[2];
  ovrHmdDesc  hmdDesc;
  ovrHmd      hmd;

public:
  RiftApp(const RiftWrapperArgs & args) : 
      hmd(args.hmd), hmdDesc(args.hmdDesc), windowSize(args.windowSize) 
  {
    initGl();
  }

  void initGl() {
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

    for_each_eye([&](ovrEyeType eye){
      PerEyeArgs & eyeArgs = eyesArgs[eye];
      const ovrFovPort & fov = fovs[eye];

      ovrGLTexture & ovrTexture = eyeArgs.ovrTexture;
      memset(&ovrTexture, 0, sizeof(ovrGLTexture));

      eyeArgs.viewAdjust = glm::translate(glm::mat4(), Rift::fromOvr(eyeRenderDescs[eye].ViewAdjust));

      ovrTextureHeader & eyeTextureHeader = ovrTexture.Texture.Header;
      eyeTextureHeader.TextureSize = ovrHmd_GetFovTextureSize(hmd, eye, fov, 1.0f);
      eyeTextureHeader.RenderViewport.Size = eyeTextureHeader.TextureSize;
      eyeTextureHeader.API = ovrRenderAPI_OpenGL;

      ovrMatrix4f ovrPerspectiveProjection = ovrMatrix4f_Projection(fov, 0.01f, 100.0f, true);
      eyeArgs.projection = Rift::fromOvr(ovrPerspectiveProjection);

      // Allocate the frameBuffer that will hold the scene, and then be
      // re-rendered to the screen with distortion
      uvec2 frameBufferSize = Rift::fromOvr(eyeTextureHeader.TextureSize);

      using namespace oglplus;
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

  virtual bool isDone() {
    return false;
  }
  
  void onTick() {
    updateState();
    drawFrame();
  }

  virtual void updateState() = 0;
  virtual void drawScene() = 0;

  void drawFrame() {
    static int frameIndex = 0;
    ovrHmd_BeginFrame(hmd, frameIndex++);
    MatrixStack & mv = Stacks::modelview();
    MatrixStack & pr = Stacks::projection();
    for (int i = 0; i < 2; ++i) {
      ovrEyeType eye = currentEye = hmdDesc.EyeRenderOrder[i];
      PerEyeArgs & eyeArgs = eyesArgs[eye];

      pr.top() = eyeArgs.projection;
      Stacks::with_push(pr, mv, [&]{
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
        drawScene();
        oglplus::DefaultFramebuffer().Bind(oglplus::Framebuffer::Target::Draw);
        ovrHmd_EndEyeRender(hmd, eye, renderPose, &(eyeArgs.ovrTexture.Texture));
      });
      GL_CHECK_ERROR;
    }

    ovrHmd_EndFrame(hmd);
    GL_CHECK_ERROR;
  }
};



// Combine some macros together to create a single macro
// to launch a class containing a run method
#define RUN_OVR_APP(AppClass) \
MAIN_DECL { \
  if (!ovr_Initialize()) { \
      SAY_ERR("Failed to initialize the Oculus SDK"); \
      return -1; \
  } \
  int result = -1; \
  try { \
    result = AppClass().run(); \
  } catch (std::exception & error) { \
    SAY_ERR(error.what()); \
  } catch (std::string & error) { \
    SAY_ERR(error.c_str()); \
  } \
  ovr_Shutdown(); \
  return result; \
}

