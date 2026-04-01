import sys
import os
import glob

# Go up one level from backend\ to project root
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Finds the .pyd regardless of the cp312 suffix or Debug/Release folder
matches = glob.glob(os.path.join(BASE_DIR, "out", "build", "**", "raytracer_bindings*.pyd"), recursive=True)
if matches:
    sys.path.append(os.path.dirname(matches[0]))
else:
    raise RuntimeError("raytracer_bindings.pyd not found did you build the project?")

import raytracer_bindings
print(raytracer_bindings.add(3, 4)) 