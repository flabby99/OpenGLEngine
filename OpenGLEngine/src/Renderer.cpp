#include "Renderer.h"
#include "ErrorHandling.h"

namespace render {
	void Renderer::Draw(scene::Object object, const Shader* shader)
	{
		shader->Bind();
		//TODO figure out how to update uniforms
		object.GetMesh().va.Bind();
		object.GetMesh().ib.Bind();
		GLCall(glDrawElements(GL_TRIANGLES, object.GetMesh().ib.GetCount(), GL_UNSIGNED_INT, (void*)0);)
	}
	void Renderer::Clear()
	{
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}
} //namespace render