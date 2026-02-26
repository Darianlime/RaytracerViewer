#ifndef PROPERTIES_GUI_H
#define PROPERTIES_GUI_H

#include "UseImGui.h"

class PropertiesGui : public UseImGui {

public:
	PropertiesGui();
	void Update() override;
};

#endif // !PROPERTIES_GUI_H