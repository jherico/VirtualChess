#pragma once


struct Mesh {
  static vec3 transform(const mat4 & mat, const vec3 & vec) {
    vec4 result = mat * vec4(vec, 1);
    return vec3(result.x, result.y, result.z);
  }

  typedef std::vector<vec4> VVec4;
  typedef std::vector<vec3> VVec3;
  typedef std::vector<vec2> VVec2;
  typedef std::vector<GLuint> VS;

  gl::MatrixStack model;
  glm::vec3 color{ 1, 1, 1 };

  VVec4 positions;
  VVec4 normals;
  VVec3 colors;
  VVec2 texCoords;
  VS indices;

public:
  Mesh() {
  }

  Mesh(const Mesh & other)
    : model(other.model), color(other.color),
    positions(other.positions), normals(other.normals), colors(other.colors), texCoords(other.texCoords), indices(other.indices)
  {
  }

  gl::MatrixStack & getModel() {
    return model;
  }

  vec3 & getColor() {
    return color;
  }


  void clear() {
    positions.clear();
    normals.clear();
    colors.clear();
    texCoords.clear();
    indices.clear();
  }


  template<typename T>
  void add_all(T & dest, const T & src) {
    dest.reserve(dest.size() + src.size());
    dest.insert(dest.end(), src.begin(), src.end());
  }

  template<typename T>
  void add_all_transformed(const mat4 & xfm, T & dest, const T & src) {
    int destSize = dest.size();
    dest.reserve(dest.size() + src.size());
    for (size_t i = 0; i < src.size(); ++i) {
      dest.push_back(xfm * src[i]);
    }
  }

  template<typename T>
  void add_all_incremented(const size_t increment, T & dest, const T & src) {
    dest.reserve(dest.size() + src.size());
    for (size_t i = 0; i < src.size(); ++i) {
      dest.push_back(src[i] + increment);
    }
  }

  void fillColors(bool force = false) {
    if (force || colors.size()) {
      if (colors.size() != positions.size()) {
        add_all(colors, VVec3(positions.size() - colors.size(), color));
      }
    }
  }

  void fillNormals(bool force = false) {
    if (force || normals.size()) {
      while (normals.size() != positions.size()) {
        normals.push_back(positions[normals.size()]);
      }
    }
  }

  void addVertex(const glm::vec3 & vertex) {
    positions.push_back(model.top() * vec4(vertex, 1));
    indices.push_back((GLuint)indices.size());
  }

  void addMesh(const Mesh & mesh, bool forceColor) {
    int indexOffset = positions.size();

    // Positions are transformed
    add_all_transformed(model.top(), positions, mesh.positions);

    // normals are transformed with only the rotation, not the translation
    model.push().untranslate();
    add_all_transformed(model.top(), normals, mesh.normals);
    model.pop();

    // colors and tex coords are simply copied
    add_all(colors, mesh.colors);
    fillColors(forceColor);

    add_all(texCoords, mesh.texCoords);
    if (texCoords.size() && texCoords.size() != positions.size()) {
      add_all(texCoords, VVec2(positions.size() - texCoords.size(), glm::vec2()));
    }

    // indices are copied and incremented
    add_all_incremented(indexOffset, indices, mesh.indices);
  }

  void addQuad(float width, float height) {
    int indexOffset = positions.size();
    float x = width / 2.0f;
    float y = height / 2.0f;

    // Positions are transformed
    VVec4 quad;
    quad.push_back(vec4(-x, -y, 0, 1));
    quad.push_back(vec4(x, -y, 0, 1));
    quad.push_back(vec4(x, y, 0, 1));
    quad.push_back(vec4(-x, y, 0, 1));
    add_all_transformed(model.top(), positions, quad);

    // normals are transformed only by rotation, not translation
    VVec4 norm;
    for (int i = 0; i < 4; i++) {
      norm.push_back(vec4(0, 0, 1, 1));
    }
    model.push().untranslate();
    add_all_transformed(model.top(), normals, norm);
    model.pop();

    // indices are copied and incremented
    VS quadIndices;
    quadIndices.push_back(0);
    quadIndices.push_back(1);
    quadIndices.push_back(2);
    quadIndices.push_back(0);
    quadIndices.push_back(2);
    quadIndices.push_back(3);
    add_all_incremented(indexOffset, indices, quadIndices);
    fillColors();
  }

  void addQuad(const vec2 & size) {
    addQuad(size.x, size.y);
  }

  void validate() const {
    size_t vertexCount = positions.size();
    if (!normals.empty() && normals.size() != vertexCount) {
      throw new std::runtime_error("Incorrect number of normals");
    }
    if (!colors.empty() && colors.size() != vertexCount) {
      throw new std::runtime_error("Incorrect number of colors");
    }
    if (!texCoords.empty() && texCoords.size() != vertexCount) {
      throw new std::runtime_error("Incorrect number of texture coordinates");
    }
  }

  /*
    void clear();
    void validate() const;
    void fillColors(bool force = false);
    void fillNormals(bool force = false);
    void addVertex(const glm::vec4 & vertex);
    void addVertex(const glm::vec3 & vertex);
    void addMesh(const Mesh & mesh, bool populateColor = false);
    void addQuad(float width = 1.0f, float height = 1.0f);
  */
};
