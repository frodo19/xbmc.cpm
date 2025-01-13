/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "ProcessInfo.h"

#include "ServiceBroker.h"
#include "cores/DataCacheCore.h"
#include "cores/AudioEngine/Utils/AEStreamInfo.h"
#include "settings/AdvancedSettings.h"
#include "settings/SettingsComponent.h"
#include "utils/AMLUtils.h"

#include <libavutil/avutil.h>

#include <memory>
#include <mutex>

CCriticalSection createSection;
std::map<std::string, CreateProcessControl> CProcessInfo::m_processControls;

void CProcessInfo::RegisterProcessControl(const std::string& id, CreateProcessControl createFunc)
{
  std::unique_lock<CCriticalSection> lock(createSection);

  m_processControls.clear();
  m_processControls[id] = createFunc;
}

CProcessInfo* CProcessInfo::CreateInstance()
{
  std::unique_lock<CCriticalSection> lock(createSection);

  CProcessInfo *ret = nullptr;
  for (auto &info : m_processControls)
  {
    ret = info.second();
    if (ret)
      return ret;
  }
  return new CProcessInfo();
}

CProcessInfo::CProcessInfo()
{
  m_videoSettingsLocked =
      std::make_unique<CVideoSettingsLocked>(m_videoSettings, m_settingsSection);
}

void CProcessInfo::SetDataCache(CDataCacheCore *cache)
{
  m_dataCache = cache;

  ResetVideoCodecInfo();
  m_renderGuiLayer = false;
  m_renderVideoLayer = false;
  m_dataCache->SetGuiRender(m_renderGuiLayer);
  m_dataCache->SetVideoRender(m_renderVideoLayer);
}

//******************************************************************************
// video codec
//******************************************************************************
void CProcessInfo::ResetVideoCodecInfo()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoPts = 0;
  m_videoIsHWDecoder = false;
  m_videoDecoderName = "unknown";
  m_videoDeintMethod = "unknown";
  m_videoPixelFormat = "unknown";
  m_videoStereoMode.clear();
  m_videoWidth = 0;
  m_videoHeight = 0;
  m_videoFPS = 0.0;
  m_videoDAR = 0.0;
  m_videoBitDepth = 0;
  m_videoHdrType = StreamHdrType::HDR_TYPE_NONE;
  m_videoSourceHdrType = StreamHdrType::HDR_TYPE_NONE;
  m_videoSourceAdditionalHdrType = StreamHdrType::HDR_TYPE_NONE;
  m_videoColorSpace = AVCOL_SPC_UNSPECIFIED;
  m_videoColorRange = AVCOL_RANGE_UNSPECIFIED;
  m_videoColorPrimaries = AVCOL_PRI_UNSPECIFIED;
  m_videoColorTransferCharacteristic = AVCOL_TRC_UNSPECIFIED;
  m_videoDoViFrameMetadata = {};
  m_videoDoViStreamInfo = {};
  m_videoSourceDoViStreamInfo = {};
  m_videoDoViCodecFourCC = "";
  m_videoHDRStaticMetadataInfo = {};
  m_videoVS10Mode = DOLBY_VISION_OUTPUT_MODE_BYPASS;
  m_videoLiveBitRate = 0;
  m_videoQueueLevel = 0;
  m_videoQueueDataLevel = 0;
  m_videoIsInterlaced = false;
  m_deintMethods.clear();
  m_deintMethods.push_back(EINTERLACEMETHOD::VS_INTERLACEMETHOD_NONE);
  m_deintMethodDefault = EINTERLACEMETHOD::VS_INTERLACEMETHOD_NONE;
  m_stateSeeking = false;

  if (m_dataCache)
  {
    m_dataCache->SetVideoPts(m_videoPts);
    m_dataCache->SetVideoDecoderName(m_videoDecoderName, m_videoIsHWDecoder);
    m_dataCache->SetVideoDeintMethod(m_videoDeintMethod);
    m_dataCache->SetVideoPixelFormat(m_videoPixelFormat);
    m_dataCache->SetVideoDimensions(m_videoWidth, m_videoHeight);
    m_dataCache->SetVideoFps(m_videoFPS);
    m_dataCache->SetVideoDAR(m_videoDAR);
    m_dataCache->SetStateSeeking(m_stateSeeking);
    m_dataCache->SetVideoStereoMode(m_videoStereoMode);
    m_dataCache->SetVideoBitDepth(m_videoBitDepth);
    m_dataCache->SetVideoHdrType(m_videoHdrType);
    m_dataCache->SetVideoSourceHdrType(m_videoSourceHdrType);
    m_dataCache->SetVideoSourceAdditionalHdrType(m_videoSourceAdditionalHdrType);
    m_dataCache->SetVideoColorSpace(m_videoColorSpace);
    m_dataCache->SetVideoColorRange(m_videoColorRange);
    m_dataCache->SetVideoColorPrimaries(m_videoColorPrimaries);
    m_dataCache->SetVideoColorTransferCharacteristic(m_videoColorTransferCharacteristic);
    m_dataCache->SetVideoDoViFrameMetadata(m_videoDoViFrameMetadata);
    m_dataCache->SetVideoDoViStreamInfo(m_videoDoViStreamInfo);
    m_dataCache->SetVideoSourceDoViStreamInfo(m_videoSourceDoViStreamInfo);
    m_dataCache->SetVideoDoViCodecFourCC(m_videoDoViCodecFourCC);
    m_dataCache->SetVideoHDRStaticMetadataInfo(m_videoHDRStaticMetadataInfo);
    m_dataCache->SetVideoVS10Mode(m_videoVS10Mode);
    m_dataCache->SetVideoLiveBitRate(m_videoLiveBitRate);
    m_dataCache->SetVideoQueueLevel(m_videoQueueLevel);
    m_dataCache->SetVideoQueueDataLevel(m_videoQueueDataLevel);
  }
}

void CProcessInfo::SetVideoPts(double pts)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoPts = pts;

  if (m_dataCache)
    m_dataCache->SetVideoPts(m_videoPts);
}

double CProcessInfo::GetVideoPts()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoPts;
}

void CProcessInfo::SetVideoDecoderName(const std::string &name, bool isHw)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoIsHWDecoder = isHw;
  m_videoDecoderName = name;

  if (m_dataCache)
    m_dataCache->SetVideoDecoderName(m_videoDecoderName, m_videoIsHWDecoder);
}

std::string CProcessInfo::GetVideoDecoderName()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoDecoderName;
}

bool CProcessInfo::IsVideoHwDecoder()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoIsHWDecoder;
}

void CProcessInfo::SetVideoDeintMethod(const std::string &method)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoDeintMethod = method;

  if (m_dataCache)
    m_dataCache->SetVideoDeintMethod(m_videoDeintMethod);
}

std::string CProcessInfo::GetVideoDeintMethod()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoDeintMethod;
}

void CProcessInfo::SetVideoPixelFormat(const std::string &pixFormat)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoPixelFormat = pixFormat;

  if (m_dataCache)
    m_dataCache->SetVideoPixelFormat(m_videoPixelFormat);
}

std::string CProcessInfo::GetVideoPixelFormat()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoPixelFormat;
}

void CProcessInfo::SetVideoStereoMode(const std::string &mode)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoStereoMode = mode;

  if (m_dataCache)
    m_dataCache->SetVideoStereoMode(m_videoStereoMode);
}

std::string CProcessInfo::GetVideoStereoMode()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoStereoMode;
}

void CProcessInfo::SetVideoDimensions(int width, int height)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoWidth = width;
  m_videoHeight = height;

  if (m_dataCache)
    m_dataCache->SetVideoDimensions(m_videoWidth, m_videoHeight);
}

void CProcessInfo::GetVideoDimensions(int &width, int &height)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  width = m_videoWidth;
  height = m_videoHeight;
}

void CProcessInfo::SetVideoBitDepth(int bitDepth)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoBitDepth = bitDepth;

  if (m_dataCache)
    m_dataCache->SetVideoBitDepth(m_videoBitDepth);
}

int CProcessInfo::GetVideoBitDepth()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoBitDepth;
}

void CProcessInfo::SetVideoHdrType(StreamHdrType hdrType)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoHdrType = hdrType;

  if (m_dataCache)
    m_dataCache->SetVideoHdrType(m_videoHdrType);
}

StreamHdrType CProcessInfo::GetVideoHdrType()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoHdrType;
}

void CProcessInfo::SetVideoSourceHdrType(StreamHdrType hdrType)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoSourceHdrType = hdrType;

  if (m_dataCache)
    m_dataCache->SetVideoSourceHdrType(m_videoSourceHdrType);
}

StreamHdrType CProcessInfo::GetVideoSourceHdrType()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoSourceHdrType;
}

void CProcessInfo::SetVideoSourceAdditionalHdrType(StreamHdrType hdrType)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoSourceAdditionalHdrType = hdrType;

  if (m_dataCache)
    m_dataCache->SetVideoSourceAdditionalHdrType(m_videoSourceAdditionalHdrType);
}

StreamHdrType CProcessInfo::GetVideoSourceAdditionalHdrType()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoSourceAdditionalHdrType;
}

void CProcessInfo::SetVideoColorSpace(AVColorSpace colorSpace)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoColorSpace = colorSpace;

  if (m_dataCache)
    m_dataCache->SetVideoColorSpace(m_videoColorSpace);
}

AVColorSpace CProcessInfo::GetVideoColorSpace()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoColorSpace;
}

void CProcessInfo::SetVideoColorRange(AVColorRange colorRange)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoColorRange = colorRange;

  if (m_dataCache)
    m_dataCache->SetVideoColorRange(m_videoColorRange);
}

AVColorRange CProcessInfo::GetVideoColorRange()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoColorRange;
}

void CProcessInfo::SetVideoColorPrimaries(AVColorPrimaries colorPrimaries)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoColorPrimaries = colorPrimaries;

  if (m_dataCache)
    m_dataCache->SetVideoColorPrimaries(m_videoColorPrimaries);
}

AVColorPrimaries CProcessInfo::GetVideoColorPrimaries()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoColorPrimaries;
}

void CProcessInfo::SetVideoColorTransferCharacteristic(AVColorTransferCharacteristic colorTransferCharacteristic)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoColorTransferCharacteristic = colorTransferCharacteristic;

  if (m_dataCache)
    m_dataCache->SetVideoColorTransferCharacteristic(m_videoColorTransferCharacteristic);
}

AVColorTransferCharacteristic CProcessInfo::GetVideoColorTransferCharacteristic()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoColorTransferCharacteristic;
}

void CProcessInfo::SetVideoDoViFrameMetadata(DOVIFrameMetadata value)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoDoViFrameMetadata = value;

  if (m_dataCache)
    m_dataCache->SetVideoDoViFrameMetadata(m_videoDoViFrameMetadata);
}

DOVIFrameMetadata CProcessInfo::GetVideoDoViFrameMetadata()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoDoViFrameMetadata;
}

void CProcessInfo::SetVideoDoViStreamMetadata(DOVIStreamMetadata value)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoDoViStreamMetadata = value;

  if (m_dataCache)
    m_dataCache->SetVideoDoViStreamMetadata(m_videoDoViStreamMetadata);
}

DOVIStreamMetadata CProcessInfo::GetVideoDoViStreamMetadata()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoDoViStreamMetadata;
}

void CProcessInfo::SetVideoDoViStreamInfo(DOVIStreamInfo value)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoDoViStreamInfo = value;

  if (m_dataCache)
    m_dataCache->SetVideoDoViStreamInfo(m_videoDoViStreamInfo);
}

DOVIStreamInfo CProcessInfo::GetVideoDoViStreamInfo()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoDoViStreamInfo;
}

void CProcessInfo::SetVideoSourceDoViStreamInfo(DOVIStreamInfo value)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoSourceDoViStreamInfo = value;

  if (m_dataCache)
    m_dataCache->SetVideoSourceDoViStreamInfo(m_videoSourceDoViStreamInfo);
}

DOVIStreamInfo CProcessInfo::GetVideoSourceDoViStreamInfo()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoSourceDoViStreamInfo;
}

void CProcessInfo::SetVideoDoViCodecFourCC(std::string codecFourCC)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoDoViCodecFourCC = codecFourCC;

  if (m_dataCache)
    m_dataCache->SetVideoDoViCodecFourCC(m_videoDoViCodecFourCC);
}

std::string CProcessInfo::GetVideoDoViCodecFourCC()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoDoViCodecFourCC;
}

void CProcessInfo::SetVideoHDRStaticMetadataInfo(HDRStaticMetadataInfo value)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoHDRStaticMetadataInfo = value;

  if (m_dataCache)
    m_dataCache->SetVideoHDRStaticMetadataInfo(m_videoHDRStaticMetadataInfo);
}

HDRStaticMetadataInfo CProcessInfo::GetVideoHDRStaticMetadataInfo()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoHDRStaticMetadataInfo;
}

void CProcessInfo::SetVideoVS10Mode(unsigned int vs10Mode)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoVS10Mode= vs10Mode;

  if (m_dataCache)
    m_dataCache->SetVideoVS10Mode(m_videoVS10Mode);
}

unsigned int CProcessInfo::GetVideoVS10Mode()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoVS10Mode;
}

void CProcessInfo::SetVideoLiveBitRate(double bitRate)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoLiveBitRate= bitRate;

  if (m_dataCache)
    m_dataCache->SetVideoLiveBitRate(m_videoLiveBitRate);
}

double CProcessInfo::GetVideoLiveBitRate()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoLiveBitRate;
}

void CProcessInfo::SetVideoQueueLevel(int level)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoQueueLevel = level;

  if (m_dataCache)
    m_dataCache->SetVideoQueueLevel(m_videoQueueLevel);
}

int CProcessInfo::GetVideoQueueLevel()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoQueueLevel;
}

void CProcessInfo::SetVideoQueueDataLevel(int level)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoQueueDataLevel = level;

  if (m_dataCache)
    m_dataCache->SetVideoQueueDataLevel(m_videoQueueDataLevel);
}

int CProcessInfo::GetVideoQueueDataLevel()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoQueueDataLevel;
}

void CProcessInfo::SetVideoFps(float fps)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoFPS = fps;

  if (m_dataCache)
    m_dataCache->SetVideoFps(m_videoFPS);
}

float CProcessInfo::GetVideoFps()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoFPS;
}

void CProcessInfo::SetVideoDAR(float dar)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoDAR = dar;

  if (m_dataCache)
    m_dataCache->SetVideoDAR(m_videoDAR);
}

float CProcessInfo::GetVideoDAR()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoDAR;
}

void CProcessInfo::SetVideoInterlaced(bool interlaced)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_videoIsInterlaced = interlaced;

  if (m_dataCache)
    m_dataCache->SetVideoInterlaced(interlaced);
}

bool CProcessInfo::GetVideoInterlaced()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_videoIsInterlaced;
}

EINTERLACEMETHOD CProcessInfo::GetFallbackDeintMethod()
{
  return VS_INTERLACEMETHOD_DEINTERLACE;
}

void CProcessInfo::SetSwDeinterlacingMethods()
{
  std::list<EINTERLACEMETHOD> methods;
  methods.push_back(EINTERLACEMETHOD::VS_INTERLACEMETHOD_NONE);
  methods.push_back(EINTERLACEMETHOD::VS_INTERLACEMETHOD_DEINTERLACE);
  methods.push_back(EINTERLACEMETHOD::VS_INTERLACEMETHOD_DEINTERLACE_HALF);

  UpdateDeinterlacingMethods(methods);
  SetDeinterlacingMethodDefault(EINTERLACEMETHOD::VS_INTERLACEMETHOD_DEINTERLACE);
}

void CProcessInfo::UpdateDeinterlacingMethods(std::list<EINTERLACEMETHOD> &methods)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_deintMethods = methods;

  for (auto &deint : m_renderInfo.m_deintMethods)
  {
    if (!Supports(deint))
      m_deintMethods.push_back(deint);
  }

  if (!Supports(EINTERLACEMETHOD::VS_INTERLACEMETHOD_NONE))
    m_deintMethods.push_front(EINTERLACEMETHOD::VS_INTERLACEMETHOD_NONE);
}

bool CProcessInfo::Supports(EINTERLACEMETHOD method) const
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  auto it = std::find(m_deintMethods.begin(), m_deintMethods.end(), method);
  if (it != m_deintMethods.end())
    return true;

  return false;
}

void CProcessInfo::SetDeinterlacingMethodDefault(EINTERLACEMETHOD method)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_deintMethodDefault = method;
}

EINTERLACEMETHOD CProcessInfo::GetDeinterlacingMethodDefault() const
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  return m_deintMethodDefault;
}

CVideoBufferManager& CProcessInfo::GetVideoBufferManager()
{
  return m_videoBufferManager;
}

std::vector<AVPixelFormat> CProcessInfo::GetPixFormats()
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  if (m_pixFormats.empty())
  {
    return GetRenderFormats();
  }
  return m_pixFormats;
}

void CProcessInfo::SetPixFormats(std::vector<AVPixelFormat> &formats)
{
  std::unique_lock<CCriticalSection> lock(m_videoCodecSection);

  m_pixFormats = formats;
}

//******************************************************************************
// player audio info
//******************************************************************************
void CProcessInfo::ResetAudioCodecInfo()
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  m_audioDecoderName = "unknown";
  m_audioChannels = "unknown";
  m_audioSampleRate = 0;;
  m_audioBitsPerSample = 0;
  m_audioIsDolbyAtmos = false;
  m_audioDtsXType = DtsXType::DTS_X_NONE;
  m_audioLiveBitRate = 0;
  m_audioQueueLevel = 0;
  m_audioQueueDataLevel = 0;

  if (m_dataCache)
  {
    m_dataCache->SetAudioDecoderName(m_audioDecoderName);
    m_dataCache->SetAudioChannels(m_audioChannels);
    m_dataCache->SetAudioSampleRate(m_audioSampleRate);
    m_dataCache->SetAudioBitsPerSample(m_audioBitsPerSample);
    m_dataCache->SetAudioIsDolbyAtmos(m_audioIsDolbyAtmos);
    m_dataCache->SetAudioDtsXType(m_audioDtsXType);
    m_dataCache->SetAudioLiveBitRate(m_audioLiveBitRate);
    m_dataCache->SetAudioQueueLevel(m_audioQueueLevel);
    m_dataCache->SetAudioQueueDataLevel(m_audioQueueLevel);
  }
}

void CProcessInfo::SetAudioDecoderName(const std::string &name)
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  m_audioDecoderName = name;

  if (m_dataCache)
    m_dataCache->SetAudioDecoderName(m_audioDecoderName);
}

std::string CProcessInfo::GetAudioDecoderName()
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  return m_audioDecoderName;
}

void CProcessInfo::SetAudioChannels(const std::string &channels)
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  m_audioChannels = channels;

  if (m_dataCache)
    m_dataCache->SetAudioChannels(m_audioChannels);
}

std::string CProcessInfo::GetAudioChannels()
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  return m_audioChannels;
}

void CProcessInfo::SetAudioSampleRate(int sampleRate)
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  m_audioSampleRate = sampleRate;

  if (m_dataCache)
    m_dataCache->SetAudioSampleRate(m_audioSampleRate);
}

int CProcessInfo::GetAudioSampleRate()
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  return m_audioSampleRate;
}

void CProcessInfo::SetAudioBitsPerSample(int bitsPerSample)
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  m_audioBitsPerSample = bitsPerSample;

  if (m_dataCache)
    m_dataCache->SetAudioBitsPerSample(m_audioBitsPerSample);
}

int CProcessInfo::GetAudioBitsPerSample()
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  return m_audioBitsPerSample;
}

void CProcessInfo::SetAudioIsDolbyAtmos(bool isDolbyAtmos)
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  m_audioIsDolbyAtmos = isDolbyAtmos;

  if (m_dataCache)
    m_dataCache->SetAudioIsDolbyAtmos(m_audioIsDolbyAtmos);
}

bool CProcessInfo::GetAudioIsDolbyAtmos()
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  return m_audioIsDolbyAtmos;
}

void CProcessInfo::SetAudioDtsXType(DtsXType dtsXType)
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  m_audioDtsXType = dtsXType;

  if (m_dataCache)
    m_dataCache->SetAudioDtsXType(m_audioDtsXType);
}

DtsXType CProcessInfo::GetAudioDtsXType()
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  return m_audioDtsXType;
}

void CProcessInfo::SetAudioLiveBitRate(double bitRate)
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  m_audioLiveBitRate = bitRate;

  if (m_dataCache)
    m_dataCache->SetAudioLiveBitRate(m_audioLiveBitRate);
}

double CProcessInfo::GetAudioLiveBitRate()
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  return m_audioLiveBitRate;
}

void CProcessInfo::SetAudioQueueLevel(int level)
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  m_audioQueueLevel = level;

  if (m_dataCache)
    m_dataCache->SetAudioQueueLevel(m_audioQueueLevel);
}

int CProcessInfo::GetAudioQueueLevel()
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  return m_audioQueueLevel;
}

void CProcessInfo::SetAudioQueueDataLevel(int level)
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  m_audioQueueDataLevel = level;

  if (m_dataCache)
    m_dataCache->SetAudioQueueDataLevel(m_audioQueueDataLevel);
}

int CProcessInfo::GetAudioQueueDataLevel()
{
  std::unique_lock<CCriticalSection> lock(m_audioCodecSection);

  return m_audioQueueDataLevel;
}

bool CProcessInfo::AllowDTSHDDecode()
{
  return true;
}

void CProcessInfo::SetRenderClockSync(bool enabled)
{
  std::unique_lock<CCriticalSection> lock(m_renderSection);

  m_isClockSync = enabled;

  if (m_dataCache)
    m_dataCache->SetRenderClockSync(enabled);
}

bool CProcessInfo::IsRenderClockSync()
{
  std::unique_lock<CCriticalSection> lock(m_renderSection);

  return m_isClockSync;
}

void CProcessInfo::UpdateRenderInfo(CRenderInfo &info)
{
  std::unique_lock<CCriticalSection> lock(m_renderSection);

  m_renderInfo = info;

  for (auto &deint : m_renderInfo.m_deintMethods)
  {
    if (!Supports(deint))
      m_deintMethods.push_back(deint);
  }
}

void CProcessInfo::UpdateRenderBuffers(int queued, int discard, int free)
{
  std::unique_lock<CCriticalSection> lock(m_renderSection);
  m_renderBufQueued = queued;
  m_renderBufDiscard = discard;
  m_renderBufFree = free;
}

void CProcessInfo::GetRenderBuffers(int &queued, int &discard, int &free)
{
  std::unique_lock<CCriticalSection> lock(m_renderSection);
  queued = m_renderBufQueued;
  discard = m_renderBufDiscard;
  free = m_renderBufFree;
}

std::vector<AVPixelFormat> CProcessInfo::GetRenderFormats()
{
  std::vector<AVPixelFormat> formats;
  formats.push_back(AV_PIX_FMT_YUV420P);
  return formats;
}

//******************************************************************************
// player states
//******************************************************************************
void CProcessInfo::SeekFinished(int64_t offset)
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);
  if (m_dataCache)
    m_dataCache->SeekFinished(offset);
}

void CProcessInfo::SetStateSeeking(bool active)
{
  std::unique_lock<CCriticalSection> lock(m_renderSection);

  m_stateSeeking = active;

  if (m_dataCache)
    m_dataCache->SetStateSeeking(active);
}

bool CProcessInfo::IsSeeking()
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  return m_stateSeeking;
}

void CProcessInfo::SetStateRealtime(bool state)
{
  std::unique_lock<CCriticalSection> lock(m_renderSection);

  m_realTimeStream = state;
}

bool CProcessInfo::IsRealtimeStream()
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  return m_realTimeStream;
}

void CProcessInfo::SetSpeed(float speed)
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  m_speed = speed;
  m_newSpeed = speed;

  if (m_dataCache)
    m_dataCache->SetSpeed(m_newTempo, speed);
}

void CProcessInfo::SetNewSpeed(float speed)
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  m_newSpeed = speed;

  if (m_dataCache)
    m_dataCache->SetSpeed(m_tempo, speed);
}

float CProcessInfo::GetNewSpeed()
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  return m_newSpeed;
}

void CProcessInfo::SetFrameAdvance(bool fa)
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  m_frameAdvance = fa;

  if (m_dataCache)
    m_dataCache->SetFrameAdvance(fa);
}

bool CProcessInfo::IsFrameAdvance()
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  return m_frameAdvance;
}

void CProcessInfo::SetTempo(float tempo)
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  m_tempo = tempo;
  m_newTempo = tempo;

  if (m_dataCache)
    m_dataCache->SetSpeed(tempo, m_newSpeed);
}

void CProcessInfo::SetNewTempo(float tempo)
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  m_newTempo = tempo;

  if (m_dataCache)
    m_dataCache->SetSpeed(tempo, m_speed);
}

float CProcessInfo::GetNewTempo()
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  return m_newTempo;
}

float CProcessInfo::MinTempoPlatform()
{
  return 0.75f;
}

float CProcessInfo::MaxTempoPlatform()
{
  return 1.55f;
}

bool CProcessInfo::IsTempoAllowed(float tempo)
{
  if (tempo > MinTempoPlatform() &&
      (tempo < MaxTempoPlatform() || tempo < CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_maxTempo))
    return true;

  return false;
}

unsigned int CProcessInfo::GetMaxPassthroughOffSyncDuration() const
{
  return CServiceBroker::GetSettingsComponent()
      ->GetAdvancedSettings()
      ->m_maxPassthroughOffSyncDuration;
}

void CProcessInfo::SetLevelVQ(int level)
{
  m_levelVQ = level;
}

int CProcessInfo::GetLevelVQ()
{
  return m_levelVQ;
}

void CProcessInfo::SetGuiRender(bool gui)
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  bool change = (m_renderGuiLayer != gui);
  m_renderGuiLayer = gui;
  if (change)
  {
    if (m_dataCache)
        m_dataCache->SetGuiRender(gui);
  }
}

bool CProcessInfo::GetGuiRender()
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  return m_renderGuiLayer;
}

void CProcessInfo::SetVideoRender(bool video)
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  bool change = (m_renderVideoLayer != video);
  m_renderVideoLayer = video;
  if (change)
  {
    if (m_dataCache)
      m_dataCache->SetVideoRender(video);
  }
}

bool CProcessInfo::GetVideoRender()
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);

  return m_renderVideoLayer;
}

void CProcessInfo::SetPlayTimes(time_t start, int64_t current, int64_t min, int64_t max)
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);
  m_startTime = start;
  m_time = current;
  m_timeMin = min;
  m_timeMax = max;

  if (m_dataCache)
  {
    m_dataCache->SetPlayTimes(start, current, min, max);
  }
}

int64_t CProcessInfo::GetMaxTime()
{
  std::unique_lock<CCriticalSection> lock(m_stateSection);
  return m_timeMax;
}

//******************************************************************************
// settings
//******************************************************************************
CVideoSettings CProcessInfo::GetVideoSettings()
{
  std::unique_lock<CCriticalSection> lock(m_settingsSection);
  return m_videoSettings;
}

CVideoSettingsLocked& CProcessInfo::GetVideoSettingsLocked()
{
  std::unique_lock<CCriticalSection> lock(m_settingsSection);
  return *m_videoSettingsLocked;
}

void CProcessInfo::SetVideoSettings(CVideoSettings &settings)
{
  std::unique_lock<CCriticalSection> lock(m_settingsSection);
  m_videoSettings = settings;
}
