#include "RtAudioRenderer.h"

#include <RtAudio.h>

#include <QDebug>

int rtAudioMoo(void *outputBuffer, void * /*inputBuffer*/,
               unsigned int nBufferFrames, double /*streamTime*/,
               RtAudioStreamStatus status, void *userData) {
  RtAudioRenderer *renderer = (RtAudioRenderer *)userData;
  if (status) qWarning() << "Stream underflow detected!";

  int byte_per_smp, num_channels;
  if (!renderer->m_pxtn->get_byte_per_smp(&byte_per_smp)) return 1;
  if (!renderer->m_pxtn->get_destination_quality(&num_channels, nullptr))
    return 1;

  int size = nBufferFrames * byte_per_smp * num_channels;
  qDebug() << size;
  renderer->m_pxtn->Moo(renderer->m_moo_state, outputBuffer, size, nullptr,
                        nullptr);
  return 0;
}

RtAudioRenderer::RtAudioRenderer(const pxtnService *pxtn)
    : m_pxtn(pxtn), m_dac() {
  if (m_dac.getDeviceCount() < 1)
    throw std::runtime_error("No audio devices found");

  RtAudio::StreamParameters parameters;
  parameters.deviceId = m_dac.getDefaultOutputDevice();
  parameters.nChannels = 2;
  parameters.firstChannel = 0;
  if (m_pxtn->tones_ready(m_moo_state) != pxtnOK) {
    throw std::runtime_error("tones ready failed");
  }

  int sampleRate = 44100;
  unsigned int bufferFrames = 256;  // 256 sample frames
  m_dac.openStream(&parameters, NULL, RTAUDIO_SINT16, sampleRate, &bufferFrames,
                   &rtAudioMoo, this);
  m_dac.startStream();
  qDebug() << "started stream";
}

RtAudioRenderer::~RtAudioRenderer() {
  try {
    m_dac.stopStream();
  } catch (RtAudioError &e) {
    e.printMessage();
  }
  if (m_dac.isStreamOpen()) m_dac.closeStream();
}
