#ifndef VIEWPORTGUI_H
#define VIEWPORTGUI_H

#include "UseImGui.h"

class PropertiesGui : public UseImGui {

public:
	PropertiesGui();
	void Update() override;
};

#endif // !VIEWPORTGUI_H