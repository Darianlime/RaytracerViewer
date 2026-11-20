#include "Gui/MenuGui.h"
#include <file.h>
#include <stdexcept>
#include <chrono>
#include <cmath>

MenuGui::MenuGui(HWND hwnd, ObjectFactory& objectFactory, ViewportGui& viewportGui, TimelineGui& timelineGui, UpdateGUIState& updateState) : 
    hwnd(hwnd), objectFactory(objectFactory), viewportGui(viewportGui), timelineGui(timelineGui), updateState(updateState) {

    exportThread = std::thread(&MenuGui::ExportWorker, this);
}

MenuGui::~MenuGui() {
    {
        std::lock_guard<std::mutex> lock(exportMutex);
        quitting = true;
    }
    exportCv.notify_one();
    if (exportThread.joinable()) {
        exportThread.join();
    }
}

void MenuGui::PostUpdate()
{
}


void MenuGui::Update()
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Import OBJ", "Ctrl+Shift+I")) { 
                if (OpenFile("OBJ Files\0*.obj\0""\0")) {
                    objectFactory.CreateObj(path);
                    updateState.updateType = UpdateType::CREATING_OBJECT;
                }
            }
            if (ImGui::MenuItem("Export Animation", "Ctrl+Shift+E", false, !isExporting)) {
                ffmpegAvailable = CheckFfmpeg();     
                if (ffmpegAvailable) {
                    openExportPopup = true;
                } else {
                    openFfmpegPopup = true;
                }
            }
			//if (ImGui::MenuItem("Export PPM Image", "Ctrl+Shift+E")) {
   //             if (SaveFile("All Files\0*.*\0""\0")) {
   //                 std::cout << "Exporting image to: " << path << std::endl;
			//		
   //             }
   //         }
			if (ImGui::MenuItem("Exit", "Alt+F4")) { /* Do stuff */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Objects"))
        {
            for (auto& factory : objectFactory.GetFactoryMap()) {
			    int typeMapSize = factory.second->GetTypeMapSize();
                for (int i = 0; i < typeMapSize; i++) {
                    std::string label = factory.second->GetTypeIndex(i);
                    std::string shortcut = "Ctrl+" + std::to_string(i);
                    if (ImGui::MenuItem(label.c_str(), shortcut.c_str())) {
				        std::cout << "creating object: " << label << std::endl;
					    factory.second->CreateObject(factory.second->GetTypeIndex(i), std::vector<std::string>());
						updateState.updateType = UpdateType::CREATING_OBJECT;
                    }
                }
		    }
            if (ImGui::MenuItem("Plane", "")) {
                CreatePresetObj("C:/dev/RaytracerViewer/RaytracerViewer/assets/plane.obj");
                Model* model = objectFactory.GetFactory<ModelFactory>().GetObjects().back().get();
                model->checker.enabled = true;
                model->checker.tileSize = 1.0f;
            }
            if (ImGui::MenuItem("Stanford Bunny", "")) {
                CreatePresetObj("C:/dev/RaytracerViewer/RaytracerViewer/assets/bunny.obj");
            }
            if (ImGui::MenuItem("Stanford Dragon 202K", "")) {
                CreatePresetObj("C:/dev/RaytracerViewer/RaytracerViewer/assets/dragon202k.obj");
            }
            if (ImGui::MenuItem("Stanford Dragon 871k", "")) {
                CreatePresetObj("C:/dev/RaytracerViewer/RaytracerViewer/assets/dragon871k.obj");
            }
            if (ImGui::MenuItem("Stanford Buddha", "")) {
                CreatePresetObj("C:/dev/RaytracerViewer/RaytracerViewer/assets/buddha.obj");
            }
            ImGui::EndMenu();
        }
        //if (ImGui::BeginMenu("Resolu"))
        //{
        //    
        //    ImGui::EndMenu();
        //}
        ImGui::EndMainMenuBar();
        DrawExportPopup();
		DrawFfmpegPopup();
    }
}

void MenuGui::CreatePresetObj(const std::string& objPath) {
    std::cout << "creating object: " << objPath << std::endl;
    objectFactory.CreateObj(objPath);
    updateState.updateType = UpdateType::CREATING_OBJECT;
}

void MenuGui::ExportWorker() {
    while (true) {
        int w, h, fps;
        std::string outPath;
        {
            std::unique_lock<std::mutex> lock(exportMutex);
            exportCv.wait(lock, [this] { return exportRequested || quitting; });
            if (quitting) return;
            outPath = exportPath;  
            w = exportW; h = exportH; fps = exportFps;
            exportRequested = false;
            isExporting = true;
            viewportGui.SetExportLock(true);
            timelineGui.SetExportLock(true);
        }
        RunExport(outPath, w, h, fps);
        viewportGui.SetExportLock(false);
        timelineGui.SetExportLock(false);
        isExporting = false;       
    }
}

bool MenuGui::RunExport(const std::string& outPath, int w, int h, int fps) {
    int oldDepth = viewportGui.shadeDepth.load();
    viewportGui.shadeDepth = 4;
    viewportGui.ApplyRenderSize(w, h);

    char cmd[1024];
    std::snprintf(cmd, sizeof cmd,
        "ffmpeg -y -loglevel error "
        "-f rawvideo -pix_fmt rgb24 -s %dx%d -framerate %d -i - "
        "-vf \"pad=ceil(iw/2)*2:ceil(ih/2)*2\" "
        "-c:v libx264 -pix_fmt yuv420p \"%s\"",
        w, h, fps, outPath.c_str());

    FILE* ff = _popen(cmd, "wb");
    if (!ff) { std::cerr << "Could not start ffmpeg\n"; return false; }

    for (int frame = timelineGui.GetStartFrame(); frame <= timelineGui.GetEndFrame(); ++frame) {
        {
            std::lock_guard<std::mutex> lock(exportMutex);
            if (quitting) break;
        }
        timelineGui.SetCurrentFrame(frame);
        timelineGui.LerpTransform(objectFactory);
        viewportGui.StartRendering();
        viewportGui.WaitForRender();

        const auto& data = viewportGui.GetPixels();
        if (fwrite(data.data(), 1, data.size(), ff) != data.size()) {
            std::cerr << "Error writing frame " << frame << "\n";
            _pclose(ff);
            return false;
        }
    }
    int rc = _pclose(ff);
    if (rc != 0) std::cerr << "ffmpeg exited with code " << rc << "\n";

    viewportGui.shadeDepth = oldDepth;
    viewportGui.ApplyRenderSize(viewportGui.fixedW, viewportGui.fixedH);
    return rc == 0;
}

bool MenuGui::OpenFile(const char* filter)
{
    OPENFILENAME ofn;
    char fileName[512]{};
    //const char filters[] =
    //    "All Files\0*.*\0"
    //    "OBJ Files\0*.obj\0"
    //    "\0";

    ZeroMemory(&ofn, sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = fileName;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 512;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetOpenFileName(&ofn)) {
        path = fileName;
		std::cout << "Selected file: " << path << std::endl;
        return true;
    }
	return false;
}

bool MenuGui::SaveFile(const char* filter)
{
    OPENFILENAME ofn{};
    char fileName[512]{};
    //const char filters[] =
    //    "All Files\0*.*\0"
    //    "OBJ Files\0*.obj\0"
    //    "\0";

    ZeroMemory(&ofn, sizeof(OPENFILENAME));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = fileName;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 512;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 2;
    ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = "mp4";

    if (GetSaveFileName(&ofn)) {
        path = fileName;
        std::cout << "Selected Save File: " << path << std::endl;
        return true;
    }
    return false;
}

void MenuGui::DrawExportPopup() {
    if (openExportPopup) {
        ImGui::OpenPopup("Export Animation");
        openExportPopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Export Animation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        struct Preset { const char* name; int w, h; };
        static const Preset presets[] = {
            { "640 x 360",   640,  360 },
            { "1280 x 720 (720p)",  1280, 720 },
            { "1920 x 1080 (1080p)", 1920, 1080 },
            { "2560 x 1440 (1440p)", 2560, 1440 },
            { "3840 x 2160 (4K)",   3840, 2160 },
            { "Custom", 0, 0 },
        };
        static int presetIdx = 2;
        static int customW = 1920, customH = 1080;
        static int fps = 24;

        if (ImGui::BeginCombo("Resolution", presets[presetIdx].name)) {
            for (int i = 0; i < IM_ARRAYSIZE(presets); ++i)
                if (ImGui::Selectable(presets[i].name, presetIdx == i))
                    presetIdx = i;
            ImGui::EndCombo();
        }

        bool custom = presets[presetIdx].w == 0;
        if (custom) {
            ImGui::InputInt("Width", &customW, 2, 100);
            ImGui::InputInt("Height", &customH, 2, 100);
        }
        ImGui::InputInt("FPS", &fps);

        int w = custom ? customW : presets[presetIdx].w;
        int h = custom ? customH : presets[presetIdx].h;
        w = std::clamp(w, 16, 8192) & ~1;
        h = std::clamp(h, 16, 8192) & ~1;
        fps = std::clamp(fps, 1, 120);

        ImGui::Text("Output: %d x %d @ %d fps", w, h, fps);
        ImGui::Separator();

        if (ImGui::Button("Export...", ImVec2(120, 0))) {
            if (SaveFile("MP4 Files\0*.mp4\0All Files\0*.*\0\0")) {
                {
                    std::lock_guard<std::mutex> lock(exportMutex);
                    exportPath = path;
                    exportW = w; exportH = h; exportFps = fps;
                    exportRequested = true;
                    isExporting = true;
                    viewportGui.SetExportLock(true);
                }
                exportCv.notify_one();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

bool MenuGui::CheckFfmpeg() {
    return std::system("ffmpeg -version >NUL 2>&1") == 0;
}

void MenuGui::DrawFfmpegPopup() {
    if (openFfmpegPopup) {
        ImGui::OpenPopup("FFmpeg Required");
        openFfmpegPopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("FFmpeg Required", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        static const char* kCmd = "winget install -e --id Gyan.FFmpeg";

        ImGui::TextWrapped("MP4 export needs FFmpeg, which was not found on your PATH.");
        ImGui::Spacing();
        ImGui::Text("Install it by running this in a terminal:");

        ImGui::SetNextItemWidth(360);
        char buf[64];
        strncpy_s(buf, kCmd, _TRUNCATE);
        ImGui::InputText("##cmd", buf, sizeof buf, ImGuiInputTextFlags_ReadOnly);

        ImGui::TextWrapped("Then restart this app.");
        ImGui::Spacing();

        if (ImGui::Button("Copy command")) ImGui::SetClipboardText(kCmd);
        ImGui::SameLine();
        if (ImGui::Button("OK", ImVec2(80, 0))) ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}