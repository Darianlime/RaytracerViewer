#include "Gui/PropertiesGui.h"

PropertiesGui::PropertiesGui(UpdateGUIState& updateState) : updateState(updateState)
{
}

void PropertiesGui::PostUpdate()
{
}

void PropertiesGui::Update(ObjectFactory& objectFactory)
{
    updateState.deletingIndex = -1;
    auto markPropertiesUpdated = [&]() {
        if (ImGui::IsItemDeactivatedAfterEdit()) updateState.updateType = UpdateType::PROPERTIES_OBJECTS;
    };
    ModelFactory& models = objectFactory.GetFactory<ModelFactory>();
    LightFactory& lights = objectFactory.GetFactory<LightFactory>();

	// Objects window docker
    ImGui::Begin("Objects");
    int i = 0;
    for (unique_ptr<Model>& model : models.GetObjects()) {
        if (!model) continue;
        std::string label = model.get()->GetName() + std::to_string(i);
        bool selected = (updateState.selectedObjectIndex == i);
        if (ImGui::Selectable(label.c_str(), &selected)) {
            // Handle selection
			updateState.selectedObjectIndex = i;
			std::cout << "Selected model index: " << updateState.selectedObjectIndex << std::endl;
            selected = true;
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
                updateState.deletingIndex = i;
                updateState.updateType = UpdateType::DELETING_MODEL;
            }
            ImGui::EndPopup();
        }
        i++;
    }
    int j = models.GetObjects().size();
    for (unique_ptr<Light>& light : lights.GetObjects()) {
        std::string label = light.get()->GetName() + std::to_string(j - models.GetObjects().size());
        bool selected = (updateState.selectedObjectIndex == j);
        if (ImGui::Selectable(label.c_str(), &selected)) {
            // Handle selection
            updateState.selectedObjectIndex = j;
            std::cout << "Selected model index: " << updateState.selectedObjectIndex << std::endl;
            selected = true;
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
                updateState.deletingIndex = j - models.GetObjects().size();
                updateState.updateType = UpdateType::DELETING_LIGHT;
            }
            ImGui::EndPopup();
        }
        j++;
    }
    ImGui::End();

	// Properties window docker
    ImGui::Begin("Properties");
    if (ImGui::CollapsingHeader("Camera")) {
		Raytracer::Camera& cam = objectFactory.GetCameraIndex(0);
        ImGui::DragFloat3("Position", (float*)&cam.eye, 0.1f);
        if (ImGui::IsItemDeactivatedAfterEdit()) updateState.updateType = UpdateType::PROPERTIES_CAMERA;
        
        ImGui::DragFloat3("View Direction", (float*)&cam.viewdir, 0.1f);
        if (ImGui::IsItemDeactivatedAfterEdit()) { updateState.updateType = UpdateType::PROPERTIES_CAMERA; cam.CalcCoords(); };

        ImGui::DragFloat("Fov", &cam.vfov, 0.1f);
        if (ImGui::IsItemDeactivatedAfterEdit()) updateState.updateType = UpdateType::PROPERTIES_CAMERA;
    }
    if (ImGui::CollapsingHeader("Materials")) {
        for (int i = 0; i < objectFactory.GetMatsSize()-1; i++) {
            Material& mat = objectFactory.GetMatIndex(i);
            std::string label = "Material " + std::to_string(i);
            if (ImGui::TreeNode(label.c_str())) {

				ImGui::ColorEdit3("Diffuse", (float*)&mat.diffuse);
                markPropertiesUpdated();

                ImGui::ColorEdit3("Specular", (float*)&mat.specular);
                markPropertiesUpdated();

				ImGui::DragFloat3("Coefficients", (float*)&mat.k, 0.01f, 0.0f, 1.0f);
				markPropertiesUpdated();

				ImGui::DragFloat("Shininess", &mat.n, 0.1f, 0.0f);
                markPropertiesUpdated();

                ImGui::ColorEdit3("Alpha", (float*)&mat.alpha);
                markPropertiesUpdated();

                ImGui::DragFloat("Index of Refraction", &mat.refractionIndex, 0.1f, 1.0f, 4.1f);
                markPropertiesUpdated();

                if (ImGui::Button("Delete Material")) {
					updateState.deletingIndex = i;
                    updateState.updateType = UpdateType::DELETING_MATERIAL;
                }

                ImGui::TreePop();
            }
		}
        if (ImGui::Button("Add Material")) {
			std::cout << "adding material" << std::endl;
			objectFactory.AddMaterial(Material(Color(0.2f, 1.0f, 0.2f, false), Color(1.0f, 1.0f, 1.0f, false), Vec3(0.2f, 0.6f, 0.0f), 100, Color(1.0f, 1.0f, 1.0f, false), 1.0f));
            updateState.updateType = UpdateType::PROPERTIES_OBJECTS;
        }
    }

	// Display properties of the selected model
    if (updateState.selectedObjectIndex >= 0 && updateState.selectedObjectIndex < models.GetObjects().size()) {
        ImGui::SeparatorText("Transform");
        unique_ptr<Model>& model = models.GetObjects()[updateState.selectedObjectIndex];
        std::string label = model.get()->GetName();
        ImGui::Text("%s", label.c_str());

        ImGui::DragFloat3("Position", (float*)&model->pos, 0.1f);
        if (ImGui::IsItemDeactivatedAfterEdit()) { std::cout << "Updating model position" << std::endl; model->UpdateTransformation(); updateState.updateType = UpdateType::PROPERTIES_OBJECTS; }

        ImGui::DragFloat3("Rotation", (float*)&model->rot, 0.1f);
        if (ImGui::IsItemDeactivatedAfterEdit()) { std::cout << "Updating model rotation" << std::endl; model->UpdateTransformation(); updateState.updateType = UpdateType::PROPERTIES_OBJECTS; }

        ImGui::DragFloat3("Size", (float*)&model->size, 0.1f);
        if (ImGui::IsItemDeactivatedAfterEdit()) { std::cout << "Updating model size" << std::endl; model->UpdateTransformation(); updateState.updateType = UpdateType::PROPERTIES_OBJECTS; }

        ImGui::DragInt("Material Index", &model->mat, 0.1f);
        markPropertiesUpdated();

        ImGui::Checkbox("Smooth Shading", &model->isSmoothShadingOn);
        markPropertiesUpdated();
	}
	else if (updateState.selectedObjectIndex >= models.GetObjects().size() && updateState.selectedObjectIndex < models.GetObjects().size() + lights.GetObjects().size()) {
		ImGui::SeparatorText("Light Properties");
		int lightIndex = updateState.selectedObjectIndex - models.GetObjects().size();
		unique_ptr<Light>& light = lights.GetObjects()[lightIndex];
		std::string label = light.get()->GetName();
		ImGui::Text("%s", label.c_str());

		ImGui::DragFloat3("Position", (float*)&light->pos, 0.1f);
		if (ImGui::IsItemDeactivatedAfterEdit()) { std::cout << "Updating light position" << std::endl; light->SetLightDir(light->pos); updateState.updateType = UpdateType::PROPERTIES_OBJECTS; }

		ImGui::DragFloat("Intensity", &light->intensity, 0.1f);
		markPropertiesUpdated();

		ImGui::Text("Type: %s", Light::GetTypeMap()[light->type].c_str());
	}
    ImGui::End();
}


void PropertiesGui::Update()
{
}
