# Astro project

Some apps to explore astrophotography algorithms

## Dependencies

For CUDA get the CUDA Toolkit.

3rd party libs from `vcpkg`: Now fully automated via `CMakePresets.json` and `vcpkg.json`.
```
mkdir out && mkdir out/debug
cmake --preset linux-debug
``` 
and to build
```
cmake --build --preset linux-debug --target astro_cli
```

