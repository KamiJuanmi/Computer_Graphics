#include "ModelRenderer.h"
#include <globjects/base/File.h>
#include <globjects/State.h>
#include <iostream>
#include <filesystem>
#include <imgui.h>
#include "Viewer.h"
#include "Scene.h"
#include "Model.h"
#include <sstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace minity;
using namespace gl;
using namespace glm;
using namespace globjects;

ModelRenderer::ModelRenderer(Viewer* viewer) : Renderer(viewer)
{
	m_lightVertices->setStorage(std::array<vec3, 1>({ vec3(0.0f) }), GL_NONE_BIT);
	auto lightVertexBinding = m_lightArray->binding(0);
	lightVertexBinding->setBuffer(m_lightVertices.get(), 0, sizeof(vec3));
	lightVertexBinding->setFormat(3, GL_FLOAT);
	m_lightArray->enable(0);
	m_lightArray->unbind();

	createShaderProgram("model-base", {
		{ GL_VERTEX_SHADER,"./res/model/model-base-vs.glsl" },
		{ GL_GEOMETRY_SHADER,"./res/model/model-base-gs.glsl" },
		{ GL_FRAGMENT_SHADER,"./res/model/model-base-fs.glsl" },
		}, 
		{ "./res/model/model-globals.glsl" });

	createShaderProgram("model-light", {
		{ GL_VERTEX_SHADER,"./res/model/model-light-vs.glsl" },
		{ GL_FRAGMENT_SHADER,"./res/model/model-light-fs.glsl" },
		}, { "./res/model/model-globals.glsl" });
}

void ModelRenderer::display()
{
	// Save OpenGL state
	auto currentState = State::currentState();

	// retrieve/compute all necessary matrices and related properties
	const mat4 viewMatrix = viewer()->viewTransform();
	const mat4 inverseViewMatrix = inverse(viewMatrix);
	const mat4 modelViewMatrix = viewer()->modelViewTransform();
	const mat4 inverseModelViewMatrix = inverse(modelViewMatrix);
	const mat4 modelLightMatrix = viewer()->modelLightTransform();
	const mat4 inverseModelLightMatrix = inverse(modelLightMatrix);
	const mat4 modelViewProjectionMatrix = viewer()->modelViewProjectionTransform();
	const mat4 inverseModelViewProjectionMatrix = inverse(modelViewProjectionMatrix);
	const mat4 projectionMatrix = viewer()->projectionTransform();
	const mat4 inverseProjectionMatrix = inverse(projectionMatrix);
	const mat3 normalMatrix = mat3(transpose(inverseModelViewMatrix));
	const mat3 inverseNormalMatrix = inverse(normalMatrix);
	const vec2 viewportSize = viewer()->viewportSize();

	auto shaderProgramModelBase = shaderProgram("model-base");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	viewer()->scene()->model()->vertexArray().bind();

	const std::vector<Group> & groups = viewer()->scene()->model()->groups();
	const std::vector<Material> & materials = viewer()->scene()->model()->materials();

	static std::vector<bool> groupEnabled(groups.size(), true);
	static bool wireframeEnabled = true;
	static bool lightSourceEnabled = true;
	static vec4 wireframeLineColor = vec4(1.0f);

	if (ImGui::BeginMenu("Model"))
	{
		ImGui::Checkbox("Wireframe Enabled", &wireframeEnabled);
		ImGui::Checkbox("Light Source Enabled", &lightSourceEnabled);

		if (wireframeEnabled)
		{
			if (ImGui::CollapsingHeader("Wireframe"))
			{
				ImGui::ColorEdit4("Line Color", (float*)&wireframeLineColor, ImGuiColorEditFlags_AlphaBar);
			}
		}

		if (ImGui::CollapsingHeader("Groups"))
		{
			for (uint i = 0; i < groups.size(); i++)
			{
				bool checked = groupEnabled.at(i);
				ImGui::Checkbox(groups.at(i).name.c_str(), &checked);
				groupEnabled[i] = checked;
			}

		}

		ImGui::EndMenu();
	}

	vec4 worldCameraPosition = inverseModelViewMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 worldLightPosition = inverseModelLightMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f);

	shaderProgramModelBase->setUniform("modelViewProjectionMatrix", modelViewProjectionMatrix);
	shaderProgramModelBase->setUniform("viewportSize", viewportSize);
	shaderProgramModelBase->setUniform("worldCameraPosition", vec3(worldCameraPosition));
	shaderProgramModelBase->setUniform("worldLightPosition", vec3(worldLightPosition));
	shaderProgramModelBase->setUniform("wireframeEnabled", wireframeEnabled);
	shaderProgramModelBase->setUniform("wireframeLineColor", wireframeLineColor);

	shaderProgramModelBase->use();

	for (uint i = 0; i < groups.size(); i++)
	{
		if (groupEnabled.at(i))
		{
			const Material & material = materials.at(groups.at(i).materialIndex);

			if (reset_prop) {
				m_diffuse = material.diffuse;
				m_specular = material.specular;
				m_ambient = material.ambient;
				m_shininess = material.shininess;
				reset_prop = false;
			}

			if (material.diffuseTexture)
			{
				shaderProgramModelBase->setUniform("diffuseTexture", 0);
				material.diffuseTexture->bindActive(0);
			}

			if (material.ambientTexture)
			{
				shaderProgramModelBase->setUniform("ambientTexture", 1);
				material.ambientTexture->bindActive(1);
			}

			if (material.specularTexture)
			{
				shaderProgramModelBase->setUniform("specularTexture", 2);
				material.specularTexture->bindActive(2);
			}

			if (material.objectSpaceNormalTexture)
			{
				shaderProgramModelBase->setUniform("objectSpaceNormals", 3);
				material.objectSpaceNormalTexture->bindActive(3);
			}

			if (material.tangentSpaceNormalTexture)
			{
				shaderProgramModelBase->setUniform("tangentSpaceNormals", 4);
				material.tangentSpaceNormalTexture->bindActive(4);
			}

			viewer()->scene()->model()->vertexArray().drawElements(GL_TRIANGLES, groups.at(i).count(), GL_UNSIGNED_INT, (void*)(sizeof(GLuint)*groups.at(i).startIndex));

			if (material.diffuseTexture)
			{
				material.diffuseTexture->unbind();
			}
			if (material.ambientTexture)
			{
				material.ambientTexture->unbind();
			}
			if (material.specularTexture)
			{
				material.specularTexture->unbind();
			}
			if (material.objectSpaceNormalTexture)
			{
				material.objectSpaceNormalTexture->unbind();
			}
			if (material.tangentSpaceNormalTexture)
			{
				material.tangentSpaceNormalTexture->unbind();
			}
		}
	}

	if (ImGui::BeginMenu("Assignment1")) {
		if (ImGui::CollapsingHeader("Light Control"))
		{
			ImGui::ColorEdit3("Ambient Light", (float*)(&light_a));
			ImGui::ColorEdit3("Diffuse Light", (float*)(&light_d));
			ImGui::ColorEdit3("Specular Light", (float*)(&light_s));
		}
		if (ImGui::CollapsingHeader("Properties Control"))
		{
			ImGui::ColorEdit3("Ka", (float*)(&m_ambient));
			ImGui::ColorEdit3("Kd", (float*)(&m_diffuse));
			ImGui::ColorEdit3("Ks", (float*)(&m_specular));
			ImGui::SliderFloat("shininess", &m_shininess, 0.0f, 300.0f);
			ImGui::Checkbox("Reset Properties", &reset_prop);
		}

		ImGui::EndMenu();
	}

	shaderProgramModelBase->setUniform("light_A", light_a);
	shaderProgramModelBase->setUniform("light_S", light_s);
	shaderProgramModelBase->setUniform("light_D", light_d);

	shaderProgramModelBase->setUniform("shininess", m_shininess);
	shaderProgramModelBase->setUniform("diffuseColor", m_diffuse);
	shaderProgramModelBase->setUniform("specularColor", m_specular);
	shaderProgramModelBase->setUniform("ambientColor", m_ambient);

	if (ImGui::BeginMenu("Assignment2")) {

		ImGui::Checkbox("Ambient  Textures", &ambTxt);
		ImGui::Checkbox("Diffuse  Textures", &difTxt);
		ImGui::Checkbox("Specular Textures", &spcTxt);


		const char* items[] = { "None", "Object Space", "Tangent Space" };
		static const char* current_item = "None";
		if (ImGui::BeginCombo("Space Textures", current_item)) 
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == items[n]); 
				if (ImGui::Selectable(items[n], is_selected))
					current_item = items[n];
				if (is_selected)
					ImGui::SetItemDefaultFocus();  
			}
			switch (current_item[0])
			{
			case 'N':	
				if(objSpace == true || tangSpace == true)
					objSpace = tangSpace = false;
				break;
			case 'O':	
				if (tangSpace == true)
					tangSpace = false;
				if (objSpace !=true)
					objSpace = true;
				break;
			case 'T':	
				if (objSpace == true)
					objSpace = false;
				if (tangSpace != true)
					tangSpace = true;
				break;
			}
			ImGui::EndCombo();
		}
		if (tangSpace) {
			if (ImGui::CollapsingHeader("Bump Mapping"))
			{
				ImGui::SliderFloat("Amplitude", &amp, 1.0f, 300.0f);
				ImGui::SliderFloat("Frequency", &freq, 1.0f, 300.0f);
				ImGui::Checkbox("Enabel Bump Mapping", &bumpMapping);
			}
		}		

		ImGui::EndMenu();
	}

	shaderProgramModelBase->setUniform("diff_txt", difTxt);
	shaderProgramModelBase->setUniform("ambn_txt", ambTxt);
	shaderProgramModelBase->setUniform("spec_txt", spcTxt);
	shaderProgramModelBase->setUniform("objSpace", objSpace);
	shaderProgramModelBase->setUniform("tangSpace", tangSpace);
	shaderProgramModelBase->setUniform("bumpMapping", bumpMapping);
	shaderProgramModelBase->setUniform("amp", amp);
	shaderProgramModelBase->setUniform("freq", freq);

	shaderProgramModelBase->release();

	viewer()->scene()->model()->vertexArray().unbind();


	if (lightSourceEnabled)
	{
		auto shaderProgramModelLight = shaderProgram("model-light");
		shaderProgramModelLight->setUniform("modelViewProjectionMatrix", modelViewProjectionMatrix * inverseModelLightMatrix);
		shaderProgramModelLight->setUniform("viewportSize", viewportSize);

		glEnable(GL_PROGRAM_POINT_SIZE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);

		m_lightArray->bind();

		shaderProgramModelLight->use();
		m_lightArray->drawArrays(GL_POINTS, 0, 1);
		shaderProgramModelLight->release();

		m_lightArray->unbind();

		glDisable(GL_PROGRAM_POINT_SIZE);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}

	// Restore OpenGL state (disabled to to issues with some Intel drivers)
	// currentState->apply();
}