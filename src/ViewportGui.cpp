#include "Gui/ViewportGui.h"



ViewportGui::ViewportGui() : viewportSize(0, 0), viewport(1, 1, Color(0.2f, 0.2f, 0.2f, false)), camera(Vec3(0.0f, 0.0f, 10.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f), 45), raycast(camera.GetEye()), pixels(1 * RGB_STRIDE, 0)
{
	viewportTexture.SetTexImage(GL_RGB, 1, 1, GL_RGB, nullptr);
	objectFactory.CreateFactory<ShapeFactory>();
	objectFactory.CreateFactory<LightFactory>();
	objectFactory.AddMaterial(Material(Color(0.0f, 1.0f, 0.0f, false), Color(1.0f, 1.0f, 1.0f, false), Vec3(0.2f, 0.6f, 0.2f), 10));
	std::vector<std::string> sphere = {"", "0.0f", "0.0f", "-8.0f", "4.0", "0"};
	objectFactory.GetFactoryMap()[typeid(ShapeFactory)].get()->CreateObject(string("sphere"), sphere);
	numThreads = std::thread::hardware_concurrency() - 2;
	renderThreads.reserve(numThreads);
}

void ViewportGui::WorkerRenderer(ImVec2 startIndex, ImVec2 blockSize)
{
	cout << "working on block: " << startIndex.x << " " << startIndex.y << endl;
	//cout << "block size: " << blockSize.x << " " << blockSize.y << endl;
	Raycast lRaycast(camera.GetEye());
	for (int i = 0; i < blockSize.y; i++) {
		for (int j = 0; j < blockSize.x; j++) {
			//float alphaDepthCue = 1.0f;
			pair<Vec3, bool> intersectedPoint(Vec3(numeric_limits<float>::infinity(), numeric_limits<float>::infinity(), numeric_limits<float>::infinity()), false);
			Color color = lRaycast.TraceRay(viewport.GetWindowLocation(startIndex.x + j, startIndex.y + i), viewport.bkgcolor, objectFactory, intersectedPoint);

			Color pixelColor = Color(color.GetVec(), true);
			int pixelIndex = ((startIndex.y + i) * viewport.GetWidth() + (startIndex.x + j)) * RGB_STRIDE;
			pixels[pixelIndex] = static_cast<unsigned char>(pixelColor.r);
			pixels[pixelIndex + 1] = static_cast<unsigned char>(pixelColor.g);
			pixels[pixelIndex + 2] = static_cast<unsigned char>(pixelColor.b);
		}
	}
}

void ViewportGui::PostUpdate()
{
	// CAN MAKE SO IF VIEWPORT SIZE CHANGES, THEN ONLY UPDATE THE PIXELS IN THE VIEWPORT
	if ((viewportSize.x != 0 && viewportSize.y != 0) && (lastViewportSize.x != viewportSize.x && lastViewportSize.y != viewportSize.y)) {
		lastViewportSize = viewportSize;
		viewport.SetWidthHeight(viewportSize.x, viewportSize.y);
		viewport.CalcWindowCorners(camera);
		int size = viewport.GetWidth() * viewport.GetHeight();
		pixels.resize(size * RGB_STRIDE);

		blocksCreated = 0;
		blockWidth = ceil(viewport.GetWidth() / static_cast<float>(BLOCK_SIZE));
		blockHeight = ceil(viewport.GetHeight() / static_cast<float>(BLOCK_SIZE));

		// allocate memory for texture
		viewportTexture.SetTexImage(GL_RGB, viewport.GetWidth(), viewport.GetHeight(), GL_RGB, nullptr);
	}

	int i = 0;
	int numThreadMin = numThreads;
	// if there are more threads than blocks, then only create threads for the number of blocks
	if (numThreads > blockWidth * blockHeight - numThreads) {
		numThreadMin = blockWidth * blockHeight;
	}
	// if there are more blocks than threads, then only create threads for the number of threads
	numThreadMin = min(numThreadMin, blockWidth * blockHeight - blocksCreated);
	//cout << "min: " << numThreadMin << " thread size: " << renderThreads.size() << endl;
	while (renderThreads.size() < numThreadMin && i < numThreadMin) {
		//cout << "creating thread for block: " << blocksCreated % blockWidth * BLOCK_SIZE << " " << blocksCreated / blockWidth * BLOCK_SIZE << endl;
		int startIndexX = blocksCreated % blockWidth * BLOCK_SIZE;
		int startIndexY = blocksCreated / blockWidth * BLOCK_SIZE;
		ImVec2 blockSize = ImVec2(min(BLOCK_SIZE, viewport.GetWidth() - startIndexX), min(BLOCK_SIZE, viewport.GetHeight() - startIndexY));
		future<void> f = async(launch::async, &ViewportGui::WorkerRenderer, this, ImVec2(startIndexX, startIndexY), blockSize);
		renderThreads.emplace_back(std::move(f));
		i++;
		blocksCreated++;
	}

	if (!renderThreads.empty()) {
		for (int i = 0; i < renderThreads.size(); i++) {
			if (renderThreads[i].wait_for(chrono::seconds(0)) == future_status::ready) {
				renderThreads[i].get();
				renderThreads.erase(renderThreads.begin() + i);
				i--;
				//cout << "blocks finished: " << blocksFinished << endl;
			}
		}
	}

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
	ImGui::Image((void*)(intptr_t)viewportTexture.tex, viewportSize);
	ImGui::End();
}
