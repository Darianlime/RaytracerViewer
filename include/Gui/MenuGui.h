#ifndef MENU_GUI_H
#define MENU_GUI_H

#include "UseImGui.h"
#include "Factory/ObjectFactory.h"

class MenuGui: public UseImGui {
private:
public:
	MenuGui();
	void PostUpdate() override;
	void Update(ObjectFactory& objectFactory);
	void Update() override;

};

#endif // !PROPERTIES_GUI_H