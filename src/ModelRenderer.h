#pragma once
#include "Renderer.h"
#include <memory>

#include <glm/glm.hpp>
#include <glbinding/gl/gl.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Buffer.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/Framebuffer.h>
#include <globjects/Renderbuffer.h>
#include <globjects/Texture.h>
#include <globjects/base/File.h>
#include <globjects/TextureHandle.h>
#include <globjects/NamedString.h>
#include <globjects/base/StaticStringSource.h>

namespace minity
{
	class Viewer;

	class ModelRenderer : public Renderer
	{
	public:
		ModelRenderer(Viewer *viewer);
		virtual void display();

	private:

		std::unique_ptr<globjects::VertexArray> m_lightArray = std::make_unique<globjects::VertexArray>();
		std::unique_ptr<globjects::Buffer> m_lightVertices = std::make_unique<globjects::Buffer>();

		glm::vec3 light_a = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 light_d = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 light_s = glm::vec3(0.0f, 0.0f, 0.0f);

		float m_shininess = 0.0f;

		glm::vec3 m_ambient = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 m_diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 m_specular = glm::vec3(0.0f, 0.0f, 0.0f);

		bool reset_prop = true;
	};

}