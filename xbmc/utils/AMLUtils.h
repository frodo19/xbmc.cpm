/*
 *  Copyright (C) 2011-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "windowing/Resolution.h"

#include <string>
#include <vector>

enum AML_DEVICE_TYPE
{
  AML_DEVICE_TYPE_UNINIT   = -2,
  AML_DEVICE_TYPE_UNKNOWN  = -1,
  AML_DEVICE_TYPE_M1,
  AML_DEVICE_TYPE_M3,
  AML_DEVICE_TYPE_M6,
  AML_DEVICE_TYPE_M8,   // S802
  AML_DEVICE_TYPE_M8B,  // S805
  AML_DEVICE_TYPE_M8M2  // S812
};

enum AML_DISPLAY_AXIS_PARAM
{
  AML_DISPLAY_AXIS_PARAM_X = 0,
  AML_DISPLAY_AXIS_PARAM_Y,
  AML_DISPLAY_AXIS_PARAM_WIDTH,
  AML_DISPLAY_AXIS_PARAM_HEIGHT
};

enum AML_SUPPORT_H264_4K2K
{
  AML_SUPPORT_H264_4K2K_UNINIT = -1,
  AML_NO_H264_4K2K,
  AML_HAS_H264_4K2K,
  AML_HAS_H264_4K2K_SAME_PROFILE
};

enum DV_MODE : int
{
  DV_MODE_ON = 0,
  DV_MODE_ON_DEMAND,
  DV_MODE_OFF
};

enum DV_TYPE : int
{
  DV_TYPE_DISPLAY_LED = 0,
  DV_TYPE_PLAYER_LED_LLDV,
  DV_TYPE_PLAYER_LED_HDR
};

enum DV_COLORIMETRY : int
{
  DV_COLORIMETRY_BT2020NC = 1,
  DV_COLORIMETRY_REMOVE
};

#define AML_GXBB    0x1F
#define AML_GXL     0x21
#define AML_GXM     0x22
#define AML_G12A    0x28
#define AML_G12B    0x29
#define AML_SM1     0x2B

int  aml_get_cpufamily_id();
bool aml_display_support_dv();
bool aml_dv_support_ll();
bool aml_dv_support_std();
bool aml_display_support_3d();
bool aml_support_hevc();
bool aml_support_hevc_4k2k();
bool aml_support_hevc_8k4k();
bool aml_support_hevc_10bit();
AML_SUPPORT_H264_4K2K aml_support_h264_4k2k();
bool aml_support_vp9();
bool aml_support_av1();
bool aml_support_dolby_vision();
bool aml_dolby_vision_enabled();
void aml_dv_on(unsigned int mode, bool enable = false);
void aml_dv_off(bool disable = false);
int aml_dv_set_osd_max(int max)
void aml_dv_enable();
void aml_dv_disable();
bool aml_is_dv_enable();
void aml_dv_display_trigger();
void aml_dv_start();
bool aml_has_frac_rate_policy();
bool aml_video_started();
void aml_video_mute(bool mute);
void aml_set_audio_passthrough(bool passthrough);
void aml_set_3d_video_mode(unsigned int mode, bool framepacking_support, int view_mode);
bool aml_mode_to_resolution(const char *mode, RESOLUTION_INFO *res);
bool aml_get_native_resolution(RESOLUTION_INFO *res);
bool aml_set_native_resolution(const RESOLUTION_INFO &res, std::string framebuffer_name, const int stereo_mode, bool force_mode_switch);
bool aml_probe_resolutions(std::vector<RESOLUTION_INFO> &resolutions);
bool aml_set_display_resolution(const RESOLUTION_INFO &res, std::string framebuffer_name, bool force_mode_switch);
void aml_handle_scale(const RESOLUTION_INFO &res);
void aml_handle_display_stereo_mode(const int stereo_mode);
void aml_enable_freeScale(const RESOLUTION_INFO &res);
void aml_disable_freeScale();
void aml_set_framebuffer_resolution(const RESOLUTION_INFO &res, std::string framebuffer_name);
void aml_set_framebuffer_resolution(unsigned int width, unsigned int height, std::string framebuffer_name);
bool aml_read_reg(const std::string &reg, uint32_t &reg_val);
bool aml_has_capability_ignore_alpha();
bool aml_set_reg_ignore_alpha();
bool aml_unset_reg_ignore_alpha();