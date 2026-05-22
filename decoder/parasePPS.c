#include "parasePPS.h"


static void bs_init(bitstream_t *bs, u_char *data, int size)
{
	bs->data = data;
	bs->size = size;
	bs->bit_pos = 0;
}

static int bs_eof(bitstream_t *bs)
{
	return (bs->bit_pos >= bs->size * 8);
}

static unsigned int bs_read_bits(bitstream_t *bs, int n)
{
	unsigned int val = 0;
	int i;
	for (i = 0; i < n; i++) {
		if (bs_eof(bs)) return 0;
		int byte_idx = bs->bit_pos / 8;
		int bit_idx  = 7 - (bs->bit_pos % 8);
		val = (val << 1) | ((bs->data[byte_idx] >> bit_idx) & 1);
		bs->bit_pos++;
	}
	return val;
}

static unsigned int bs_read_bit(bitstream_t *bs)
{
	return bs_read_bits(bs, 1);
}

/* unsigned exp-golomb: 0th order */
static unsigned int bs_read_ue(bitstream_t *bs)
{
	int leading_zeros = 0;
	while (!bs_eof(bs) && bs_read_bit(bs) == 0)
		leading_zeros++;
	if (leading_zeros == 0) return 0;
	unsigned int val = bs_read_bits(bs, leading_zeros);
	return (1 << leading_zeros) - 1 + val;
}

/* signed exp-golomb */
static int bs_read_se(bitstream_t *bs)
{
	unsigned int val = bs_read_ue(bs);
	if (val & 1)
		return (int)((val + 1) >> 1);
	else
		return -(int)(val >> 1);
}

/*
 * Remove emulation prevention bytes (0x00 0x00 0x03 -> 0x00 0x00)
 * Returns the new length after removal
 */
static int remove_emulation_prevention(u_char *src, int len, u_char *dst)
{
	int dst_len = 0;
	int i;
	for (i = 0; i < len; i++) {
		if (i + 2 < len && src[i] == 0x00 && src[i+1] == 0x00 && src[i+2] == 0x03) {
			dst[dst_len++] = 0x00;
			dst[dst_len++] = 0x00;
			i += 2; // skip 0x03
		} else {
			dst[dst_len++] = src[i];
		}
	}
	return dst_len;
}

/*
 * Parse HEVC PPS using proper bitstream exp-golomb decoding
 * Reference: ITU-T H.265 Section 7.3.2.3
 */
int parase_pps(u_char *buf, int len, u_char *rbsp_buf,HEVCPPS_t hevcPPS)
{
	
	if (len < 4) return -1;

	/* Ensure rbsp_buf is large enough */
	if (rbsp_buf == NULL || 256 < len) {
	    return -1;
	}

	int rbsp_len;
	bitstream_t bs;

	/* Remove emulation prevention bytes, skip 2-byte NAL header */
	rbsp_len = remove_emulation_prevention(buf + 2, len - 2, rbsp_buf);
	bs_init(&bs, rbsp_buf, rbsp_len);

	/* pps_pic_parameter_set_id */
	unsigned int pps_id = bs_read_ue(&bs);
	/* pps_seq_parameter_set_id */
	hevcPPS->sps_id = bs_read_ue(&bs);

	/* dependent_slice_segments_enabled_flag */
	hevcPPS->dependent_slice_segments_enabled_flag = bs_read_bit(&bs);
	/* output_flag_present_flag */
	hevcPPS->output_flag_present_flag = bs_read_bit(&bs);
	/* num_extra_slice_header_bits */
	hevcPPS->num_extra_slice_header_bits = bs_read_bits(&bs, 3);
	/* sign_data_hiding_enabled_flag */
	hevcPPS->sign_data_hiding_flag = bs_read_bit(&bs);
	/* cabac_init_present_flag */
	hevcPPS->cabac_init_present_flag = bs_read_bit(&bs);

	/* num_ref_idx_l0_default_active_minus1 */
	hevcPPS->num_ref_idx_l0_default_active = bs_read_ue(&bs) + 1;
	/* num_ref_idx_l1_default_active_minus1 */
	hevcPPS->num_ref_idx_l1_default_active = bs_read_ue(&bs) + 1;

	/* init_qp_minus26 */
	hevcPPS->pic_init_qp_minus26 = bs_read_se(&bs);

	/* constrained_intra_pred_flag */
	hevcPPS->constrained_intra_pred_flag = bs_read_bit(&bs);
	/* transform_skip_enabled_flag */
	hevcPPS->transform_skip_enabled_flag = bs_read_bit(&bs);

	/* cu_qp_delta_enabled_flag */
	hevcPPS->cu_qp_delta_enabled_flag = bs_read_bit(&bs);
	if (hevcPPS->cu_qp_delta_enabled_flag) {
		/* diff_cu_qp_delta_depth */
		hevcPPS->diff_cu_qp_delta_depth = bs_read_ue(&bs);
	}

	/* pps_cb_qp_offset */
	hevcPPS->cb_qp_offset = bs_read_se(&bs);
	/* pps_cr_qp_offset */
	hevcPPS->cr_qp_offset = bs_read_se(&bs);

	/* pps_slice_chroma_qp_offsets_present_flag */
	hevcPPS->pic_slice_level_chroma_qp_offsets_present_flag = bs_read_bit(&bs);
	/* weighted_pred_flag */
	hevcPPS->weighted_pred_flag = bs_read_bit(&bs);
	/* weighted_bipred_flag */
	hevcPPS->weighted_bipred_flag = bs_read_bit(&bs);
	/* transquant_bypass_enabled_flag */
	hevcPPS->transquant_bypass_enable_flag = bs_read_bit(&bs);

	/* tiles_enabled_flag */
	hevcPPS->tiles_enabled_flag = bs_read_bit(&bs);
	/* entropy_coding_sync_enabled_flag */
	hevcPPS->entropy_coding_sync_enabled_flag = bs_read_bit(&bs);

	if (hevcPPS->tiles_enabled_flag) {
		/* num_tile_columns_minus1 */
		hevcPPS->num_tile_columns = bs_read_ue(&bs) + 1;
		/* num_tile_rows_minus1 */
		hevcPPS->num_tile_rows = bs_read_ue(&bs) + 1;

		//printf("tiles_enabled: columns=%d rows=%d\r\n",
		//       hevcPPS->num_tile_columns, hevcPPS->num_tile_rows);
	} else {
		hevcPPS->num_tile_columns = 1;
		hevcPPS->num_tile_rows = 1;
	}

	/* Reset slice counters when PPS is parsed */
	(void)pps_id;
	return (hevcPPS->tiles_enabled_flag) ? 1 : 0;
}