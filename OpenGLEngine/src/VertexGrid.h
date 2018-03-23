#pragma once
#include <memory>

namespace render {
  class VertexArray;
  class VertexGrid {
  private:
    int width_;
    int height_;
    std::unique_ptr<VertexArray> vao_;
  public:
    inline VertexGrid() {}
    VertexGrid(const int const width, const int const height);
    inline const int GetNumVertices() {
      return width_ * height_;
    }
    inline const VertexArray* GetVAO() {
      return vao_.get();
    }
  };
}