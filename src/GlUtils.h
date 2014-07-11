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

#include <vector>
#include <string>

namespace Layout {
  namespace Attribute {
    enum {
      Position = 0,
      TexCoord0 = 1,
      Normal = 2,
      Color = 3,
      TexCoord1 = 4,
      InstanceTransform = 5,
    };
  }

  namespace Uniform {
    enum {
      Projection = 0,
      ModelView = 1,
    };
  }
}

class GlUtils {
public:
  static oglplus::Context & context();

  static oglplus::Texture & getCubemapTexture(
    Resource resource);

  static oglplus::Program getProgram(
    Resource vertexResource,
    Resource fragmentResource);

  static void renderSkybox(
    Resource firstResource);


  /*
  static void drawColorCube(bool lit = false);
  static void drawQuad(
  const vec2 & min = vec2(-1),
  const vec2 & max = vec2(1));

  static void drawAngleTicks();
  static void draw3dGrid();
  static void draw3dVector(vec3 vec, const vec3 & col);

  static gl::GeometryPtr getColorCubeGeometry();
  static gl::GeometryPtr getCubeGeometry();
  static gl::GeometryPtr getWireCubeGeometry();

  static gl::GeometryPtr getQuadGeometry(
    float aspect, float size = 2.0f
  );

  static gl::GeometryPtr getQuadGeometry(
      const vec2 & min = vec2(-1),
      const vec2 & max = vec2(1),
      const vec2 & texMin = vec2(0),
      const vec2 & texMax = vec2(1)
  );
  */


  /*
  static void getImageData(
    const std::vector<unsigned char> & indata,
    uvec2 & outSize,
    std::vector<unsigned char> & outData,
    bool flip = true
  );

  static void getImageData(
    Resource resource,
    uvec2 & outSize,
    std::vector<unsigned char> & outData,
    bool flip = true
    );

  template<GLenum TextureType>
  static void getImageAsTexture(
    std::shared_ptr<gl::Texture<TextureType> > & texture,
    Resource resource,
    uvec2 & outSize,
    GLenum target = TextureType) {
    typedef gl::Texture<TextureType>
      Texture;
    typedef std::shared_ptr<gl::Texture<TextureType> >
      TexturePtr;
    typedef std::vector<unsigned char>
      Vector;
    Vector imageData;
    getImageData(resource, outSize, imageData);
    texture = TexturePtr(new Texture());
    texture->bind();
#ifdef HAVE_OPENCV
    glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
#endif
    texture->image2d(outSize, &imageData[0]);
    texture->parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture->parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    Texture::unbind();
  }
  */
  /**
  * A convenience method for loading images into textures
  * when you don't care about the dimensions of the image
  template<
    GLenum TextureType = GL_TEXTURE_2D, 
    GLenum TextureFomat = GL_RGBA8
  >
  static typename gl::Texture<TextureType, TextureFomat>::Ptr  
    getImageAsTexture(
      Resource resource,
      GLenum target = TextureType) {
    uvec2 imageSize;
    std::shared_ptr<gl::Texture<TextureType, TextureFomat> > texture;
    getImageAsTexture(texture, resource, target);
    return texture;
  }

  */

  /**
   * A convenience method for loading images into textures
   * when you don't care about the dimensions of the image
  template<
    GLenum TextureType = GL_TEXTURE_2D,
    GLenum TextureFomat = GL_RGBA8
  >
  static void getImageAsTexture(
    std::shared_ptr<gl::Texture<TextureType, TextureFomat> > & texture,
    Resource resource,
    GLenum target = TextureType) {
    uvec2 imageSize;
    getImageAsTexture(texture, resource, imageSize, target);
  }


  template<GLenum TextureType = GL_TEXTURE_2D>
  static void getImageAsGeometryAndTexture(
    Resource resource,
    gl::GeometryPtr & geometry,
    std::shared_ptr<gl::Texture<TextureType> > & texture) {

    uvec2 imageSize;
    GlUtils::getImageAsTexture(texture, resource, imageSize);
    float imageAspectRatio = aspect(imageSize);
    vec2 geometryMax(1.0f, 1.0f / imageAspectRatio);
    vec2 geometryMin = geometryMax * -1.0f;
    geometry = GlUtils::getQuadGeometry(geometryMin, geometryMax);
  }
  */

  //static const Mesh & getMesh(Resource resource);
  /*
  static void renderGeometry(
      const gl::GeometryPtr & geometry,
      gl::ProgramPtr program);

      */
  /*
  static void renderBunny();
  static void renderArtificialHorizon( float alpha = 1.0f );
  static void renderRift();

  static void renderParagraph(const std::string & str);

  static void renderString(const std::string & str, vec2 & cursor,
      float fontSize = 12.0f, Resource font =
          Resource::FONTS_INCONSOLATA_MEDIUM_SDFF);

  static void renderString(const std::string & str, vec3 & cursor,
      float fontSize = 12.0f, Resource font =
          Resource::FONTS_INCONSOLATA_MEDIUM_SDFF);

  static void tumble(const vec3 & camera = vec3(0, 0, 1));

  static vec2 quantize(const vec2 & t, float scale) {
    float width = scale * 2.0f;
    vec2 t2 = t + scale;
    vec2 t3 = floor(t2 / width) * width;
    return t3;
  }

  static void scaleRenderGrid(float scale, const vec2 & p) {
    vec2 p3 = quantize(p, scale);
    gl::MatrixStack & mv = gl::Stacks::modelview();
    mv.push().translate(vec3(p3.x - scale, 0, p3.y - scale)).scale(
      scale);
    GlUtils::draw3dGrid();
    mv.pop();

    mv.push().translate(vec3(p3.x - scale, 0, p3.y + scale)).scale(
      scale);
    GlUtils::draw3dGrid();
    mv.pop();

    mv.push().translate(vec3(p3.x + scale, 0, p3.y - scale)).scale(
      scale);
    GlUtils::draw3dGrid();
    mv.pop();

    mv.push().translate(vec3(p3.x + scale, 0, p3.y + scale)).scale(
      scale);
    GlUtils::draw3dGrid();
    mv.pop();
  }

  static void renderFloorGrid(const vec2 & position) {
    scaleRenderGrid(1.0, position);
    scaleRenderGrid(10.0, position);
    scaleRenderGrid(100.0, position);
  }

  static void renderFloorGrid(const vec3 & position) {
    renderFloorGrid(vec2(position.x, position.z));
  }

  static void renderFloorGrid(const mat4 & camera) {
    renderFloorGrid(vec2(camera[3].x, camera[3].z));
  }

  static void cubeRecurse(int depth = 6, float elapsed = Platform::elapsedSeconds());
  static void dancingCubes(int elements = 8, float elapsed = Platform::elapsedSeconds());
  */

  static const vec3 X_AXIS;
  static const vec3 Y_AXIS;
  static const vec3 Z_AXIS;
  static const vec3 ORIGIN;
  static const vec3 ONE;
  static const vec3 UP;
};

struct Colors {
  static const vec3 gray;
  static const vec3 white;
  static const vec3 red;
  static const vec3 green;
  static const vec3 blue;
  static const vec3 cyan;
  static const vec3 magenta;
  static const vec3 yellow;
  static const vec3 black;
  static const vec3 aliceBlue;
  static const vec3 antiqueWhite;
  static const vec3 aqua;
  static const vec3 aquamarine;
  static const vec3 azure;
  static const vec3 beige;
  static const vec3 bisque;
  static const vec3 blanchedAlmond;
  static const vec3 blueViolet;
  static const vec3 brown;
  static const vec3 burlyWood;
  static const vec3 cadetBlue;
  static const vec3 chartreuse;
  static const vec3 chocolate;
  static const vec3 coral;
  static const vec3 cornflowerBlue;
  static const vec3 cornsilk;
  static const vec3 crimson;
  static const vec3 darkBlue;
  static const vec3 darkCyan;
  static const vec3 darkGoldenRod;
  static const vec3 darkGray;
  static const vec3 darkGrey;
  static const vec3 darkGreen;
  static const vec3 darkKhaki;
  static const vec3 darkMagenta;
  static const vec3 darkOliveGreen;
  static const vec3 darkorange;
  static const vec3 darkOrchid;
  static const vec3 darkRed;
  static const vec3 darkSalmon;
  static const vec3 darkSeaGreen;
  static const vec3 darkSlateBlue;
  static const vec3 darkSlateGray;
  static const vec3 darkSlateGrey;
  static const vec3 darkTurquoise;
  static const vec3 darkViolet;
  static const vec3 deepPink;
  static const vec3 deepSkyBlue;
  static const vec3 dimGray;
  static const vec3 dimGrey;
  static const vec3 dodgerBlue;
  static const vec3 fireBrick;
  static const vec3 floralWhite;
  static const vec3 forestGreen;
  static const vec3 fuchsia;
  static const vec3 gainsboro;
  static const vec3 ghostWhite;
  static const vec3 gold;
  static const vec3 goldenRod;
  static const vec3 grey;
  static const vec3 greenYellow;
  static const vec3 honeyDew;
  static const vec3 hotPink;
  static const vec3 indianRed;
  static const vec3 indigo;
  static const vec3 ivory;
  static const vec3 khaki;
  static const vec3 lavender;
  static const vec3 lavenderBlush;
  static const vec3 lawnGreen;
  static const vec3 lemonChiffon;
  static const vec3 lightBlue;
  static const vec3 lightCoral;
  static const vec3 lightCyan;
  static const vec3 lightGoldenRodYellow;
  static const vec3 lightGray;
  static const vec3 lightGrey;
  static const vec3 lightGreen;
  static const vec3 lightPink;
  static const vec3 lightSalmon;
  static const vec3 lightSeaGreen;
  static const vec3 lightSkyBlue;
  static const vec3 lightSlateGray;
  static const vec3 lightSlateGrey;
  static const vec3 lightSteelBlue;
  static const vec3 lightYellow;
  static const vec3 lime;
  static const vec3 limeGreen;
  static const vec3 linen;
  static const vec3 maroon;
  static const vec3 mediumAquaMarine;
  static const vec3 mediumBlue;
  static const vec3 mediumOrchid;
  static const vec3 mediumPurple;
  static const vec3 mediumSeaGreen;
  static const vec3 mediumSlateBlue;
  static const vec3 mediumSpringGreen;
  static const vec3 mediumTurquoise;
  static const vec3 mediumVioletRed;
  static const vec3 midnightBlue;
  static const vec3 mintCream;
  static const vec3 mistyRose;
  static const vec3 moccasin;
  static const vec3 navajoWhite;
  static const vec3 navy;
  static const vec3 oldLace;
  static const vec3 olive;
  static const vec3 oliveDrab;
  static const vec3 orange;
  static const vec3 orangeRed;
  static const vec3 orchid;
  static const vec3 paleGoldenRod;
  static const vec3 paleGreen;
  static const vec3 paleTurquoise;
  static const vec3 paleVioletRed;
  static const vec3 papayaWhip;
  static const vec3 peachPuff;
  static const vec3 peru;
  static const vec3 pink;
  static const vec3 plum;
  static const vec3 powderBlue;
  static const vec3 purple;
  static const vec3 rosyBrown;
  static const vec3 royalBlue;
  static const vec3 saddleBrown;
  static const vec3 salmon;
  static const vec3 sandyBrown;
  static const vec3 seaGreen;
  static const vec3 seaShell;
  static const vec3 sienna;
  static const vec3 silver;
  static const vec3 skyBlue;
  static const vec3 slateBlue;
  static const vec3 slateGray;
  static const vec3 slateGrey;
  static const vec3 snow;
  static const vec3 springGreen;
  static const vec3 steelBlue;
  static const vec3 blueSteel;
  static const vec3 tan;
  static const vec3 teal;
  static const vec3 thistle;
  static const vec3 tomato;
  static const vec3 turquoise;
  static const vec3 violet;
  static const vec3 wheat;
  static const vec3 whiteSmoke;
  static const vec3 yellowGreen;
};
