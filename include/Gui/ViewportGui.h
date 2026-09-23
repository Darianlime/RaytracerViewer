#ifndef VIEWPORTGUI_H
#define VIEWPORTGUI_H

#include "UseImGui.h"
#include "ScreenTexture.h"
#include "raycast.h"
#include "viewport.h"
#include "camera.h"
#include "Threads/ThreadSafeQueue.h"`

#include <thread>
#include <future>
#include <atomic>
#include <chrono>
#include <algorithm>
#include "GUIState.h"

using namespace Raytracer;

struct Tile {
	int x;
	int y;
	int width;
	int height;
};

class ViewportGui : public UseImGui {
	private:
		bool start = true;

		const uint8_t BLOCK_SIZE = 32;
		const uint8_t RGB_STRIDE = 3;
		std::atomic<int> blocksFinished = 0;
		int blockWidth = 0;
		int blockHeight = 0;
		int numThreads = 0;
		std::atomic<bool> isRendering = true;
		std::atomic<int> tilesRendered = 0;
		std::atomic<int> rowsRendered = 0;
		bool hasWork = false;
		bool uploadedThisFrame = false;

		std::condition_variable cv;
		std::condition_variable doneCv;
		std::atomic<uint64_t> renderGeneration{ 0 };
		std::mutex mtx;
		ThreadSafeQueue<Tile> blockQueue;
		vector<std::thread> renderWorkers;
		void WorkerRenderer(ObjectFactory& objectFactory);

		void StartRendering();
		void OverrideRendering();
		void StopRendering();

		ObjectFactory& objectFactory;
		UpdateGUIState& updateGUIState;
		ScreenTexture screenTexture;
		ImVec2 viewportSize;
		ImVec2 lastViewportSize;
		Viewport viewport;
		std::vector<unsigned char> pixels;

	public:
		ViewportGui(ObjectFactory& objectFactory, UpdateGUIState& updateState);
		~ViewportGui();
		void PostUpdate();
		void PropertiesUpdate();
		void MenuUpdate(int isUpdatingProperties, std::string path);
		void Update() override;
};

#endif // !VIEWPORTGUI_H