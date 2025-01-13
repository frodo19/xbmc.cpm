/*
 *  Copyright (C) 2024 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "HDR10Plus.h"

enum class PeakBrightnessSource {
  Histogram = 0,
  Histogram99,
  MaxScl,
  MaxSclLuminance,
  HistogramPlus
};

struct VdrDmData {

  VdrDmData() {} // Default constructor

  uint16_t min_pq;
  uint16_t max_pq;
  uint16_t avg_pq;

  uint16_t max_display_mastering_luminance;
  uint16_t min_display_mastering_luminance;
  uint16_t max_content_light_level;
  uint16_t max_frame_average_light_level;
};

std::vector<uint8_t> create_rpu_nalu_for_hdr10plus(
  const Hdr10PlusMetadata& meta,
  PeakBrightnessSource peak_source,
  uint16_t max_display_mastering_luminance,
  uint16_t min_display_mastering_luminance,
  uint16_t max_content_light_level,
  uint16_t max_frame_average_light_level);
