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
	updatingIndex = -1;
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

                if (ImGui::Button("Delete Material")) {
					updatingIndex = i;
                    isUpdating = UpdateType::DELETING_MATERIAL;
                }

                ImGui::TreePop();
            }
		}
        if (ImGui::Button("Add Material")) {
			std::cout << "adding material" << std::endl;
			objectFactory.AddMaterial(Material(Color(0.2f, 1.0f, 0.2f, false), Color(1.0f, 1.0f, 1.0f, false), Vec3(0.2f, 0.6f, 0.0f), 100, Color(1.0f, 1.0f, 1.0f, false), 1.0f));
            isUpdating = UpdateType::PROPERTIES_OBJECTS;
        }
    }
    if (ImGui::CollapsingHeader("Objects"))
    {
        ModelFactory& models = objectFactory.GetFactory<ModelFactory>();
        LightFactory& lights = objectFactory.GetFactory<LightFactory>();
        int i = 0;
        for (unique_ptr<Model>& model : models.GetObjects()) {
			if (!model) continue;
            std::string label = model.get()->GetName() + std::to_string(i);
            if (ImGui::TreeNode(label.c_str())) {
                ImGui::DragFloat3("Position", (float*)&model->pos, 0.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) { model->UpdateTransformation(); isUpdating = UpdateType::PROPERTIES_OBJECTS; }

                ImGui::DragFloat3("Rotation", (float*)&model->rot, 0.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) { model->UpdateTransformation(); isUpdating = UpdateType::PROPERTIES_OBJECTS; }

                ImGui::DragFloat3("Size", (float*)&model->size, 0.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) { model->UpdateTransformation(); isUpdating = UpdateType::PROPERTIES_OBJECTS; }

                ImGui::DragInt("Material Index", &model->mat, 0.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) { model->UpdateTransformation(); isUpdating = UpdateType::PROPERTIES_OBJECTS; }

                if (ImGui::Button("Delete Model")) {
					updatingIndex = i;
					isUpdating = UpdateType::DELETING_MODEL;
                }

                ImGui::TreePop();
            }
            i++;
		}
        int j = 0;
        for (unique_ptr<Light>& light : lights.GetObjects()) {
            std::string label = light.get()->GetName() + std::to_string(j);
            if (ImGui::TreeNode(label.c_str())) {
                ImGui::DragFloat3("Position", (float*)&light->pos, 0.1f);
                light->SetLightDir(light->pos);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

                ImGui::DragFloat("Intensity", &light->intensity, 0.1f);
                if (ImGui::IsItemDeactivatedAfterEdit()) isUpdating = UpdateType::PROPERTIES_OBJECTS;

                ImGui::Text("Type: %s", Light::GetTypeMap()[light->type].c_str());
                if (ImGui::Button("Delete Light")) {
					updatingIndex = j;
                    isUpdating = UpdateType::DELETING_LIGHT;
                }
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
