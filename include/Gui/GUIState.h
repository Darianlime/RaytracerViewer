#pragma once

enum UpdateType {
	NONE = 0,
	PROPERTIES_OBJECTS = 1 << 0,
	PROPERTIES_CAMERA = 1 << 1,
	DELETING_MODEL = 1 << 2,
	DELETING_LIGHT = 1 << 3,
	DELETING_MATERIAL = 1 << 4,
	CREATING_OBJECT = 1 << 5
};

struct UpdateGUIState {
	int updateType = NONE;	
	int deletingIndex = -1;
	int selectedObjectIndex = -1;
};

