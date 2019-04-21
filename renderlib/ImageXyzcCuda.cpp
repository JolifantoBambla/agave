#include "ImageXyzcCuda.h"

#include "ImageXYZC.h"
#include "Logging.h"
#include "gl/Util.h"

#include "gl/Util.h"

#include <QElapsedTimer>
#include <QtDebug>

void
ChannelCuda::allocGpu(ImageXYZC* img, int channel)
{
  Channelu16* ch = img->channel(channel);

  // LUT buffer

  const int LUT_SIZE = 256;

  m_gpuBytes += (32) / 8 * LUT_SIZE;

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glGenTextures(1, &m_VolumeLutGLTexture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_VolumeLutGLTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, LUT_SIZE, 1);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, LUT_SIZE, 1, 0, GL_RED, GL_FLOAT, ch->m_lut);

  glBindTexture(GL_TEXTURE_2D, 0);
  check_gl("volume lut texture creation");
}

void
ChannelCuda::deallocGpu()
{
  glDeleteTextures(1, &m_VolumeLutGLTexture);
  m_VolumeLutGLTexture = 0;

  m_gpuBytes = 0;
}

void
ChannelCuda::updateLutGpu(int channel, ImageXYZC* img)
{
  static const int LUT_SIZE = 256;
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, 0);
  check_gl("update lut texture");
  glBindTexture(GL_TEXTURE_2D, m_VolumeLutGLTexture);
  check_gl("update lut texture");

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, LUT_SIZE, 1, GL_RED, GL_FLOAT, img->channel(channel)->m_lut);
  check_gl("update lut texture");

  glBindTexture(GL_TEXTURE_2D, 0);
  check_gl("update lut texture");
}

void
ImageCuda::createVolumeTextureFusedRGBA8(ImageXYZC* img)
{
  m_gpuBytes += (8 + 8 + 8 + 8) / 8 * img->sizeX() * img->sizeY() * img->sizeZ();

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glGenTextures(1, &m_VolumeGLTexture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_3D, m_VolumeGLTexture);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, img->sizeX(), img->sizeY(), img->sizeZ());
  glBindTexture(GL_TEXTURE_3D, 0);
  check_gl("fused rgb volume texture creation");
}

void
ImageCuda::createVolumeTexture4x16(ImageXYZC* img)
{
  m_gpuBytes += (16 * 4) / 8 * img->sizeX() * img->sizeY() * img->sizeZ();

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glGenTextures(1, &m_VolumeGLTexture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_3D, m_VolumeGLTexture);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA16, img->sizeX(), img->sizeY(), img->sizeZ());
  glBindTexture(GL_TEXTURE_3D, 0);
  check_gl("volume texture creation");
}

void
ImageCuda::updateVolumeData4x16(ImageXYZC* img, int c0, int c1, int c2, int c3)
{
  QElapsedTimer timer;
  timer.start();

  const int N = 4;
  int ch[4] = { c0, c1, c2, c3 };
  // interleaved all channels.
  // first 4.
  size_t xyz = img->sizeX() * img->sizeY() * img->sizeZ();
  uint16_t* v = new uint16_t[xyz * N];

  for (uint32_t i = 0; i < xyz; ++i) {
    for (int j = 0; j < N; ++j) {
      v[N * (i) + j] = img->channel(ch[j])->m_ptr[(i)];
    }
  }

  LOG_DEBUG << "Prepared interleaved hostmem buffer: " << timer.elapsed() << "ms";
  timer.start();

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_3D, m_VolumeGLTexture);
  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, img->sizeX(), img->sizeY(), img->sizeZ(), GL_RGBA, GL_UNSIGNED_SHORT, v);
  glBindTexture(GL_TEXTURE_3D, 0);
  check_gl("update volume texture");

  LOG_DEBUG << "Copy volume to gpu: " << timer.elapsed() << "ms";

  delete[] v;
}

void
ImageCuda::allocGpuInterleaved(ImageXYZC* img)
{
  deallocGpu();
  m_channels.clear();

  QElapsedTimer timer;
  timer.start();

  createVolumeTexture4x16(img);
  uint32_t numChannels = img->sizeC();
  updateVolumeData4x16(
    img, 0, std::min(1u, numChannels - 1), std::min(2u, numChannels - 1), std::min(3u, numChannels - 1));

  for (uint32_t i = 0; i < numChannels; ++i) {
    ChannelCuda c;
    c.m_index = i;
    c.allocGpu(img, i);
    m_channels.push_back(c);

    m_gpuBytes += c.m_gpuBytes;
  }

  LOG_DEBUG << "allocGPUinterleaved: Image to GPU in " << timer.elapsed() << "ms";
  LOG_DEBUG << "allocGPUinterleaved: GPU bytes: " << m_gpuBytes;
}

void
ImageCuda::deallocGpu()
{
  for (size_t i = 0; i < m_channels.size(); ++i) {
    m_channels[i].deallocGpu();
  }

  // needs current gl context.

  check_gl("pre-destroy gl volume texture");
  //  glBindTexture(GL_TEXTURE_3D, 0);
  glDeleteTextures(1, &m_VolumeGLTexture);
  check_gl("destroy gl volume texture");
  m_VolumeGLTexture = 0;

  m_gpuBytes = 0;
}

void
ImageCuda::updateLutGpu(int channel, ImageXYZC* img)
{
  m_channels[channel].updateLutGpu(channel, img);
}
