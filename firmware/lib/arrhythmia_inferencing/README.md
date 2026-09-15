# Edge Impulse model placeholder

The original repository did not contain the exported Edge Impulse model, so the reported neural-network result cannot be reproduced from source yet. The firmware detects this automatically and compiles in BPM-range-only mode.

To enable inference:

1. Train an Edge Impulse classification model using the same 10-value BPM window used by the notebook and firmware.
2. From **Deployment**, export the complete Arduino or C++ library.
3. Extract the package so PlatformIO can find `arrhythmia_inferencing.h` from this library directory. Keep the generated `edge-impulse-sdk`, `model-parameters`, and `tflite-model` content together.
4. If your generated header has a different project name, either rename its public header to `arrhythmia_inferencing.h` or update the include in `src/main.cpp`.
5. Rebuild and check the serial monitor for `Edge Impulse model found`.

Do not copy only the header: Edge Impulse exports contain generated model data and runtime sources that are also required.

