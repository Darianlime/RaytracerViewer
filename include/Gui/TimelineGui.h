#ifndef TIMELINE_GUI_H
#define TIMELINE_GUI_H

#include "UseImGui.h"
#include "Animation/imgui_neo_sequencer.h"
#include "GUIState.h"
#include <unordered_set>

#define FPS 24
#define FRAME_TIME (1.0f / FPS)	

enum class Channel { None, Transform, Position, Rotation, Scale };
static Channel selectedChannel = Channel::None;

struct FrameChange {
	ImGui::FrameIndexType frame;
	Vec3 position;
	Vec3 rotation;
	Vec3 scale;
};

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
	float accumulator;
	int32_t currentFrame;
	int32_t previousFrame;
	int32_t startFrame;
	int32_t endFrame;
	bool doDelete = false;
	bool wasDragging = false;
	bool playing = false;
	bool capToFps = true;

	std::atomic<bool> exportLocked{ false };

	void SortKeyframes(std::vector<Keyframe>& keyframes);
	Keyframe* FindKeyframe(std::vector<Keyframe>& keys, ImGui::FrameIndexType frame);
	int FindLeftKeyframeIndex(std::vector<Keyframe>& keys, ImGui::FrameIndexType target);
	bool LerpKeyframes(ImGui::FrameIndexType currentFrame, std::vector<Keyframe>& keys, Vec3& result);
	void DrawPlaybackControls();
public:
	TimelineGui(ObjectFactory& objectFactory, UpdateGUIState& updateState);
	void PostUpdate() override;
	void Update() override;

	int IsUpdating() const { return isUpdating; }
	std::vector<ObjectKeys>& GetObjectKeyframes();
	void HandleKeyFrames(std::vector<Keyframe>& keyframes, Vec3 value, Channel channel);
	bool LerpTransform(ObjectFactory& objectFactory);

	int32_t GetCurrentFrame() const { return currentFrame; }
	int32_t GetStartFrame() const { return startFrame; }
	int32_t GetEndFrame() const { return endFrame; }
	int32_t SetCurrentFrame(int32_t frame) { currentFrame = frame; return currentFrame; }

	void SetExportLock(bool v) { exportLocked = v; }
	std::function<bool()> isRenderIdle = [] { return true; };
};

#endif // !PROPERTIES_GUI_H