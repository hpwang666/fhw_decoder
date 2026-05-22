#ifndef _PARASE_H
#define _PARASE_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/*
 * Bitstream reader for exp-golomb and fixed-length code parsing
 */


typedef struct HEVCPPS_st *HEVCPPS_t;

typedef struct {
	u_char *data;
	int     size;    // total bytes
	int     bit_pos; // current bit position
} bitstream_t;


struct HEVCPPS_st {
    unsigned int sps_id; ///< seq_parameter_set_id

    uint8_t sign_data_hiding_flag;

    uint8_t cabac_init_present_flag;

    int num_ref_idx_l0_default_active; ///< num_ref_idx_l0_default_active_minus1 + 1
    int num_ref_idx_l1_default_active; ///< num_ref_idx_l1_default_active_minus1 + 1
    int pic_init_qp_minus26;

    uint8_t constrained_intra_pred_flag;
    uint8_t transform_skip_enabled_flag;

    uint8_t cu_qp_delta_enabled_flag;
    int diff_cu_qp_delta_depth;

    int cb_qp_offset;
    int cr_qp_offset;
    uint8_t pic_slice_level_chroma_qp_offsets_present_flag;
    uint8_t weighted_pred_flag;
    uint8_t weighted_bipred_flag;
    uint8_t output_flag_present_flag;
    uint8_t transquant_bypass_enable_flag;

    uint8_t dependent_slice_segments_enabled_flag;
    uint8_t tiles_enabled_flag;
    uint8_t entropy_coding_sync_enabled_flag;

    int num_tile_columns;   ///< num_tile_columns_minus1 + 1
    int num_tile_rows;      ///< num_tile_rows_minus1 + 1
    uint8_t uniform_spacing_flag;
    uint8_t loop_filter_across_tiles_enabled_flag;

    uint8_t seq_loop_filter_across_slices_enabled_flag;

    uint8_t deblocking_filter_control_present_flag;
    uint8_t deblocking_filter_override_enabled_flag;
    uint8_t m_inferScalingListFlag;
    int m_scalingListRefLayerId; 
    uint8_t disable_dbf;
    int8_t beta_offset;    ///< beta_offset_div2 * 2
    int8_t tc_offset;      ///< tc_offset_div2 * 2

    uint8_t scaling_list_data_present_flag;
    //ScalingList scaling_list;

    uint8_t lists_modification_present_flag;
    int log2_parallel_merge_level; ///< log2_parallel_merge_level_minus2 + 2
    int num_extra_slice_header_bits;
    uint8_t slice_header_extension_present_flag;
    uint8_t log2_max_transform_skip_block_size;
    uint8_t cross_component_prediction_enabled_flag;
    uint8_t chroma_qp_offset_list_enabled_flag;
    uint8_t diff_cu_chroma_qp_offset_depth;
    uint8_t chroma_qp_offset_list_len_minus1;
    int8_t  cb_qp_offset_list[5];
    int8_t  cr_qp_offset_list[5];
    uint8_t log2_sao_offset_scale_luma;
    uint8_t log2_sao_offset_scale_chroma;


    // Inferred parameters
    unsigned int *column_width;  ///< ColumnWidth
    unsigned int *row_height;    ///< RowHeight
    int *col_idxX;

    int *ctb_addr_rs_to_ts; ///< CtbAddrRSToTS
    int *ctb_addr_ts_to_rs; ///< CtbAddrTSToRS
    int *ctb_row_to_rs;
    int *tile_id;           ///< TileId
    int *tile_width;           ///< TileWidth
    int *tile_pos_rs;       ///< TilePosRS
    int *min_tb_addr_zs;    ///< MinTbAddrZS
    int *min_tb_addr_zs_tab;///< MinTbAddrZS

};


int parase_pps(u_char *buf, int len, u_char *rbsp_buf,HEVCPPS_t hevcPPS);


#endif
