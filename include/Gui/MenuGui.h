#ifndef MENU_GUI_H
#define MENU_GUI_H

#include "UseImGui.h"
#include "Factory/ObjectFactory.h"

#include "windows.h"

class MenuGui: public UseImGui {
private:
	int isUpdating;
	HWND hwnd;
	string path = "";
public:
	MenuGui(HWND hwnd);
	void PostUpdate() override;
	void Update(ObjectFactory& objectFactory);
	void Update() override;
	bool OpenFile(const char* filter);

	int IsUpdating() const { return isUpdating; }
};

#endif // !PROPERTIES_GUI_H