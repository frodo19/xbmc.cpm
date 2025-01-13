/*
 *  Copyright (C) 2024 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "HDR10PlusWriter.h"

#include "BitstreamIoWriter.h"
#include "HDR10PlusConvert.h"
#include "HDR10Plus.h"
#include "HevcSei.h"

void write_rpu(BitstreamIoWriter& writer, VdrDmData& vdr_dm_data) {

    writer.write_n<uint8_t>(2, 6);      // 6   (000010)         rpu_type           2 for the BDA Ultra HD Blu-ray Option-A HDR coding system
    writer.write_n<uint16_t>(18, 11);   // 17  (00000010010)    rpu_format         18
    writer.write_n<uint8_t>(1, 4);      // 21  (0001)           vdr_rpu_profile
    writer.write_n<uint8_t>(0, 4);      // 25  (0000)           vdr_rpu_level
    writer.write(true);                 // 26  (1)              vdr_seq_info_present_flag
    writer.write(false);                // 27  (0)              chroma_resampling_explicit_filter_flag
    writer.write_n<uint8_t>(0, 2);      // 29  (00)             coefficient_data_type
    writer.write_ue(23);                // 38  (000011000)      coefficient_log2_denom  23
    writer.write_n<uint8_t>(1, 2);      // 40  (01)             vdr_rpu_normalized_idc

    writer.write(false);                // 41  (0)              bl_video_full_range_flag
    writer.write_ue(2);                 // 44  (011)            bl_bit_depth_minus8
    writer.write_ue(2);                 // 47  (011)            el_bit_depth_minus8
    writer.write_ue(4);                 // 52  (00101)          vdr_bit_depth_minus8
    writer.write(false);                // 53  (0)              spatial_resampling_filter_flag
    writer.write_n<uint8_t>(0, 3);      // 56  (000)            reserved_zero_3bits

    writer.write(false);                // 57  (0)              el_spatial_resampling_filter_flag
    writer.write(true);                 // 58  (1)              disable_residual_flag

    writer.write(true);                 // 59  (1)              vdr_dm_metadata_present_flag
    writer.write(false);                // 60  (0)              use_prev_vdr_rpu_flag

    writer.write_ue(0);                 // 61  (1)              vdr_rpu_id

    writer.write_ue(0);                 // 62  (1)              mapping_color_space
    writer.write_ue(0);                 // 63  (1)              mapping_chroma_format_idc

    // 1.
    writer.write_ue(0);                 // 64  (1)              num_pivots_minus2
    writer.write_n<uint16_t>(0, 10);    // 74  (0000000000)     pivots[0]
    writer.write_n<uint16_t>(1023, 10); // 84  (1111111111)     pivots[1]

    // 2.
    writer.write_ue(0);                 // 85  (1)              num_pivots_minus2
    writer.write_n<uint16_t>(0, 10);    // 95  (0000000000)     pivots[0]
    writer.write_n<uint16_t>(1023, 10); // 105 (1111111111)     pivots[1]

    // 3.
    writer.write_ue(0);                 // 106 (1)              num_pivots_minus2
    writer.write_n<uint16_t>(0, 10);    // 116 (0000000000)     pivots[0] 
    writer.write_n<uint16_t>(1023, 10); // 126 (1111111111)     pivots[1]

    writer.write_ue(0);                 // 127 (1)              num_x_partitions_minus1
    writer.write_ue(0);                 // 128 (1)              num_y_partitions_minus1

    // 1.
    writer.write_ue(0);                 // 129 (1)              mapping_idc Polynomial = 0
    writer.write_ue(0);                 // 130 (1)              poly_order_minus1
    writer.write(false);                // 131 (0)              linear_interp_flag
    writer.write_se(0);                 // 132 (1)              poly_coef_int 0
    writer.write_n<uint64_t>(0, 23);    // 155 (00000000000000000000000)    poly_coef :: coefficient_log2_denom_length
    writer.write_se(1);                 // 158 (010)            poly_coef_int 1
    writer.write_n<uint64_t>(0, 23);    // 181 (00000000000000000000000)    poly_coef :: coefficient_log2_denom_length

    // 152 OK. 

    // 2.
    writer.write_ue(0);                 // 182 (1)              mapping_idc Polynomial = 0
    writer.write_ue(0);                 // 183 (1)              poly_order_minus1
    writer.write(false);                // 184 (0)              linear_interp_flag
    writer.write_se(0);                 // 185 (1)              poly_coef_int 0
    writer.write_n<uint64_t>(0, 23);    // 208 (00000000000000000000000)    poly_coef :: coefficient_log2_denom_length
    writer.write_se(1);                 // 211 (010)            poly_coef_int 1 
    writer.write_n<uint64_t>(0, 23);    // 234 (00000000000000000000000)    poly_coef :: coefficient_log2_denom_length

    // 3.
    writer.write_ue(0);                 // 235 (1)              mapping_idc Polynomial = 0
    writer.write_ue(0);                 // 236 (1)              poly_order_minus1
    writer.write(false);                // 237 (0)              linear_interp_flag
    writer.write_se(0);                 // 238 (1)              poly_coef_int 0 
    writer.write_n<uint64_t>(0, 23);    // 261 (00000000000000000000000)    poly_coef :: coefficient_log2_denom_length
    writer.write_se(1);                 // 264 (010)            poly_coef_int 1 
    writer.write_n<uint64_t>(0, 23);    // 287 (00000000000000000000000)    poly_coef :: coefficient_log2_denom_length

    writer.write_ue(0);                 // 288 (1)              affected_dm_metadata_id
    writer.write_ue(0);                 // 289 (1)              current_dm_metadata_id
    writer.write_ue(1);                 // 292 (010)            scene_refresh_flag

    writer.write_signed_n<int16_t>( 9574, 16);  // (0010010101101100)  ycc_to_rgb_coef0
    writer.write_signed_n<int16_t>(    0, 16);  // (0000000000000000)  ycc_to_rgb_coef1
    writer.write_signed_n<int16_t>(13802, 16);  // (0011010111101010)  ycc_to_rgb_coef2
    writer.write_signed_n<int16_t>( 9574, 16);  // (0010010101101100)  ycc_to_rgb_coef3
    writer.write_signed_n<int16_t>(-1540, 16);  // (1111101000000100)  ycc_to_rgb_coef4
    writer.write_signed_n<int16_t>(-5348, 16);  // (1110101100011100)  ycc_to_rgb_coef5
    writer.write_signed_n<int16_t>( 9574, 16);  // (0010010101101100)  ycc_to_rgb_coef6
    writer.write_signed_n<int16_t>(17610, 16);  // (0100010010111010)  ycc_to_rgb_coef7
    writer.write_signed_n<int16_t>(    0, 16);  // (0000000000000000)  ycc_to_rgb_coef8

    writer.write_n<uint32_t>( 16777216, 32);    // (00000001000000000000000000000000) ycc_to_rgb_offset0
    writer.write_n<uint32_t>(134217728, 32);    // (00001000000000000000000000000000) ycc_to_rgb_offset1
    writer.write_n<uint32_t>(134217728, 32);    // (000010000000000000000000000000000 ycc_to_rgb_offset2

    writer.write_signed_n<int16_t>( 7222, 16);  // (0001110000110110) rgb_to_lms_coef0
    writer.write_signed_n<int16_t>( 8771, 16);  // (0010001001000011) rgb_to_lms_coef1
    writer.write_signed_n<int16_t>(  390, 16);  // (0000000110000110) rgb_to_lms_coef2
    writer.write_signed_n<int16_t>( 2654, 16);  // (0000101001011110) rgb_to_lms_coef3
    writer.write_signed_n<int16_t>(12430, 16);  // (0011000010001110) rgb_to_lms_coef4
    writer.write_signed_n<int16_t>( 1300, 16);  // (0000010100010100) rgb_to_lms_coef5
    writer.write_signed_n<int16_t>(    0, 16);  // (0000000000000000) rgb_to_lms_coef6
    writer.write_signed_n<int16_t>(  422, 16);  // (0000000110100110) rgb_to_lms_coef7
    writer.write_signed_n<int16_t>(15962, 16);  // (0011111001011010) rgb_to_lms_coef8

    writer.write_n<uint16_t>(65535, 16);        // (1111111111111111) signal_eotf                           Ultra-HD Blu-ray
    writer.write_n<uint16_t>(0, 16);            // (0000000000000000) signal_eotf_param0                    Ultra-HD Blu-ray
    writer.write_n<uint16_t>(0, 16);            // (0000000000000000) signal_eotf_param1                    Ultra-HD Blu-ray
    writer.write_n<uint32_t>(0, 32);            // (00000000000000000000000000000000) signal_eotf_param2    Ultra-HD Blu-ray

    writer.write_n<uint8_t>(12, 5);             // (01100)  signal_bit_depth
    writer.write_n<uint8_t>(0, 2);              // (00)     signal_color_space       YCbCr
    writer.write_n<uint8_t>(0, 2);              // (00)     signal_chroma_format     4:2:0 ?
    writer.write_n<uint8_t>(1, 2);              // (01)     signal_full_range_flag   Full Range

    writer.write_n<uint16_t>(vdr_dm_data.min_pq, 12);
    writer.write_n<uint16_t>(vdr_dm_data.max_pq, 12);

    writer.write_n<uint16_t>(42, 10);           // (0000101010) source_diagonal (display diagonal in inches - TODO: Any effect?)

    writer.write_ue(3);                         // (00100)      num_ext_blocks (5 bits)

    writer.byte_align();                        // dm_alignment_zero_bit

    // L1 -----------
    writer.write_ue(5);                         // (00110)          length_bytes
    writer.write_n<uint8_t>(1, 8);              // (00000001)       level
    writer.write_n<uint16_t>(vdr_dm_data.min_pq, 12);
    writer.write_n<uint16_t>(vdr_dm_data.max_pq, 12);
    writer.write_n<uint16_t>(vdr_dm_data.avg_pq, 12);
    writer.write_n<uint8_t>(0, 4);              // (0000)           alignment of 4 bits.

    // L5 -----------
    writer.write_ue(7);                         // (0001000)        length_bytes
    writer.write_n<uint8_t>(5, 8);              // (00000101)       level
    writer.write_n<uint16_t>(0, 13);            // (0000000000000)  active_area_left_offset
    writer.write_n<uint16_t>(0, 13);            // (0000000000000)  active_area_right_offset
    writer.write_n<uint16_t>(0, 13);            // (0000000000000)  active_area_top_offset
    writer.write_n<uint16_t>(0, 13);            // (0000000000000)  active_area_bottom_offset
    writer.write_n<uint8_t>(0, 4);              // (0000)           alignment of 4 bits.

    // L6 -----------
    writer.write_ue(8);                         // (0001001)        length_bytes
    writer.write_n<uint8_t>(6, 8);              // (00000110)       level
    writer.write_n<uint16_t>(vdr_dm_data.max_display_mastering_luminance, 16);
    writer.write_n<uint16_t>(vdr_dm_data.min_display_mastering_luminance, 16);
    writer.write_n<uint16_t>(vdr_dm_data.max_content_light_level, 16);
    writer.write_n<uint16_t>(vdr_dm_data.max_frame_average_light_level, 16);

    writer.byte_align();                        // ext_dm_alignment_zero_bit
};

static uint32_t crc32_table[256] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

static uint32_t calc_crc32(const void *data, size_t data_size)
{
	const uint8_t *p = (uint8_t *)data;
	uint32_t crc = 0xFFFFFFFF;

	while (data_size) {
		crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *p) & 0xff];
		p++;
		data_size--;
	}

	return crc;
}

std::vector<uint8_t> create_rpu_nalu(VdrDmData& vdr_dm_data) {

  // Dolby Vision Profile 8.1 133 Bytes long.
  BitstreamIoWriter writer(133);

  writer.write_n<uint8_t>(0x19, 8);  // RPU prefix
  write_rpu(writer, vdr_dm_data);
  writer.write_n<uint32_t>(calc_crc32(writer.as_slice() + 1, writer.as_slice_size() - 1), 32);
  writer.write_n<uint8_t>(0x80, 8);  // FINAL_BYTE

  std::vector<uint8_t> out = writer.into_inner();

  HevcAddStartCodeEmulationPrevention3Byte(out);

  // Add NAL unit type
  out.insert(out.begin(), 0x01);
  out.insert(out.begin(), 0x7C);

  return out;
}