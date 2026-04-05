#ifndef RAYTRACER_API_H
#define RAYTRACER_API_H

#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

#include "Threads/ThreadSafeQueue.h"

class RaytracerAPI {
	private:
		//ParseJsonScene(const std::string& jsonScene));
	public:
		RaytracerAPI();

		int Render(const std::string& jsonScene, int width, int height);
};

#endif // !RAYTRACER_API_H