#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <string>
#include <factory/ObjectFactory.h>
#include <Gui/ViewportGui.h>

namespace Benchmark {
    struct BenchArgs {
        std::string mesh = "dragon202k.obj";
        int threads = 10, bvh = 1, pose = 1, frames = 30, lights = 1, block = 16;
    };

    BenchArgs ParseArgs(int argc, char** argv);
    int RunBenchmark(const BenchArgs& a, ObjectFactory& objectFactory, ViewportGui& viewportGui);
	void AddBenchLight(ObjectFactory& of, const Vec3& p, float level);
    void SetupBenchScene(ObjectFactory& of, Model* m, int pose, int lights);
}
#endif