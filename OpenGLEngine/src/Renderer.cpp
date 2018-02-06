#include "Renderer.h"
#include "ErrorHandling.h"

namespace render {
  void Renderer::Draw(scene::Object object, Shader* shader, glm::mat4 view)
  {
    shader->Bind();
    object.GetMesh().va.Bind();
    object.GetMesh().ib.Bind();
    //TODO figure out how to update uniforms
    glm::mat4 model_matrix = object.GetGlobalModelMatrix();
    if (object.GetDiffuseTexture() != NULL) {
        object.GetDiffuseTexture()->Bind();
    }
    shader->SetUniform4fv("model", model_matrix);
    shader->SetUniform3f("colour", object.GetColour());
    shader->SetUniform4fv("mv_it", glm::transpose(glm::inverse(view * model_matrix)));
    GLCall(glDrawElements(GL_TRIANGLES, object.GetMesh().ib.GetCount(), GL_UNSIGNED_INT, (void*)0));
  }
  void Renderer::Draw(scene::Object object)
  {
    object.GetMesh().va.Bind();
    object.GetMesh().ib.Bind();
    if (object.GetDiffuseTexture() != NULL) {
      object.GetDiffuseTexture()->Bind();
    }
    GLCall(glDrawElements(GL_TRIANGLES, object.GetMesh().ib.GetCount(), GL_UNSIGNED_INT, (void*)0));
  }
  void Renderer::Render(scene::Object object)
  {
    shader_->Bind();
    object.GetMesh().va.Bind();
    object.GetMesh().ib.Bind();
    //TODO figure out how to update uniforms
    glm::mat4 model_matrix = object.GetGlobalModelMatrix();
    if (object.GetDiffuseTexture() != NULL) {
      object.GetDiffuseTexture()->Bind();
    }
    shader_->SetUniform4fv("model", model_matrix);
    shader_->SetUniform3f("colour", object.GetColour());
    shader_->SetUniform4fv("mv_it", glm::transpose(glm::inverse(view_ * model_matrix)));
    GLCall(glDrawElements(GL_TRIANGLES, object.GetMesh().ib.GetCount(), GL_UNSIGNED_INT, (void*)0));
  }
  void Renderer::Clear()
  {
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  }
} //namespace render