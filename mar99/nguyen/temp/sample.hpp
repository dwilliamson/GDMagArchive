#include "meshes.hpp"
#include "raster.hpp"
#include "camera.hpp"

void InitScene(void);
void InnerLoop(void);
void CameraControl(int , int );
void SetSpotCamera(void);
void SetScreenCamera(void);
void MeshControl(int, float);
void ToggleCapture(void);