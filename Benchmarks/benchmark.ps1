# Find project root
$root = Split-Path $PSScriptRoot -Parent

# Possible executable locations
$exeCandidates = @(
    (Join-Path $root "RaytracerViewer.exe"),
    (Join-Path $root "RaytracerViewer\out\build\x64-Release\Release\RaytracerViewer.exe")
    (Join-Path $root "out\build\x64-Release\Release\RaytracerViewer.exe")
)

# Select the first executable that exists
$exe = $exeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1

if (-not $exe) {
    Write-Error "RaytracerViewer.exe not found. Build or download the Release version first."
    exit 1
}

# Output file
$outputFile = Join-Path $PSScriptRoot "benchmark_results.txt"

Write-Host "Executable: $exe"
Write-Host "Saving results to: $outputFile"
Write-Host "Starting benchmarks..."

# Dragon multithreading benchmark
foreach ($t in 1, 2, 4, 8, 10) {
    Write-Host "`nTesting Dragon with $t threads..."
    & $exe --bench --mesh dragon202k.obj --threads $t --pose 2 --lights 1 --frames 30 | Select-String "mesh="
}

# Bunny BVH enabled
Write-Host "`nTesting Bunny with BVH enabled..."
& $exe --bench --mesh bunny.obj --bvh 1 --pose 2 --frames 30 | Select-String "mesh="

# Bunny BVH disabled
Write-Host "`nTesting Bunny with BVH disabled..."
& $exe --bench --mesh bunny.obj --bvh 0 --pose 2 --frames 3 | Select-String "mesh="

Write-Host "`nAll benchmarks completed!"
