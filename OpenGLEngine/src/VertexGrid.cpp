#include "VertexGrid.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

namespace render {
 VertexGrid::VertexGrid(const int width, const int height) {
    width_ = width;
    height_ = height;
    float width_increment = 2.0f / width;
    float height_increment = 2.0f / height;
    float start_width = -1.0f + width_increment / 2.0f;
    float start_height = -1.0f + height_increment / 2.0f;
    float* vertices = new float[width * height * 2];
    for (int i = 0; i < width; ++i) {
      for (int j = 0; j < height; ++j) {
        vertices[2 * (i * height + j)] = (float)(start_width + width_increment * i);
        vertices[2 * (i * height + j) + 1] = (float)(start_height + height_increment * j);
      }
    }
    //vertices[0] = 0.5f;
    //vertices[1] = 0.5f;
    auto points_vb = std::make_shared<VertexBuffer>(
      vertices,
      (unsigned int)sizeof(float) * 2 * width * height
      );
    vao_ = std::make_unique<VertexArray>();
    vao_->Addbuffer_2f(points_vb, 0);
    delete[] vertices;
  }
}