#include "Gui/PropertiesGui.h"

PropertiesGui::PropertiesGui()
{
}

void PropertiesGui::PostUpdate()
{
}

void PropertiesGui::Update(ObjectFactory& objectFactory)
{
    isUpdating = false;
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
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = true;

                ImGui::ColorEdit3("Specular", (float*)&mat.specular);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = true;

				ImGui::DragFloat3("Specular Coefficients", (float*)&mat.k, 0.01f, 0.0f, 1.0f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = true;

				ImGui::DragFloat("Shininess", &mat.n, 0.1f, 0.0f, 100.0f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = true;

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
		std::vector<std::unique_ptr<Mesh>>& meshes = objectFactory.GetFactory<MeshFactory>().GetObjects();
        std::vector<std::unique_ptr<Light>>& lights = objectFactory.GetFactory<LightFactory>().GetObjects();
        int i = 0;
        for (unique_ptr<Mesh>& mesh : meshes) {
            std::string label = mesh.get()->GetName() + std::to_string(i);
            if (ImGui::TreeNode(label.c_str())) {
                ImGui::DragFloat3("Position", (float*)&mesh->pos, 0.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = true;

                ImGui::DragInt("Material Index", &mesh->mat, 0.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = true;

                ImGui::Text("Material Index: %d", mesh->mat);
                ImGui::Text("Texture Index: %d", mesh->tex);
                ImGui::Text("Type: %s", Mesh::GetTypeMap()[mesh->type].c_str());
                ImGui::TreePop();
            }
            i++;
		}
        int j = 0;
        for (unique_ptr<Light>& light : lights) {
            std::string label = light.get()->GetName() + std::to_string(i);
            if (ImGui::TreeNode(label.c_str())) {
                ImGui::DragFloat3("Position", (float*)&light->pos, 0.1f);
                light->SetLightDir(light->pos);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = true;

                ImGui::DragFloat("Intensity", &light->intensity, 0.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = true;

                ImGui::Text("Type: %s", Light::GetTypeMap()[light->type].c_str());
                ImGui::TreePop();
            }
            j++;
        }
	}
    ImGui::End();
}


void PropertiesGui::Update()
{
}
