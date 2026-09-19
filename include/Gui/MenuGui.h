#ifndef MENU_GUI_H
#define MENU_GUI_H

#include "UseImGui.h"
#include "Factory/ObjectFactory.h"

#include "windows.h"
#include "GUIState.h"

class MenuGui: public UseImGui {
private:
	UpdateGUIState& updateState;
	HWND hwnd;
	string path = "";
public:
	MenuGui(HWND hwnd, UpdateGUIState& updateState);
	void PostUpdate() override;
	void Update(ObjectFactory& objectFactory);
	void Update() override;
	bool OpenFile(const char* filter);
	bool SaveFile(const char* filter);
};

#endif // !PROPERTIES_GUI_H