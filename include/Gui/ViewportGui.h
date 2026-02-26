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
		const int BLOCK_SIZE = 64;
		const int RGB_STRIDE = 3;
		int blocksCreated = 0;
		int blockWidth = 0;
		int blockHeight = 0;
		int numThreads = 0;
		queue<int> threadFinishIndex;
		vector<future<void>> renderThreads;
		void WorkerRenderer(ImVec2 startIndex, ImVec2 blockSize);



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
		void PostUpdate() override;
		void Update() override;
	};

#endif // !VIEWPORTGUI_H