#include "ViewportGui.h"



ViewportGui::ViewportGui()
{
	viewportTexture.SetTexImage(GL_RGB, 2, 2, GL_RGB, nullptr);
	printf("Texture valid: %d\n", glIsTexture(viewportTexture.tex));
	printf("GL version: %s\n", glGetString(GL_VERSION));
}

void ViewportGui::PostUpdate()
{
	unsigned char pixels[] = {
		255, 0, 0,    0, 255, 0, 
		0, 0, 255,    255, 255, 0  
	};
	viewportTexture.Bind();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	viewportTexture.SetTexSubImage(2, 2, GL_RGB, pixels);
}

void ViewportGui::Update()
{
	ImGui::Begin("Viewport");
	ImVec2 size = ImGui::GetContentRegionAvail();

	ImGui::Image((void*)(intptr_t)viewportTexture.tex, size);
	ImGui::End();
}
