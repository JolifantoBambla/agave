#include "pybind11/numpy.h"
#include "pybind11/pybind11.h"

//#include "pyrenderer.h"

#include "renderlib/FileReader.h"
#include "renderlib/ImageXYZC.h"

namespace py = pybind11;

class TestAgave
{
public:
  int add(int x, int y) { return x + y; }
  void load(void* data, size_t size) {}
};

PYBIND11_MODULE(pyagave, m)
{
  m.doc() = "agave plugin"; // optional module docstring
  py::class_<TestAgave>(m, "renderer")
    .def(py::init<>())
    .def("add", &TestAgave::add)
    .def("load", [](py::array_t<uint16_t, py::array::c_style | py::array::forcecast> v) {
      py::buffer_info info = v.request();

      std::vector<uint32_t> newshape;
      for (auto d : info.shape) {
        newshape.push_back(d);
      }
      std::string name = "0";

      std::shared_ptr<ImageXYZC> img = FileReader::loadFromArray_4D(static_cast<uint8_t*>(info.ptr), newshape, name);

      return img;
    });

  /*
    py::class_<OffscreenRenderer>(m, "renderer")
      .def(py::init<>())
      .def(SessionCommand::PythonName().c_str(), &OffscreenRenderer::Session)
      .def(AssetPathCommand::PythonName().c_str(), &OffscreenRenderer::AssetPath)
      .def(LoadOmeTifCommand::PythonName().c_str(), &OffscreenRenderer::LoadOmeTif)
      .def(SetCameraPosCommand::PythonName().c_str(), &OffscreenRenderer::Eye)
      .def(SetCameraTargetCommand::PythonName().c_str(), &OffscreenRenderer::Target)
      .def(SetCameraUpCommand::PythonName().c_str(), &OffscreenRenderer::Up)
      .def(SetCameraApertureCommand::PythonName().c_str(), &OffscreenRenderer::Aperture)
      .def(SetCameraProjectionCommand::PythonName().c_str(), &OffscreenRenderer::CameraProjection)
      .def(SetCameraFocalDistanceCommand::PythonName().c_str(), &OffscreenRenderer::Focaldist)
      .def(SetCameraExposureCommand::PythonName().c_str(), &OffscreenRenderer::Exposure)
      .def(SetDiffuseColorCommand::PythonName().c_str(), &OffscreenRenderer::MatDiffuse)
      .def(SetSpecularColorCommand::PythonName().c_str(), &OffscreenRenderer::MatSpecular)
      .def(SetEmissiveColorCommand::PythonName().c_str(), &OffscreenRenderer::MatEmissive)
      .def(SetRenderIterationsCommand::PythonName().c_str(), &OffscreenRenderer::RenderIterations)
      .def(SetStreamModeCommand::PythonName().c_str(), &OffscreenRenderer::StreamMode)
      .def(RequestRedrawCommand::PythonName().c_str(), &OffscreenRenderer::Redraw)
      .def(SetResolutionCommand::PythonName().c_str(), &OffscreenRenderer::SetResolution)
      .def(SetDensityCommand::PythonName().c_str(), &OffscreenRenderer::Density)
      .def(FrameSceneCommand::PythonName().c_str(), &OffscreenRenderer::FrameScene)
      .def(SetGlossinessCommand::PythonName().c_str(), &OffscreenRenderer::MatGlossiness)
      .def(EnableChannelCommand::PythonName().c_str(), &OffscreenRenderer::EnableChannel)
      .def(SetWindowLevelCommand::PythonName().c_str(), &OffscreenRenderer::SetWindowLevel)
      .def(OrbitCameraCommand::PythonName().c_str(), &OffscreenRenderer::OrbitCamera)
      .def(SetSkylightTopColorCommand::PythonName().c_str(), &OffscreenRenderer::SkylightTopColor)
      .def(SetSkylightMiddleColorCommand::PythonName().c_str(), &OffscreenRenderer::SkylightMiddleColor)
      .def(SetSkylightBottomColorCommand::PythonName().c_str(), &OffscreenRenderer::SkylightBottomColor)
      .def(SetLightPosCommand::PythonName().c_str(), &OffscreenRenderer::LightPos)
      .def(SetLightColorCommand::PythonName().c_str(), &OffscreenRenderer::LightColor)
      .def(SetLightSizeCommand::PythonName().c_str(), &OffscreenRenderer::LightSize)
      .def(SetClipRegionCommand::PythonName().c_str(), &OffscreenRenderer::SetClipRegion)
      .def(SetVoxelScaleCommand::PythonName().c_str(), &OffscreenRenderer::SetVoxelScale)
      .def(AutoThresholdCommand::PythonName().c_str(), &OffscreenRenderer::AutoThreshold)
      .def(SetPercentileThresholdCommand::PythonName().c_str(), &OffscreenRenderer::SetPercentileThreshold)
      .def(SetOpacityCommand::PythonName().c_str(), &OffscreenRenderer::MatOpacity)
      .def(SetPrimaryRayStepSizeCommand::PythonName().c_str(), &OffscreenRenderer::SetPrimaryRayStepSize)
      .def(SetSecondaryRayStepSizeCommand::PythonName().c_str(), &OffscreenRenderer::SetSecondaryRayStepSize)
      .def(SetBackgroundColorCommand::PythonName().c_str(), &OffscreenRenderer::BackgroundColor)
      .def(SetIsovalueThresholdCommand::PythonName().c_str(), &OffscreenRenderer::SetIsovalueThreshold);
      */
}
