#ifndef TIMELINE_GUI_H
#define TIMELINE_GUI_H

#include "UseImGui.h"
#include "Factory/ObjectFactory.h"
#include "Animation/imgui_neo_sequencer.h"
#include "GUIState.h"

struct Keyframe {
	ImGui::FrameIndexType frame;
	Vec3 value; // This could represent position, rotation, or scale depending on the context
};

struct ObjectKeys {
	std::vector<Keyframe> positionKeys;
	std::vector<Keyframe> rotationKeys;
	std::vector<Keyframe> scaleKeys;
};

class TimelineGui : public UseImGui {
private:
	ObjectFactory& objectFactory;
	UpdateGUIState& updateState;
	std::vector<ObjectKeys> objectKeyframes;
	int currentObjectSize = 0;
	int isUpdating;
	int32_t currentFrame;
	int32_t startFrame;
	int32_t endFrame;
	bool doDelete = false;
public:
	TimelineGui(ObjectFactory& objectFactory, UpdateGUIState& updateState);
	void PostUpdate() override;
	void Update() override;

	int IsUpdating() const { return isUpdating; }
	std::vector<ObjectKeys>& GetObjectKeyframes();
	void HandleKeyFrames(std::vector<Keyframe>& keyframes, Vec3 value);
};

#endif // !PROPERTIES_GUI_H