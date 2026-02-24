#ifndef VIEWPORTGUI_H
#define VIEWPORTGUI_H

#include "UseImGui.h"
#include "Texture.h"

class ViewportGui : public UseImGui {
	private:
		Texture viewportTexture;
	public:
		ViewportGui();
		void PostUpdate() override;
		void Update() override;
	};

#endif // !VIEWPORTGUI_H