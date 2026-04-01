#ifndef MENU_GUI_H
#define MENU_GUI_H

#include "UseImGui.h"
#include "Factory/ObjectFactory.h"

class MenuGui: public UseImGui {
private:
	int isUpdating;
public:
	MenuGui();
	void PostUpdate() override;
	void Update(ObjectFactory& objectFactory);
	void Update() override;

	int IsUpdating() const { return isUpdating; }
};

#endif // !PROPERTIES_GUI_H