#pragma once
//REFERENCE inspired by https://www.youtube.com/watch?v=jjaTTRFXRAk
//The Cherno Project
namespace core {
	class Renderer {
		void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
	};
} //namespace core