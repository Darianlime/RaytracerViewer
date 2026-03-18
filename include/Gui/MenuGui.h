#ifndef MENU_GUI_H
#define MENU_GUI_H

#include "UseImGui.h"
#include "Factory/ObjectFactory.h"

class MenuGui: public UseImGui {
private:
	bool isUpdating = false;
public:
	MenuGui();
	void PostUpdate() override;
	void Update(ObjectFactory& objectFactory);
	void Update() override;

	bool IsUpdating() const { return isUpdating; }
};

#endif // !PROPERTIES_GUI_H