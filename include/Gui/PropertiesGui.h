#ifndef PROPERTIES_GUI_H
#define PROPERTIES_GUI_H

#include "UseImGui.h"
#include "Factory/ObjectFactory.h"

class PropertiesGui : public UseImGui {
private:
	int isUpdating;
public:
	PropertiesGui();
	void PostUpdate() override;
	void Update(ObjectFactory& objectFactory);
	void Update() override;

	int IsUpdating() const { return isUpdating; }

};

#endif // !PROPERTIES_GUI_H