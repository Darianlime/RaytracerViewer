#ifndef PROPERTIES_GUI_H
#define PROPERTIES_GUI_H

#include "UseImGui.h"
#include "Factory/ObjectFactory.h"

class PropertiesGui : public UseImGui {
private:
	int isUpdating;
	int updatingIndex;
public:
	PropertiesGui();
	void PostUpdate() override;
	void Update(ObjectFactory& objectFactory);
	void Update() override;

	int IsUpdating() const { return isUpdating; }
	int GetUpdatingIndex() const { return updatingIndex; }

};

#endif // !PROPERTIES_GUI_H