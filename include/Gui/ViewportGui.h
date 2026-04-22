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

		const uint8_t BLOCK_SIZE = 64;
		const uint8_t RGB_STRIDE = 3;
		std::atomic<int> blocksFinished = 0;
		int blockWidth = 0;
		int blockHeight = 0;
		int numThreads = 0;
		std::atomic<bool> isRendering = true;
		std::atomic<int> tilesRendered = 0;
		std::atomic<int> rowsRendered = 0;
		bool hasWork = false;
		std::condition_variable cv;
		std::mutex mtx;
		ThreadSafeQueue<Tile> blockQueue;
		vector<std::thread> renderWorkers;
		void WorkerRenderer(ObjectFactory& objectFactory);

		void StartRendering();
		void OverrideRendering();
		void StopRendering();

		ObjectFactory& objectFactory;
		ScreenTexture screenTexture;
		ImVec2 viewportSize;
		ImVec2 lastViewportSize;
		Viewport viewport;
		std::vector<unsigned char> pixels;

	public:
		ViewportGui(ObjectFactory& objectFactory);
		~ViewportGui();
		void PostUpdate();
		void PropertiesUpdate(int isUpdatingProperties, int index);
		void Update() override;
};

#endif // !VIEWPORTGUI_H