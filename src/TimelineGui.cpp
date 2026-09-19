#include "Gui/TimelineGui.h"

static bool transformOpen = false;

TimelineGui::TimelineGui(ObjectFactory& objectFactory, UpdateGUIState& updateState) : objectFactory(objectFactory), updateState(updateState), isUpdating(0), currentFrame(0), startFrame(-10), endFrame(64), currentObjectSize(0), objectKeyframes{}, doDelete(true) {
    objectKeyframes.push_back(ObjectKeys());
    objectKeyframes.back().positionKeys.push_back({ 10, Vec3(0.0f, 0.0f, 0.0f) });
    currentObjectSize++;
}

std::vector<ObjectKeys>& TimelineGui::GetObjectKeyframes() {
	return objectKeyframes;
}

void TimelineGui::HandleKeyFrames(std::vector<Keyframe>& keyframes, Vec3 value) {
    if (ImGui::IsKeyPressed(ImGuiKey_I)) {
		keyframes.push_back({currentFrame, value});
    }

    for (auto& v : keyframes) {
        ImGui::NeoKeyframe(&v.frame);
    }

    uint32_t count = ImGui::GetNeoKeyframeSelectionSize();
    ImGui::FrameIndexType* toRemove = new ImGui::FrameIndexType[count];
    ImGui::GetNeoKeyframeSelection(toRemove);

	// Delete selected keyframes when the Delete key is pressed
    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        if (count > 0) {
            for (uint32_t i = 0; i < count; ++i)
            {
                auto& keys = keyframes;
                keys.erase(std::remove_if(keys.begin(), keys.end(),
                    [toRemove, i](const Keyframe& kf) { return kf.frame == toRemove[i]; }),
                    keys.end());
            }
        }
    }
}

void TimelineGui::PostUpdate()
{
}

void TimelineGui::Update()
{
    if (updateState.updateType == (int)UpdateType::CREATING_OBJECT) {
		std::cout << "Creating new object keyframe" << std::endl;
		objectKeyframes.push_back(ObjectKeys());
        currentObjectSize++;
    }
	if (updateState.updateType == (int)UpdateType::DELETING_MODEL) {
		objectKeyframes.erase(objectKeyframes.begin() + updateState.deletingIndex);
		currentObjectSize--;
	}
    
	ImGui::Begin("Timeline");

    if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame, &endFrame, { 0, 0 },
        ImGuiNeoSequencerFlags_EnableSelection | ImGuiNeoSequencerFlags_Selection_EnableDragging |
        ImGuiNeoSequencerFlags_Selection_EnableDeletion | ImGuiNeoSequencerFlags_AllowLengthChanging))
    {
        if (ImGui::BeginNeoGroup("Transform", &transformOpen)) {
            auto& selectedObject = objectFactory.GetFactory<ModelFactory>().GetObjects()[updateState.selectedObjectIndex];
            if (ImGui::BeginNeoTimelineEx("Position")) {
				HandleKeyFrames(objectKeyframes[updateState.selectedObjectIndex].positionKeys, selectedObject->pos);
                ImGui::EndNeoTimeLine();
            }
            if (ImGui::BeginNeoTimelineEx("Rotation")) {
                HandleKeyFrames(objectKeyframes[updateState.selectedObjectIndex].rotationKeys, selectedObject->rot);
                ImGui::EndNeoTimeLine();
            }
            if (ImGui::BeginNeoTimelineEx("Scale")) {
                HandleKeyFrames(objectKeyframes[updateState.selectedObjectIndex].scaleKeys, selectedObject->size);
                ImGui::EndNeoTimeLine();
            }
            ImGui::EndNeoGroup();
        }
        ImGui::EndNeoSequencer();
    }
	ImGui::End();
}
