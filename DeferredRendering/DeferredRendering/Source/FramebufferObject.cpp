#include "FramebufferObject.h"

#include "../EW/Shader.h"
#include "../EW/ShapeGen.h"

FramebufferObject::FramebufferObject()
{
	id = 0;

	dimensions = glm::vec2(0, 0);
}

FramebufferObject::~FramebufferObject()
{
	Destroy();
}

bool FramebufferObject::IsComplete()
{
	Bind();

	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void FramebufferObject::Create()
{
	glGenFramebuffers(1, &id);

	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

/// <summary>
/// Adds a color attachment to the Framebuffer object
/// </summary>
/// <param name="buffer">Color buffer to add</param>
/// <param name="attachmentNum">GL_COLOR_ATTACHMENT<0-8></param>
void FramebufferObject::AddColorAttachment(Texture& buffer, GLenum attachmentNum)
{
	Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentNum, GL_TEXTURE_2D, buffer.GetTexture(), 0);

	/*if (screenDimensions == glm::vec2(0))
	{

	}*/
	dimensions = buffer.GetDimensions();

	colorAttachments.push_back(buffer.GetTexture());
}

void FramebufferObject::AddColorAttachment(RenderBuffer& buffer, GLenum attachmentNum)
{
	Bind();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentNum, GL_RENDERBUFFER, buffer.GetRenderBuffer());

	colorAttachments.push_back(buffer.GetRenderBuffer());
}


void FramebufferObject::AddDepthAttachment(Texture& buffer)
{
	Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.GetTexture(), 0);

	depthAttachments.push_back(buffer.GetTexture());
}


void FramebufferObject::AddDepthAttachment(RenderBuffer& buffer)
{
	Bind();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer.GetRenderBuffer());

	depthAttachments.push_back(buffer.GetRenderBuffer());
}

void FramebufferObject::Bind()
{
	glViewport(0, 0, dimensions.x, dimensions.y);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glEnable(GL_DEPTH_TEST);
}


void FramebufferObject::Unbind(glm::vec2 viewportDimensions)
{
	glViewport(0, 0, viewportDimensions.x, viewportDimensions.y);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
}


void FramebufferObject::Clear(glm::vec3 clearColor, float alpha)
{
	Bind();

	glClearColor(clearColor.r, clearColor.g, clearColor.b, alpha);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void FramebufferObject::Destroy()
{
	postprocessEffects.clear();

	glDeleteFramebuffers(1, &id);
}


void FramebufferObject::ExposeImGui()
{
	if (ImGui::BeginCombo("Postprocessing Effect", postprocessEffects[currentEffect]->GetName().c_str(), ImGuiComboFlags_None))
	{
		for (int i = 0; i < postprocessEffects.size(); i++)
		{
			bool selected = postprocessEffects[currentEffect]->GetName() == postprocessEffects[i]->GetName();

			if (ImGui::Selectable(postprocessEffects[i]->GetName().c_str(), selected))
			{
				currentEffect = i;
			}

			if (selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	postprocessEffects[currentEffect]->ExposeImGui();
}

void FramebufferObject::AddEffect(PostprocessEffect* effect)
{
	effect->SetParent(this);
	postprocessEffects.push_back(effect);
}

void FramebufferObject::SetupShader()
{
	if (currentEffect >= postprocessEffects.size())
	{
		printf("Postprocessing effect vector out of bounds");
		return;
	}

	postprocessEffects[currentEffect]->SetupShader(colorAttachments);
}