#include "Gui/PropertiesGui.h"

PropertiesGui::PropertiesGui()
{
}

void PropertiesGui::PostUpdate()
{
}

void PropertiesGui::Update(ObjectFactory& objectFactory)
{
    isUpdating = 0;
    ImGui::Begin("Properties");
    if (ImGui::CollapsingHeader("Camera")) {
		Raytracer::Camera& cam = objectFactory.GetCameraIndex(0);
        ImGui::DragFloat3("Position", (float*)&cam.eye, 0.1f);
        if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_CAMERA;
        
        ImGui::DragFloat3("View Direction", (float*)&cam.viewdir, 0.1f);
        if (ImGui::IsItemDeactivatedAfterEdit()) { isUpdating = UpdateType::PROPERTIES_CAMERA; cam.CalcCoords(); };

        ImGui::DragFloat("Fov", &cam.vfov, 0.1f);
        if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_CAMERA;
    }
    if (ImGui::CollapsingHeader("Materials")) {
        for (int i = 0; i < objectFactory.GetMatsSize()-1; i++) {
            Material& mat = objectFactory.GetMatIndex(i);
            std::string label = "Material " + std::to_string(i);
            if (ImGui::TreeNode(label.c_str())) {
				ImGui::ColorEdit3("Diffuse", (float*)&mat.diffuse);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

                ImGui::ColorEdit3("Specular", (float*)&mat.specular);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

				ImGui::DragFloat3("Coefficients", (float*)&mat.k, 0.01f, 0.0f, 1.0f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

				ImGui::DragFloat("Shininess", &mat.n, 0.1f, 0.0f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

                ImGui::ColorEdit3("Alpha", (float*)&mat.alpha);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

                ImGui::DragFloat("Index of Refraction", &mat.refractionIndex, 0.1f, 1.0f, 4.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;
                ImGui::TreePop();
            }
		}
        if (ImGui::Button("Add Material")) {
			std::cout << "adding material" << std::endl;
			objectFactory.AddMaterial(Material(Color(0.2f, 1.0f, 0.2f, false), Color(1.0f, 1.0f, 1.0f, false), Vec3(0.2f, 0.6f, 0.0f), 100, Color(1.0f, 1.0f, 1.0f, false), 1.0f));
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
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

                ImGui::DragFloat3("Rotation", (float*)&mesh->rot, 0.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

                ImGui::DragFloat3("Size", (float*)&mesh->size, 0.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

                ImGui::DragInt("Material Index", &mesh->mat, 0.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

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
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

                ImGui::DragFloat("Intensity", &light->intensity, 0.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

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
