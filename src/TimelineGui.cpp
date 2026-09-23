#include "Gui/TimelineGui.h"
#include <algorithm>
#include <chrono>

static bool transformOpen = false;

static Vec3 Lerp(const Vec3& start, const Vec3& end, float t) {
    return Vec3(start.x + (end.x - start.x) * t, start.y + (end.y - start.y) * t, start.z + (end.z - start.z) * t);
}

TimelineGui::TimelineGui(ObjectFactory& objectFactory, UpdateGUIState& updateState) : objectFactory(objectFactory), updateState(updateState), isUpdating(0), currentFrame(0), previousFrame(0), startFrame(0), endFrame(64), accumulator(0.0f), currentObjectSize(0), objectKeyframes{}, doDelete(true), wasDragging(false) {
    objectKeyframes.push_back(ObjectKeys());
    currentObjectSize++;
}

std::vector<ObjectKeys>& TimelineGui::GetObjectKeyframes() {
	return objectKeyframes;
}

void TimelineGui::SortKeyframes(std::vector<Keyframe>& keyframes) {
    std::sort(keyframes.begin(), keyframes.end(),
        [](const Keyframe& a, const Keyframe& b)
        {
            return a.frame < b.frame;
        });
}

Keyframe* TimelineGui::FindKeyframe(std::vector<Keyframe>& keys, ImGui::FrameIndexType frame) {
    auto it = std::lower_bound(keys.begin(), keys.end(), frame,
        [](const Keyframe& key, ImGui::FrameIndexType frame)
        {
            return key.frame < frame;
        });

    if (it != keys.end() && it->frame == frame)
        return &(*it);

    return nullptr;
}

int TimelineGui::FindLeftKeyframeIndex(std::vector<Keyframe>& keys, ImGui::FrameIndexType target) {
    if (keys.empty()) {
        return -1;
    }

    auto it = std::lower_bound(keys.begin(), keys.end(), target,
        [](const Keyframe& key, ImGui::FrameIndexType frame)
        {
            return key.frame < frame;
        });

    if (it == keys.begin()) {
        if (it->frame > target) {
            return -1;
        }
        return 0;
    }
    if (it == keys.end()) {
        return static_cast<int>(keys.size() - 1);
    }
    if (it->frame == target) {
        return static_cast<int>(it - keys.begin());
    }
    return static_cast<int>(it - keys.begin()) - 1;
}

bool TimelineGui::LerpKeyframes(ImGui::FrameIndexType currentFrame, std::vector<Keyframe>& keys, Vec3& result) {
    if (keys.size() == 1 && keys[0].frame == currentFrame) {
        result = keys[0].value;
        return true;
    }
    if (keys.size() > 1) {
        int cloestLeftIndex = FindLeftKeyframeIndex(keys, currentFrame);
        if (cloestLeftIndex != -1 && cloestLeftIndex + 1 < keys.size()) {
            float t = (float)(currentFrame - keys[cloestLeftIndex].frame) / (keys[cloestLeftIndex+1].frame - keys[cloestLeftIndex].frame);
            result = Lerp(keys[cloestLeftIndex].value, keys[cloestLeftIndex+1].value, t);
            return true;
        }
    }
    return false;
}

void TimelineGui::HandleKeyFrames(std::vector<Keyframe>& keyframes, Vec3 value) {
	// Add a new keyframe when the 'I' key is pressed
    if (ImGui::IsKeyPressed(ImGuiKey_I)) {
		keyframes.push_back({currentFrame, value});
		SortKeyframes(keyframes);
    }

	// Sort keyframes when dragging ends
    bool dragging = ImGui::NeoIsDraggingSelection();
    if (wasDragging && !dragging) {
		SortKeyframes(keyframes);
    }
    wasDragging = dragging;

	// Draw keyframes
    for (auto& v : keyframes) {
        ImGui::NeoKeyframe(&v.frame);
    }

    // Delete selected keyframes when the Delete key is pressed
    uint32_t count = ImGui::GetNeoKeyframeSelectionSize();
    ImGui::FrameIndexType* toRemove = new ImGui::FrameIndexType[count];
    ImGui::GetNeoKeyframeSelection(toRemove);
    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        if (count > 0) {
            for (uint32_t i = 0; i < count; ++i) {
                auto& keys = keyframes;
                keys.erase(std::remove_if(keys.begin(), keys.end(),
                    [toRemove, i](const Keyframe& kf) { return kf.frame == toRemove[i]; }),
                    keys.end());
            }
        }
    }
}

void TimelineGui::DrawPlaybackControls() {
    float buttonWidth = 40.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;

    float totalWidth =
        buttonWidth * 5 +
        spacing * 4 +
        100.0f;

    float availableWidth = ImGui::GetContentRegionAvail().x;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availableWidth - totalWidth) * 0.6f);

    if (ImGui::Button("<", ImVec2(buttonWidth, 0))) {
        if (currentFrame > startFrame) {
            --currentFrame;
        }
    }
    ImGui::SameLine();

    if (ImGui::Button(playing ? "||" : "Play", ImVec2(buttonWidth, 0)))
    {
        playing = !playing;
    }

    ImGui::SameLine();

    if (ImGui::Button(">", ImVec2(buttonWidth, 0)))
    {
        if (currentFrame < endFrame) {
            ++currentFrame;
        }
    }
    ImGui::SameLine();
    ImGui::Text("Frame: %d / %d", currentFrame, endFrame);
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

    DrawPlaybackControls();

	if (playing) {
        accumulator += ImGui::GetIO().DeltaTime;
        if (accumulator >= FRAME_TIME)
        {
            currentFrame++;
            accumulator = 0.0f;

            if (currentFrame > endFrame)
                currentFrame = startFrame;
        }
	}

    if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame, &endFrame, { 0, 0 },
        ImGuiNeoSequencerFlags_EnableSelection | ImGuiNeoSequencerFlags_Selection_EnableDragging |
        ImGuiNeoSequencerFlags_Selection_EnableDeletion | ImGuiNeoSequencerFlags_AllowLengthChanging))
    {
        if (updateState.selectedObjectIndex < 0 || updateState.selectedObjectIndex >= currentObjectSize) {
            ImGui::EndNeoSequencer();
            ImGui::End();
            return;
        }
        auto& selectedObject = objectFactory.GetFactory<ModelFactory>().GetObjects()[updateState.selectedObjectIndex];
        auto& positionKeys = objectKeyframes[updateState.selectedObjectIndex].positionKeys;
        auto& rotationKeys = objectKeyframes[updateState.selectedObjectIndex].rotationKeys;
        auto& scaleKeys = objectKeyframes[updateState.selectedObjectIndex].scaleKeys;
		// Draw the transform group
        if (ImGui::BeginNeoGroup("Transform", &transformOpen)) {
            if (ImGui::BeginNeoTimelineEx("Position")) {
                HandleKeyFrames(positionKeys, selectedObject->pos);
                ImGui::EndNeoTimeLine();
            }
            if (ImGui::BeginNeoTimelineEx("Rotation")) {
                HandleKeyFrames(rotationKeys, selectedObject->rot);
                ImGui::EndNeoTimeLine();
            }
            if (ImGui::BeginNeoTimelineEx("Scale")) {
                HandleKeyFrames(scaleKeys, selectedObject->size);
                ImGui::EndNeoTimeLine();
            }
            ImGui::EndNeoGroup();
        }
        ImGui::EndNeoSequencer();


        if (currentFrame != previousFrame)
        {
            bool changed = false;

            changed = LerpKeyframes(currentFrame, positionKeys, selectedObject->pos) || changed;
            changed = LerpKeyframes(currentFrame, rotationKeys, selectedObject->rot) || changed;
            changed = LerpKeyframes(currentFrame, scaleKeys, selectedObject->size) || changed;

            //if (!changed) {
            //    Keyframe* positionKeyframe = FindKeyframe(positionKeys, currentFrame);
            //    Keyframe* rotationKeyframe = FindKeyframe(rotationKeys, currentFrame);
            //    Keyframe* scaleKeyframe = FindKeyframe(scaleKeys, currentFrame);

            //    if (positionKeyframe) {
            //        selectedObject->pos = positionKeyframe->value;
            //        changed = true;
            //    }
            //    if (rotationKeyframe) {
            //        selectedObject->rot = rotationKeyframe->value;
            //        changed = true;
            //    }
            //    if (scaleKeyframe) {
            //        selectedObject->size = scaleKeyframe->value;
            //        changed = true;
            //    }
            //}
            if (changed) {
                selectedObject->UpdateTransformation();
                updateState.updateType = UpdateType::PROPERTIES_OBJECTS;
            }
        }
        previousFrame = currentFrame;
    }
	ImGui::End();
}
