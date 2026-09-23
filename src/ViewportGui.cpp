#include "Gui/ViewportGui.h"

using std::min;
using std::max;
using std::numeric_limits;

ViewportGui::ViewportGui(ObjectFactory &objectFactory, UpdateGUIState& updateState) : 
	viewportSize(0, 0), 
	lastViewportSize(0,0), 
	viewport(1, 1, Color(0.7f, 0.7f, 0.7f, false), 1.0f), 
	pixels(1 * RGB_STRIDE, 0), 
	objectFactory(objectFactory),
	updateGUIState(updateState)
{
	screenTexture.SetTexImage(GL_RGB, 1, 1, GL_RGB, nullptr);
	numThreads = std::thread::hardware_concurrency()-2;

	std::cout << "Using " << numThreads << " threads for rendering." << std::endl;
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

inline static void WriteRgbByte(unsigned char* dst, const Vec3& rgb) {
	dst[0] = static_cast<unsigned char>(std::clamp(std::round(rgb.x * 255.0f), 0.0f, 255.0f));
	dst[1] = static_cast<unsigned char>(std::clamp(std::round(rgb.y * 255.0f), 0.0f, 255.0f));
	dst[2] = static_cast<unsigned char>(std::clamp(std::round(rgb.z * 255.0f), 0.0f, 255.0f));
}

void ViewportGui::WorkerRenderer(ObjectFactory& objectFactory)
{
	Raycast lRaycast(objectFactory.GetCameras()[0].GetEye(), objectFactory, 2);
	uint64_t lastSeenGen = 0;
	while (isRendering) {
		std::unique_lock lock(mtx); // lock mutex to check hasWork and rowsRendered
		cv.wait(lock, [this, lastSeenGen] {
			return renderGeneration.load() != lastSeenGen || !isRendering;
			});
		if (!isRendering) break;

		lastSeenGen = renderGeneration.load();
		lRaycast.SetEye(objectFactory.GetCameras()[0].GetEye());
		//std::cout << lRaycast.GetEye().x << " " << lRaycast.GetEye().y << " " << lRaycast.GetEye().z << std::endl;
		lock.unlock(); // unlock mutex to allow other threads to check hasWork and rowsRendered

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
					Color color = lRaycast.TraceRay(viewport.GetWindowLocation(x, y), viewport.bkgcolor, intersectedPoint);
					//Color pixelColor = Color(color.GetVec(), true);
					int pixelIndex = (y * vw + x) * RGB_STRIDE;
					WriteRgbByte(&pixels[pixelIndex], color.GetVec());

					//pixels[pixelIndex] = static_cast<unsigned char>(pixelColor.r);
					//pixels[pixelIndex + 1] = static_cast<unsigned char>(pixelColor.g);
					//pixels[pixelIndex + 2] = static_cast<unsigned char>(pixelColor.b);
				}
			}

			//blockQueue.push(Tile{ startX, startY, endX - startX, endY - startY });
			if (blocksFinished.fetch_add(1) + 1 == blockWidth * blockHeight) {
				std::lock_guard<std::mutex> lock(mtx);
				doneCv.notify_all();
			}
		}
	}
}

std::chrono::high_resolution_clock::time_point renderStartTime;

void ViewportGui::StartRendering()
{
	// reset rows/tiles/blocks so worker threads will render the new frame
	{
		std::lock_guard<std::mutex> lock(mtx); // lock mutex to update hasWork and rowsRendered
		tilesRendered = 0;
		blocksFinished = 0;
		renderGeneration.fetch_add(1);
		hasWork = true;
	}
	renderStartTime = std::chrono::high_resolution_clock::now();
	uploadedThisFrame = false; // reset so we can upload the new frame once it's done
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
		std::unique_lock<std::mutex> lock(mtx);
		doneCv.wait(lock, [this, oldBlockCount] {
			return blocksFinished.load() >= oldBlockCount || !isRendering;
		});
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

void ViewportGui::MenuUpdate(int isUpdatingProperties, std::string path)
{
	if (isUpdatingProperties > 0) {
		OverrideRendering(); // stop workers from rendering until we update the viewport and pixel buffer for the new frame
		switch (isUpdatingProperties) {
		case (int)UpdateType::PROPERTIES_CAMERA:
			
			break;
		default:
			break;
		}
		StartRendering(); // signal worker threads to start rendering the new frame
	}
}

void ViewportGui::PropertiesUpdate()
{
	if (updateGUIState.updateType > 0) {
		//std::cout << "update type: " << updateGUIState.updateType << std::endl;

		OverrideRendering(); // stop workers from rendering until we update the viewport and pixel buffer for the new frame
		switch (updateGUIState.updateType) {
			case (int)UpdateType::PROPERTIES_CAMERA:
				viewport.CalcWindowCorners(objectFactory.GetCameras()[0]);
				break;
			case (int)UpdateType::DELETING_MATERIAL:
				objectFactory.RemoveMaterial(updateGUIState.deletingIndex);
				break;
			case (int)UpdateType::DELETING_LIGHT: {
				LightFactory& lights = objectFactory.GetFactory<LightFactory>();
				lights.RemoveLight(updateGUIState.deletingIndex);
			}
			break;
			case (int)UpdateType::DELETING_MODEL: {
				ModelFactory& models = objectFactory.GetFactory<ModelFactory>();
				models.RemoveModel(updateGUIState.deletingIndex);
			}
			break;
			default:
				break;
		}
		StartRendering(); // signal worker threads to start rendering the new frame
	}
}

void ViewportGui::PostUpdate()
{
	int newWidth = (int)viewportSize.x;
	int newHeight = (int)viewportSize.y;

	// CAN MAKE SO IF VIEWPORT SIZE CHANGES, THEN ONLY UPDATE THE PIXELS IN THE VIEWPORT
	if ((newWidth != 0 && newHeight != 0) && (viewport.GetWidth() != newWidth || viewport.GetHeight() != newHeight)) {
		std::cout << "in new height and width " << std::endl;
		std::cout << newWidth << " " << newHeight << std::endl;

		OverrideRendering(); // stop workers from rendering until we update the viewport and pixel buffer for the new frame

		lastViewportSize = viewportSize;
		viewport.SetWidthHeight(newWidth, newHeight);
		viewport.CalcWindowCorners(objectFactory.GetCameras()[0]);
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

	// Drain the queue every frame regardless (cheap — just pops,
	// doesn't touch the texture), so it doesn't build up.
	//Tile tile{};
	//while (blockQueue.try_pop(tile)) {
	//	// intentionally discarded — we only upload once fully done
	//}

	// Only touch the GPU texture once the whole frame is ready.
	if (blocksFinished.load() == blockWidth * blockHeight && !uploadedThisFrame) {
		StopRendering();

		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout << "Render took: "
			<< std::chrono::duration<double, std::milli>(t1 - renderStartTime).count()
			<< "ms\n";

		screenTexture.Bind();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, viewport.GetWidth());
		screenTexture.SetTexSubImage(0, 0, viewport.GetWidth(), viewport.GetHeight(),
			GL_RGB, pixels.data());
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

		uploadedThisFrame = true; // guard so you don't re-upload every frame after finishing
	}
}

void ViewportGui::Update()
{
	PropertiesUpdate();

	ImGui::Begin("Viewport");
	viewportSize = ImGui::GetContentRegionAvail();
	//cout << "viewport size: " << viewportSize.x << " " << viewportSize.y << endl;
	int newWidth = max(1, (int)viewportSize.x);
	int newHeight = max(1, (int)viewportSize.y);

	viewportSize = ImVec2(newWidth, newHeight);

	ImGui::Image((void*)(intptr_t)screenTexture.tex, viewportSize);
	ImGui::End();
}
