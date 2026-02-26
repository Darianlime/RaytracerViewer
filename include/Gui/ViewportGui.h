#ifndef VIEWPORTGUI_H
#define VIEWPORTGUI_H

#include "UseImGui.h"
#include "Texture.h"
#include "raycast.h"
#include "viewport.h"
#include "camera.h"
#include <thread>
#include <future>
#include <queue>
#include <atomic>
#include <chrono>
#include <algorithm>

using namespace Raytracer;

class ViewportGui : public UseImGui {
	private:
		bool start = true;

		const int BLOCK_SIZE = 64;
		const int RGB_STRIDE = 3;
		int blocksCreated = 0;
		atomic<int> blocksFinished = 0;
		int blockWidth = 0;
		int blockHeight = 0;
		int numThreads = 0;
		atomic<bool> isRendering = true;
		atomic<int> tilesRendered = 0;
		atomic<int> rowsRendered = 0;
		condition_variable cv;
		mutex mtx;
		bool hasWork = false;
		vector<future<void>> renderThreads;
		vector<thread> renderWorkers;
		void WorkerRenderer();



		Texture viewportTexture;
		ImVec2 viewportSize;
		ImVec2 lastViewportSize;
		Raycast raycast;
		Viewport viewport;
		ObjectFactory objectFactory;
		Camera camera;
		std::vector<unsigned char> pixels;

	public:
		ViewportGui();
		~ViewportGui();
		void PostUpdate() override;
		void Update() override;
	};

#endif // !VIEWPORTGUI_H