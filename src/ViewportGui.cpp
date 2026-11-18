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
	{
		std::lock_guard<std::mutex> lock(mtx);
		isRendering = false;
	}

	cv.notify_all();

	for (auto& worker : renderWorkers)
	{
		if (worker.joinable())
			worker.join();
	}
}

inline static void WriteRgbByte(unsigned char* dst, const Vec3& rgb) {
	dst[0] = static_cast<unsigned char>(std::clamp(std::round(rgb.x * 255.0f), 0.0f, 255.0f));
	dst[1] = static_cast<unsigned char>(std::clamp(std::round(rgb.y * 255.0f), 0.0f, 255.0f));
	dst[2] = static_cast<unsigned char>(std::clamp(std::round(rgb.z * 255.0f), 0.0f, 255.0f));
}

void ViewportGui::HandleCameraControls() {
	if (exportLocked) return;
	if (!ImGui::IsWindowHovered() || !ImGui::IsMouseDown(ImGuiMouseButton_Right)) return;

	ImGuiIO& io = ImGui::GetIO();
	Raytracer::Camera& cam = objectFactory.GetCameraIndex(0);

	Vec3 d = cam.viewdir.Normalize();
	float yaw = std::atan2(d.x, -d.z);
	float pitch = std::asin(std::clamp(d.y, -1.0f, 1.0f));
	bool moved = false;

	// Mouse look
	if (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f) {
		const float sens = 0.003f;                       // radians per pixel
		yaw += io.MouseDelta.x * sens;
		pitch -= io.MouseDelta.y * sens;
		pitch = std::clamp(pitch, -1.55f, 1.55f);        // just under 89 degrees
		d = Vec3(std::sin(yaw) * std::cos(pitch), std::sin(pitch), -std::cos(yaw) * std::cos(pitch));
		moved = true;
	}

	// WASD + Q/E
	Vec3 right = Vec3(-d.z, 0.0f, d.x).Normalize();      // horizontal right vector
	float speed = (io.KeyShift ? 10.0f : 3.0f) * io.DeltaTime;
	Vec3 move(0, 0, 0);
	if (ImGui::IsKeyDown(ImGuiKey_W)) move = move + d;
	if (ImGui::IsKeyDown(ImGuiKey_S)) move = move - d;
	if (ImGui::IsKeyDown(ImGuiKey_D)) move = move + right;
	if (ImGui::IsKeyDown(ImGuiKey_A)) move = move - right;
	if (ImGui::IsKeyDown(ImGuiKey_E)) move = move + Vec3(0, 1, 0);
	if (ImGui::IsKeyDown(ImGuiKey_Q)) move = move - Vec3(0, 1, 0);
	if (move.x != 0 || move.y != 0 || move.z != 0) {
		cam.eye = cam.eye + move.Normalize() * speed;
		moved = true;
	}

	if (moved) {
		cam.viewdir = d;
		cam.CalcCoords();
		updateGUIState.updateType = UpdateType::PROPERTIES_CAMERA;
	}
}

void ViewportGui::ApplyCamera() {
	std::lock_guard<std::mutex> lock(mtx);
	const Raytracer::Camera& cam = objectFactory.GetCameras()[0];
	viewport.CalcWindowCorners(cam);
	frameEye = cam.GetEye();
}

void ViewportGui::WorkerRenderer(ObjectFactory& objectFactory)
{
	Raycast lRaycast(objectFactory.GetCameras()[0].GetEye(), objectFactory, shadeDepth);
	uint64_t lastSeenGen = 0;
	while (isRendering) {
		std::unique_lock lock(mtx); // lock mutex to check hasWork and rowsRendered
		cv.wait(lock, [this, lastSeenGen] {
			return renderGeneration.load() != lastSeenGen || !isRendering;
			});
		if (!isRendering) break;

		lastSeenGen = renderGeneration.load();
		lRaycast.SetEye(frameEye);
		lRaycast.SetShadeDepth(shadeDepth.load());
		activeWorkers.fetch_add(1);
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
			if (blocksFinished.fetch_add(1) + 1 >= blockWidth * blockHeight) {
				//auto renderEndTime = Clock::now();

				//double renderMs =
				//	std::chrono::duration<double, std::milli>(
				//		renderEndTime - renderStartTime
				//	).count();

				//std::cout << "Full render: "
				//	<< renderMs
				//	<< " ms\n";
				std::lock_guard<std::mutex> g(mtx);
				doneCv.notify_all();
			}
		}
		activeWorkers.fetch_sub(1);
		{ std::lock_guard<std::mutex> g(mtx); }
		doneCv.notify_all();
	}
}

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
	renderStartTime = Clock::now();
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
			return IsRenderingDone() || !isRendering;
		});
	}

	// clear leftover tiles
	Tile tile;
	while (blockQueue.try_pop(tile)) {}
} 

void ViewportGui::WaitForRender()
{
	std::unique_lock<std::mutex> lock(mtx);
	doneCv.wait(lock, [this] {
		return IsRenderingDone() || !isRendering;
	});
}

void ViewportGui::StopRendering() {
	{
		std::lock_guard<std::mutex> lock(mtx);
		hasWork = false;
	}
	cv.notify_all();
}

void ViewportGui::UploadFrameIfReady() {
	if (exportLocked) return;
	if (IsRenderingDone() && !uploadedThisFrame) {
		if (textureNeedsRealloc.exchange(false)) {
			screenTexture.SetTexImage(GL_RGB8, viewport.GetWidth(), viewport.GetHeight(), GL_RGB, nullptr);
		}
		screenTexture.Bind();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, viewport.GetWidth());
		screenTexture.SetTexSubImage(0, 0, viewport.GetWidth(), viewport.GetHeight(), GL_RGB, pixels.data());
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		uploadedThisFrame = true;
	}
}

bool ViewportGui::IsRenderingDone() const {
	return blocksFinished.load() >= blockWidth * blockHeight && activeWorkers.load() == 0;
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
	if (exportLocked) return;

	if (updateGUIState.updateType == (int)UpdateType::PROPERTIES_CAMERA) {
		cameraDirty = true;
	}
	if (cameraDirty && IsRenderingDone()) {
		cameraDirty = false;
		UploadFrameIfReady();   // show the finished frame before StartRendering clears the flag
		ApplyCamera();
		StartRendering();
	}

	if (updateGUIState.updateType > 0 && updateGUIState.updateType != (int)UpdateType::PROPERTIES_CAMERA) {
		//std::cout << "update type: " << updateGUIState.updateType << std::endl;
	

		if (updateGUIState.updateType == (int)UpdateType::FRAME_CHANGE) {
			//auto start = std::chrono::high_resolution_clock::now();
			//objectFactory.GetFactory<ModelFactory>().GetObjects()[updateGUIState.selectedObjectIndex]->UpdateTransformation();
			UploadFrameIfReady();
			StartRendering(); // signal worker threads to start rendering the new frame
			return;
			//auto end = std::chrono::high_resolution_clock::now();

			//std::cout << "UpdateTransformation: "
			//	<< std::chrono::duration<double, std::milli>(
			//		end - start).count()
			//	<< " ms\n";
		}
		else {
			OverrideRendering(); // stop workers from rendering until we update the viewport and pixel buffer for the new frame;
			switch (updateGUIState.updateType) {
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
		}
		StartRendering(); // signal worker threads to start rendering the new frame
	}
}


void ViewportGui::ApplyRenderSize(int w, int h) {
	OverrideRendering();

	viewport.SetWidthHeight(w, h);
	ApplyCamera();
	pixels.resize(size_t(w) * h * RGB_STRIDE);

	blockWidth = (w + BLOCK_SIZE - 1) / BLOCK_SIZE;
	blockHeight = (h + BLOCK_SIZE - 1) / BLOCK_SIZE;

	textureNeedsRealloc = true;
}

void ViewportGui::PostUpdate()
{
	if (!exportLocked) {
		int tw, th;
		if (true) { tw = fixedW; th = fixedH; }
		if (tw != viewport.GetWidth() || th != viewport.GetHeight()) {
			ApplyRenderSize(tw, th);
			StartRendering();
		}
	}
	UploadFrameIfReady();
}

void ViewportGui::Update()
{
	ImGui::Begin("Viewport");
	ImVec2 avail = ImGui::GetContentRegionAvail();
	viewportSize = ImVec2(std::max(1.0f, avail.x), std::max(1.0f, avail.y));

	float aspect = float(viewport.GetWidth()) / std::max(1, viewport.GetHeight());
	ImVec2 size = viewportSize;
	if (size.x / size.y > aspect) {
		size.x = size.y * aspect;
	} else {
		size.y = size.x / aspect;
	}

	ImGui::SetCursorPos(ImVec2(
		ImGui::GetCursorPosX() + (viewportSize.x - size.x) * 0.5f,
		ImGui::GetCursorPosY() + (viewportSize.y - size.y) * 0.5f)
	);

	ImGui::Image((void*)(intptr_t)screenTexture.tex, size);
	HandleCameraControls();
	ImGui::End();

	PropertiesUpdate();
}
