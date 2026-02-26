#include "Gui/ViewportGui.h"



ViewportGui::ViewportGui() : viewportSize(0, 0), lastViewportSize(0,0), viewport(1, 1, Color(0.2f, 0.2f, 0.2f, false)), camera(Vec3(0.0f, 0.0f, 10.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f), 45), raycast(camera.GetEye()), pixels(1 * RGB_STRIDE, 0)
{
	renderThreads.clear();
	viewportTexture.SetTexImage(GL_RGB, 1, 1, GL_RGB, nullptr);
	objectFactory.CreateFactory<ShapeFactory>();
	objectFactory.CreateFactory<LightFactory>();
	objectFactory.AddMaterial(Material(Color(0.0f, 1.0f, 0.0f, false), Color(1.0f, 1.0f, 1.0f, false), Vec3(0.2f, 0.6f, 0.2f), 10));
	std::vector<std::string> sphere = {"", "0.0f", "0.0f", "-8.0f", "4.0", "0"};
	objectFactory.GetFactoryMap()[typeid(ShapeFactory)].get()->CreateObject(string("sphere"), sphere);
	numThreads = std::thread::hardware_concurrency() - 2;
	renderThreads.reserve(numThreads);
	for (int i = 0; i < numThreads; i++) {
		renderWorkers.emplace_back(&ViewportGui::WorkerRenderer, this);
	}
}

ViewportGui::~ViewportGui()
{
	isRendering = false;
	for (int i = 0; i < renderWorkers.size(); i++) {
		if (renderWorkers[i].joinable()) {
			renderWorkers[i].join();
		}
	}
}

void ViewportGui::WorkerRenderer()
{
	Raycast lRaycast(camera.GetEye());
	while (isRendering) {
		unique_lock lock(mtx); // lock mutex to check hasWork and rowsRendered
		cv.wait(lock, [this] { return hasWork || !isRendering; }); // wait until there is work to do or rendering is finished
		lock.unlock(); // unlock mutex to allow other threads to check hasWork and rowsRendered

		/*int y = 0;
		while ((y = rowsRendered++) < viewport.GetHeight()) {
			for (int x = 0; x < viewport.GetWidth(); x++) {
				pair<Vec3, bool> intersectedPoint(Vec3(numeric_limits<float>::infinity(), numeric_limits<float>::infinity(), numeric_limits<float>::infinity()), false);
				Color color = lRaycast.TraceRay(viewport.GetWindowLocation(x, y), viewport.bkgcolor, objectFactory, intersectedPoint);
				Color pixelColor = Color(color.GetVec(), true);
				int pixelIndex = (y * viewport.GetWidth() + x) * RGB_STRIDE;
				pixels[pixelIndex] = static_cast<unsigned char>(pixelColor.r);
				pixels[pixelIndex + 1] = static_cast<unsigned char>(pixelColor.g);
				pixels[pixelIndex + 2] = static_cast<unsigned char>(pixelColor.b);
			}
		}*/
		while (true) {
			int tileIndex = tilesRendered++;
			if (tileIndex >= blockWidth * blockHeight) {
				break;
			}

			int tx = tileIndex % blockWidth;
			int ty = tileIndex / blockWidth;

			int startX = tx * BLOCK_SIZE;
			int startY = ty * BLOCK_SIZE;

			int endX = min(startX + BLOCK_SIZE, viewport.GetWidth());
			int endY = min(startY + BLOCK_SIZE, viewport.GetHeight());

			for (int y = startY; y < endY; y++) {
				for (int x = startX; x < endX; x++) {
					pair<Vec3, bool> intersectedPoint(Vec3(numeric_limits<float>::infinity(), numeric_limits<float>::infinity(), numeric_limits<float>::infinity()), false);
					Color color = lRaycast.TraceRay(viewport.GetWindowLocation(x, y), viewport.bkgcolor, objectFactory, intersectedPoint);
					Color pixelColor = Color(color.GetVec(), true);
					int pixelIndex = (y * viewport.GetWidth() + x) * RGB_STRIDE;
					pixels[pixelIndex] = static_cast<unsigned char>(pixelColor.r);
					pixels[pixelIndex + 1] = static_cast<unsigned char>(pixelColor.g);
					pixels[pixelIndex + 2] = static_cast<unsigned char>(pixelColor.b);
				}
			}

			blocksFinished++;
		}
	}
	//cout << blockSize.x << " " << blockSize.y << endl;
	//Raycast lRaycast(camera.GetEye());
	//for (int i = 0; i < blockSize.y; i++) {
	//	for (int j = 0; j < blockSize.x; j++) {
	//		//float alphaDepthCue = 1.0f;
	//		pair<Vec3, bool> intersectedPoint(Vec3(numeric_limits<float>::infinity(), numeric_limits<float>::infinity(), numeric_limits<float>::infinity()), false);
	//		Color color = lRaycast.TraceRay(viewport.GetWindowLocation(startIndex.x + j, startIndex.y + i), viewport.bkgcolor, objectFactory, intersectedPoint);

	//		Color pixelColor = Color(color.GetVec(), true);
	//		// CHANGE WIDTH AND HEIGHT TO ACCOUNT FOR BLOCK SIZE
	//		int pixelIndex = ((startIndex.y + i) * viewport.GetWidth() + (startIndex.x + j)) * RGB_STRIDE;
	//		//cout << "pixel index: " << pixelIndex << " width: " << viewport.GetWidth() << endl;
	//		pixels[pixelIndex] = static_cast<unsigned char>(pixelColor.r);
	//		pixels[pixelIndex + 1] = static_cast<unsigned char>(pixelColor.g);
	//		pixels[pixelIndex + 2] = static_cast<unsigned char>(pixelColor.b);
	//	}
	//}
}

void ViewportGui::PostUpdate()
{
	/*if (!renderThreads.empty()) {
		for (int i = 0; i < renderThreads.size(); i++) {
			if (renderThreads[i].wait_for(chrono::seconds(0)) == future_status::ready) {
				renderThreads[i].get();
				renderThreads.erase(renderThreads.begin() + i);
				i--;
				blocksFinished++;
			}
		}
		return;
	}*/

	int newWidth = (int)viewportSize.x;
	int newHeight = (int)viewportSize.y;

	// CAN MAKE SO IF VIEWPORT SIZE CHANGES, THEN ONLY UPDATE THE PIXELS IN THE VIEWPORT
	if ((newWidth != 0 && newHeight != 0) && (viewport.GetWidth() != newWidth || viewport.GetHeight() != newHeight)) {
		cout << "in post " << endl;
		lastViewportSize = viewportSize;
		viewport.SetWidthHeight(newWidth, newHeight);
		viewport.CalcWindowCorners(camera);
		int size = newWidth * newHeight;
		pixels.resize(size * RGB_STRIDE);

		blocksCreated = 0;
		blockWidth = ceil(newWidth / static_cast<float>(BLOCK_SIZE));
		blockHeight = ceil(newHeight / static_cast<float>(BLOCK_SIZE));
		
		// reset rows rendered to 0 so that worker threads can start rendering from the top of the viewport
		rowsRendered = 0;
		tilesRendered = 0;
		blocksFinished = 0;
		{
			lock_guard<mutex> lock(mtx); // lock mutex to update hasWork and rowsRendered
			hasWork = true;
		}
		cv.notify_all();

		// allocate memory for texture
		viewportTexture.SetTexImage(GL_RGB8, newWidth, newHeight, GL_RGB, nullptr);
	}

	if (blocksFinished == blockWidth * blockHeight) {
		{
			lock_guard<mutex> lock(mtx); // lock mutex to update hasWork and rowsRendered
			hasWork = false;
		}
	}

	//int i = 0;
	//int numThreadMin = numThreads;
	//// if there are more threads than blocks, then only create threads for the number of blocks
	//if (numThreads > blockWidth * blockHeight) {
	//	numThreadMin = blockWidth;
	//}
	//// if there are more blocks than threads, then only create threads for the number of threads
	//numThreadMin = min(numThreadMin, blockWidth * blockHeight - blocksCreated);
	////cout << "min: " << numThreadMin << " thread size: " << renderThreads.size() << endl;
	//while (renderThreads.size() < numThreadMin && i < numThreadMin) {
	//	//cout << "creating thread for block: " << blocksCreated % blockWidth * BLOCK_SIZE << " " << blocksCreated / blockWidth * BLOCK_SIZE << endl;
	//	int startIndexX = blocksCreated % blockWidth * BLOCK_SIZE;
	//	int startIndexY = blocksCreated / blockWidth * BLOCK_SIZE;
	//	ImVec2 blockSize = ImVec2(min(BLOCK_SIZE, viewport.GetWidth() - startIndexX), min(BLOCK_SIZE, viewport.GetHeight() - startIndexY));
	//	future<void> f = async(launch::async, &ViewportGui::WorkerRenderer, this, ImVec2(startIndexX, startIndexY), blockSize);
	//	renderThreads.emplace_back(std::move(f));
	//	i++;
	//	blocksCreated++;
	//}
	viewportTexture.Bind();
	// non pixel size aligned data, so set unpack alignment to 1
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// update texture with new pixel data
	viewportTexture.SetTexSubImage(viewport.GetWidth(), viewport.GetHeight(), GL_RGB, pixels.data());
}

void ViewportGui::Update()
{
	ImGui::Begin("Viewport");
	viewportSize = ImGui::GetContentRegionAvail();
	//cout << "viewport size: " << viewportSize.x << " " << viewportSize.y << endl;
	int newWidth = max(1, (int)viewportSize.x);
	int newHeight = max(1, (int)viewportSize.y);

	viewportSize = ImVec2(newWidth, newHeight);

	ImGui::Image((void*)(intptr_t)viewportTexture.tex, viewportSize);
	ImGui::End();
}
