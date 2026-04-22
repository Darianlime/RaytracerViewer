#include "Gui/MenuGui.h"
#include <file.h>
#include <stdexcept>

MenuGui::MenuGui(HWND hwnd) : hwnd(hwnd), isUpdating(0) {}

void MenuGui::PostUpdate()
{
}


void MenuGui::Update(ObjectFactory& objectFactory)
{
	isUpdating = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
            if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
			if (ImGui::MenuItem("Import OBJ", "Ctrl+Shift+I")) { 
                if (OpenFile("OBJ Files\0*.obj\0""\0")) {
                    objectFactory.GetFactory<ModelFactory>().ResetCurrentVertexStart();
                    int intialSize = objectFactory.GetFactory<ModelFactory>().GetObjects().size();
                    if (File::ReadOBJ(path, -1, -1, objectFactory) == -1) {
                        throw std::runtime_error("Failed to read OBJ file");
                    }
                    int updatedSize = objectFactory.GetFactory<ModelFactory>().GetObjects().size();
                    for (int i = intialSize; i < updatedSize; i++) {
						Model* model = objectFactory.GetFactory<ModelFactory>().GetObjects()[i].get();
						for (auto& vertex : model->GetOrgVertices()) {
							vertex = vertex - model->pos; // Center the model at the origin
						}
						std::cout << "model pos: " << model->pos.x << ", " << model->pos.y << ", " << model->pos.z << std::endl;
                        model->rot.ToString();
                        model->SetMatrix(model->pos, model->rot, model->size);
                        model->GetBVH().Init();
                        model->GetBVH().Build();
						model->localToWorld.ToString();
                    }
                    isUpdating = UpdateType::PROPERTIES_OBJECTS;
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Objects"))
        {
            for (auto& factory : objectFactory.GetFactoryMap()) {
			    int typeMapSize = factory.second->GetTypeMapSize();
                for (int i = 0; i < typeMapSize; i++) {
                    std::string label = factory.second->GetTypeIndex(i);
                    std::string shortcut = "Ctrl+" + std::to_string(i);
                    if (ImGui::MenuItem(label.c_str(), shortcut.c_str())) {
				        std::cout << "creating object: " << std::endl;
					    factory.second->CreateObject(factory.second->GetTypeIndex(i), std::vector<std::string>());
						isUpdating = UpdateType::PROPERTIES_OBJECTS;
                    }
                }
		    }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

bool MenuGui::OpenFile(const char* filter)
{
    OPENFILENAME ofn;
    char fileName[512]{};
    //const char filters[] =
    //    "All Files\0*.*\0"
    //    "OBJ Files\0*.obj\0"
    //    "\0";

    ZeroMemory(&ofn, sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = fileName;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 512;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 2;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        path = fileName;
		std::cout << "Selected file: " << path << std::endl;
        return true;
    }
	return false;
}

void MenuGui::Update()
{
}
