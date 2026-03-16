#include "Gui/MenuGui.h"

MenuGui::MenuGui()
{
}

void MenuGui::PostUpdate()
{
}


void MenuGui::Update(ObjectFactory& objectFactory)
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
            if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
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
                    }
                }
		    }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void MenuGui::Update()
{
}
