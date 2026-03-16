#include "Gui/PropertiesGui.h"

PropertiesGui::PropertiesGui()
{
}

void PropertiesGui::PostUpdate()
{
}

void PropertiesGui::Update(ObjectFactory& objectFactory)
{
    ImGui::Begin("Properties");
    if (ImGui::CollapsingHeader("Camera"))
    {

    }
    if (ImGui::CollapsingHeader("Materials")) {
        for (int i = 0; i < objectFactory.GetMatsSize(); i++) {
            Material& mat = objectFactory.GetMatIndex(i);
            std::string label = "Material " + std::to_string(i);
            if (ImGui::TreeNode(label.c_str())) {
				ImGui::ColorEdit3("Diffuse", (float*)&mat.diffuse);
                ImGui::Text("Diffuse: %.2f, %.2f, %.2f", mat.diffuse.r, mat.diffuse.g, mat.diffuse.b);
                ImGui::Text("Specular: %.2f, %.2f, %.2f", mat.specular.r, mat.specular.g, mat.specular.b);
                ImGui::Text("Specular Coefficients: %.2f, %.2f, %.2f", mat.k.x, mat.k.y, mat.k.z);
                ImGui::Text("Shininess: %.2f", mat.n);
                ImGui::TreePop();
            }
		}
    }
    if (ImGui::CollapsingHeader("Objects"))
    {
        /*for (auto& factory : objectFactory.GetFactoryMap()) {
            objectFactory.
        }*/
	}
    ImGui::End();
}


void PropertiesGui::Update()
{
}
