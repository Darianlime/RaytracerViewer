#ifndef MENU_GUI_H
#define MENU_GUI_H

#include "UseImGui.h"
#include <Gui/ViewportGui.h>
#include <Gui/TimelineGui.h>

#include "windows.h"
#include "GUIState.h"

class MenuGui: public UseImGui {
private:
	bool openExportPopup = false;
	int exportW = 1920, exportH = 1080, exportFps = 24;

	ObjectFactory& objectFactory;
	ViewportGui& viewportGui;
	TimelineGui& timelineGui;
	UpdateGUIState& updateState;
	std::thread exportThread;
	std::mutex exportMutex;
	std::condition_variable exportCv;
	bool exportRequested = false;          
	bool quitting = false;   
	std::string exportPath;
	HWND hwnd;
	std::string path = "";
	std::atomic<bool> isExporting{ false };
	bool ffmpegAvailable = false;
	bool openFfmpegPopup = false;
public:
	MenuGui(HWND hwnd, ObjectFactory& objectFactory, ViewportGui& viewportGui, TimelineGui& timelineGui, UpdateGUIState& updateState);
	~MenuGui();
	void PostUpdate() override;
	void Update() override;

	void ExportWorker();
	bool RunExport(const std::string& outPath, int w, int h, int fps);

	bool OpenFile(const char* filter);
	bool SaveFile(const char* filter);
	void DrawExportPopup();
	bool CheckFfmpeg();
	void DrawFfmpegPopup();

	void SetPath(const std::string& newPath) { path = newPath; }

	void CreatePresetObj(const std::string& objPath);


};

#endif // !PROPERTIES_GUI_H