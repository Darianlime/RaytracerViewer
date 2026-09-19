#ifndef PROPERTIES_GUI_H
#define PROPERTIES_GUI_H

#include "UseImGui.h"
#include "Factory/ObjectFactory.h"
#include "GUIState.h"

class PropertiesGui : public UseImGui {
private:
	UpdateGUIState& updateState;
public:
	PropertiesGui(UpdateGUIState& updateState);
	void PostUpdate() override;
	void Update(ObjectFactory& objectFactory);
	void Update() override;

};

#endif // !PROPERTIES_GUI_H