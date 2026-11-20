#include "Benchmark.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <vector>


namespace Benchmark {

    BenchArgs ParseArgs(int argc, char** argv) {
        BenchArgs a;
        for (int i = 1; i < argc; ++i) {
            std::string k = argv[i];
            auto next = [&]() { return (i + 1 < argc) ? argv[++i] : ""; };
            if (k == "--mesh")    a.mesh = next();
            else if (k == "--threads") a.threads = std::atoi(next());
            else if (k == "--bvh")     a.bvh = std::atoi(next());
            else if (k == "--pose")    a.pose = std::atoi(next());
            else if (k == "--frames")  a.frames = std::atoi(next());
            else if (k == "--lights")  a.lights = std::atoi(next());
			else if (k == "--block")   a.block = std::atoi(next());
        }
        return a;
    }

    int RunBenchmark(const BenchArgs& a, ObjectFactory& objectFactory, ViewportGui& viewportGui) {
        auto& models = objectFactory.GetFactory<ModelFactory>().GetObjects();
        size_t firstNew = models.size();

        std::string meshPath = "C:/dev/RaytracerViewer/RaytracerViewer/assets/" + a.mesh;
        objectFactory.CreateObj(meshPath);

        SetupBenchScene(objectFactory, models[firstNew].get(), std::clamp(a.pose, 1, 3), a.lights);
        viewportGui.SetBlockSize(a.block);
        viewportGui.ApplyRenderSize(640, 360);

        size_t tris = 0;
        double build = 0.0;
        for (size_t i = firstNew; i < models.size(); ++i) {
            Model* model = models[i].get();
            tris += model->GetTriangles().size();

            BVH& bvh = model->GetBVH();
            bvh.SetMaxDepth(a.bvh ? 64 : 0);
            auto b0 = std::chrono::steady_clock::now();
            bvh.Build();
            auto b1 = std::chrono::steady_clock::now();
            build += std::chrono::duration<double, std::milli>(b1 - b0).count();
        }

        const int warmup = a.frames >= 10 ? 5 : 1;
        std::vector<double> ms;
        for (int f = 0; f < a.frames + warmup; ++f) {
            double t = viewportGui.BenchFrameMs();
            if (f >= warmup) ms.push_back(t);
        }

        std::sort(ms.begin(), ms.end());
        double median = ms[ms.size() / 2];
        double p95 = ms[std::min(ms.size() - 1, static_cast<size_t>(ms.size() * 0.95))];
        std::printf("mesh=%s tris=%zu threads=%d bvh=%d res=640x360 build=%.0fms median=%.1fms p95=%.1fms (%.1f fps)\n",
            a.mesh.c_str(), tris, a.threads, a.bvh, build, median, p95, 1000.0 / median);

        viewportGui.SavePpm("Benchmarks/Renders/bench_" + a.mesh + "_p" +
            std::to_string(a.pose) + "_l" + std::to_string(a.lights) +
            "_b" + std::to_string(a.bvh) + ".ppm");
        return 0;
    }

    void AddBenchLight(ObjectFactory& of, const Vec3& p, float level) {
        std::string c = std::to_string(level);
        std::vector<std::string> args = {
            std::to_string(p.x), std::to_string(p.y), std::to_string(p.z),
            "1",
            c, c, c
        };
        of.GetFactoryMap()[typeid(LightFactory)].get()->CreateObject(std::string("light"), args);
    }

    void SetupBenchScene(ObjectFactory& of, Model* m, int pose, int lights) {
        const float kFov = 45.0f;
        static const float kDir[3][3] = { {0, 0.3f, 1}, {0.7f, 0.4f, 0.7f}, {0, 0.3f, 1} };
        static const float kFill[3] = { 0.90f, 0.60f, 0.20f };

        float r = 0.0f;
        for (auto& v : m->GetVertices()) {
            r = std::max(r, std::sqrt(v.pos.x * v.pos.x + v.pos.y * v.pos.y + v.pos.z * v.pos.z));
        }

        Vec3 c = m->pos;
        Vec3 dir = Vec3(kDir[pose - 1][0], kDir[pose - 1][1], kDir[pose - 1][2]).Normalize();
        float dist = r / std::sin(0.5f * kFov * 3.14159265f / 180.0f * kFill[pose - 1]);

        if (of.GetCameras().empty()) {
            of.AddCamera(Raytracer::Camera(Vec3(0, 0, 10), Vec3(0, 0, -1), Vec3(0, 1, 0), kFov));
        }
        Raytracer::Camera& cam = of.GetCameraIndex(0);   
        cam.eye = c + dir * dist;
        cam.viewdir = dir * -1.0f;
        cam.CalcCoords();

        AddBenchLight(of, c + Vec3(2 * r, 3 * r, 3 * r), 1.0f);        
        if (lights >= 3) {
            AddBenchLight(of, c + Vec3(-3 * r, 1 * r, 2 * r), 0.5f);    
            AddBenchLight(of, c + Vec3(0, 2 * r, -3 * r), 0.5f);  
        }
    }

}
