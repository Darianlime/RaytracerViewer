#include "Gui/ViewportGui.h"

using std::min;
using std::max;
using std::numeric_limits;

ViewportGui::ViewportGui(ObjectFactory &objectFactory) : 
	viewportSize(0, 0), 
	lastViewportSize(0,0), 
	viewport(1, 1, Color(0.2f, 0.2f, 0.2f, false)), 
	camera(Vec3(0.0f, 0.0f, 10.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f), 45), raycast(camera.GetEye()), 
	pixels(1 * RGB_STRIDE, 0), objectFactory(objectFactory)
{
	screenTexture.SetTexImage(GL_RGB, 1, 1, GL_RGB, nullptr);
	numThreads = std::thread::hardware_concurrency() - 2;
	for (int i = 0; i < numThreads; i++) {
		renderWorkers.emplace_back(&ViewportGui::WorkerRenderer, this, std::ref(this->objectFactory));
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

void ViewportGui::WorkerRenderer(ObjectFactory& objectFactory)
{
	Raycast lRaycast(camera.GetEye());
	while (isRendering) {
		std::unique_lock lock(mtx); // lock mutex to check hasWork and rowsRendered
		cv.wait(lock, [this] { return hasWork || !isRendering; }); // wait until there is work to do or rendering is finished
		lock.unlock(); // unlock mutex to allow other threads to check hasWork and rowsRendered

		std::cout << objectFactory.GetFactory<MeshFactory>().GetObjects().size() << std::endl;
		int i = 0;
		for (auto& mesh : objectFactory.GetFactory<MeshFactory>().GetObjects()) {
			std::cout << mesh->GetName() << i << std::endl;
			i++;
		}

		while (true) {
			int tileIndex = tilesRendered.fetch_add(1);
			if (tileIndex >= blockWidth * blockHeight) {
				break;
			}

			int tx = tileIndex % blockWidth;
			int ty = tileIndex / blockWidth;

			int startX = tx * BLOCK_SIZE;
			int startY = ty * BLOCK_SIZE;

			int vw = viewport.GetWidth();
			int vh = viewport.GetHeight();

			int endX = std::min(startX + BLOCK_SIZE, vw);
			int endY = std::min(startY + BLOCK_SIZE, vh);

			pair<Vec3, bool> intersectedPoint(Vec3(numeric_limits<float>::infinity(), numeric_limits<float>::infinity(), numeric_limits<float>::infinity()), false);
			for (int y = startY; y < endY; ++y) {
				for (int x = startX; x < endX; ++x) {
					intersectedPoint.first = Vec3(numeric_limits<float>::infinity(), numeric_limits<float>::infinity(), numeric_limits<float>::infinity());
					intersectedPoint.second = false;
					Color color = lRaycast.TraceRay(viewport.GetWindowLocation(x, y), viewport.bkgcolor, objectFactory, intersectedPoint);
					Color pixelColor = Color(color.GetVec(), true);
					int pixelIndex = (y * vw + x) * RGB_STRIDE;
					pixels[pixelIndex] = static_cast<unsigned char>(pixelColor.r);
					pixels[pixelIndex + 1] = static_cast<unsigned char>(pixelColor.g);
					pixels[pixelIndex + 2] = static_cast<unsigned char>(pixelColor.b);
				}
			}

			blockQueue.push(Tile{ startX, startY, endX - startX, endY - startY });
			blocksFinished.fetch_add(1);
		}
	}
}

void ViewportGui::StartRendering()
{
	// reset rows/tiles/blocks so worker threads will render the new frame
	{
		std::lock_guard<std::mutex> lock(mtx); // lock mutex to update hasWork and rowsRendered
		rowsRendered = 0;
		tilesRendered = 0;
		blocksFinished = 0;
		hasWork = true;
	}
	cv.notify_all();
}

void ViewportGui::OverrideRendering()
{
	// Prevent workers from starting new work and wait for current work to finish.
	int oldBlockCount = blockWidth * blockHeight;
	{
		std::lock_guard<std::mutex> lock(mtx);
		hasWork = false;
	}
	cv.notify_all();

	// wait until all blocks from the previous frame are finished (or there was nothing)
	if (oldBlockCount > 0) {
		while (blocksFinished != oldBlockCount && isRendering) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	// clear leftover tiles
	Tile tile;
	while (blockQueue.try_pop(tile)) {}
} 

void ViewportGui::StopRendering() {
	{
		std::lock_guard<std::mutex> lock(mtx);
		hasWork = false;
	}
	cv.notify_all();
}

void ViewportGui::PostUpdate(bool isUpdatingProperties)
{
	if (isUpdatingProperties) {
		OverrideRendering(); // stop workers from rendering until we update the viewport and pixel buffer for the new frame
		StartRendering(); // signal worker threads to start rendering the new frame
	}

	int newWidth = (int)viewportSize.x;
	int newHeight = (int)viewportSize.y;

	// CAN MAKE SO IF VIEWPORT SIZE CHANGES, THEN ONLY UPDATE THE PIXELS IN THE VIEWPORT
	if ((newWidth != 0 && newHeight != 0) && (viewport.GetWidth() != newWidth || viewport.GetHeight() != newHeight)) {
		std::cout << "in new height and width " << std::endl;
		std::cout << newWidth << " " << newHeight << std::endl;

		OverrideRendering(); // stop workers from rendering until we update the viewport and pixel buffer for the new frame

		lastViewportSize = viewportSize;
		viewport.SetWidthHeight(newWidth, newHeight);
		viewport.CalcWindowCorners(camera);
		int size = newWidth * newHeight;
		pixels.resize(size * RGB_STRIDE, static_cast<unsigned char>(viewport.bkgcolor.r));

		// Recompute block layout for the new frame and reset counters
		blockWidth = static_cast<int>(ceil(newWidth / static_cast<float>(BLOCK_SIZE)));
		blockHeight = static_cast<int>(ceil(newHeight / static_cast<float>(BLOCK_SIZE)));
		
		// signal worker threads to start rendering the new frame
		StartRendering();

		// allocate memory for texture
		screenTexture.SetTexImage(GL_RGB8, newWidth, newHeight, GL_RGB, nullptr);
	}

	if (blocksFinished.load() == blockWidth * blockHeight) {
		StopRendering(); // stop workers from rendering until next frame, since all blocks are finished
	}

	screenTexture.Bind();
	// non pixel size aligned data, so set unpack alignment to 1
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, viewport.GetWidth());

	const int MAX_UPDATES_PER_FRAME = 8;
	int i = 0;
	Tile tile{};
	while (i < MAX_UPDATES_PER_FRAME && blockQueue.try_pop(tile)) {
		// update texture with new pixel data
		screenTexture.SetTexSubImage(tile.x, tile.y, tile.width, tile.height, GL_RGB, &pixels[(tile.y * viewport.GetWidth() + tile.x) * RGB_STRIDE]);
		//viewportTexture.SetTexSubImage(viewport.GetWidth(), viewport.GetHeight(), GL_RGB, pixels.data());
		i++;
	}

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

void ViewportGui::Update()
{
	ImGui::Begin("Viewport");
	viewportSize = ImGui::GetContentRegionAvail();
	//cout << "viewport size: " << viewportSize.x << " " << viewportSize.y << endl;
	int newWidth = max(1, (int)viewportSize.x);
	int newHeight = max(1, (int)viewportSize.y);

	viewportSize = ImVec2(newWidth, newHeight);

	ImGui::Image((void*)(intptr_t)screenTexture.tex, viewportSize);
	ImGui::End();
}
