/*{REPLACEMEWITHLICENSE}*/
#include "small_zip.h"
#include <stddef.h> // offsetof()
#ifdef __cplusplus
extern "C" {
#endif

// TODO: More bounds & overflow checking

static inline uint8_t get_bit(uint8_t byte, uint8_t offset) {
	return byte & (1 << (offset-1));
}

static inline uint8_t next_bit(const unsigned char * data, uint32_t * byte_offset, uint8_t * bit_offset)
{
	uint8_t tmp = *bit_offset;
	uint8_t bit = get_bit(data[*byte_offset], tmp++);
	if (!(tmp &= 7)) *byte_offset += 1;
	*bit_offset = tmp;
	return bit;
}

static unsigned scan_for_eocd(small_zip_ctx * ctx) {
	small_zip_eocd eocd;
	const unsigned char * input = ctx->input;
	uint32_t input_len = ctx->input_len;
	// start at first possible eocd-sig position.
	const unsigned char * p = input + input_len - sizeof(small_zip_eocd) + 3;
	do {
		// little endian eocd sig 0x06054b50
		if (*p-- != 0x06) continue;
		if (*p-- != 0x05) continue;
		if (*p-- != 0x4b) continue;
		if (*p-- != 0x50) continue;
		memcpy(&eocd, p + 1, sizeof(eocd));
		if (small_zip_from_le16(&eocd.comment_len) != (p + sizeof(eocd) + 1 - input - input_len))
			continue;
		// no intention of supporting multi-disk zips...
		if (small_zip_from_le16(&eocd.number_of_disks) || small_zip_from_le16(&eocd.disk_with_cd_start))
			return 0;
		// also no intention of dealing with empty archives...
		if (!(ctx->file_count = small_zip_from_le16(&eocd.cd_records_on_disk)))
			return 0;
		ctx->cd_offset = small_zip_from_le32(&eocd.cd_offset);
		if (!ctx->cd_offset || ctx->cd_offset > input_len)
			return 0;
		return 1;
	} while (p >= input && (p-input) > sizeof(small_zip_eocd));
	return 0;
}

// TODO: handled cached files in ctx
unsigned small_zip_file(small_zip_ctx * ctx, small_zip_file_ctx * file_ctx, uint16_t file_index) {
	if (!ctx || !file_ctx) return 0;
	uint32_t input_len = ctx->input_len;
	const small_zip_cdfh * cdfh = (const small_zip_cdfh *)(ctx->input + ctx->cd_offset);
	while (file_index--) {
		uint32_t offset = small_zip_from_le16(&cdfh->filename_len)
		                + small_zip_from_le16(&cdfh->extra_field_len)
		                + small_zip_from_le16(&cdfh->file_comment_len);
		cdfh = (const small_zip_cdfh *)((const unsigned char *)cdfh + offset);
	}
	file_ctx->input = ctx->input;
	file_ctx->cdfh = cdfh;
	file_ctx->input_len = ctx->input_len;
	return 1;
}

void small_zip_file_sizes(const small_zip_file_ctx * file, uint32_t * compressed, uint32_t * decompressed) {
	if (compressed) *compressed = small_zip_from_le32(&file->cdfh->compressed_size);
	if (decompressed) *decompressed = small_zip_from_le32(&file->cdfh->decompressed_size);
}

const char * small_zip_file_name(const small_zip_file_ctx * file, uint16_t * filename_len) {
	*filename_len = small_zip_from_le16(&file->cdfh->filename_len);
	return (const char *)file->cdfh + sizeof(small_zip_cdfh);
}

unsigned small_zip_decompress_init(const small_zip_file_ctx * file, small_zip_decompress_ctx * ctx) {
	const small_zip_cdfh * cdfh = file->cdfh;
	uint16_t compression_method = cdfh->compression_method;
	if (compression_method != 0 && compression_method != 8) return 0;
	ctx->compression_method = compression_method;
	ctx->byte_offset = 0;
	ctx->bit_offset = 0;
	uint32_t offset = small_zip_from_le16(&cdfh->filename_len)
	                + small_zip_from_le16(&cdfh->extra_field_len)
	                + small_zip_from_le16(&cdfh->file_comment_len);
	const unsigned char * data = file->input + small_zip_from_le32(&cdfh->lfh_offset) + offset;
	if (small_zip_from_le16(&cdfh->flags) & 4) { // need to deal with the data descriptor...
		uint32_t sig;
		memcpy(&sig, data, 4);
		if (small_zip_from_le32(&sig) == 0x08074b50)
			data += 4;
		data += sizeof(small_zip_data_descriptor);
	}
	ctx->data = data;
	return 1;
}

uint32_t small_zip_decompress(small_zip_decompress_ctx * ctx, void * void_buf, uint32_t bufsize) {
	if (ctx->compression_method == 0) { // stored
		uint32_t remaining = ctx->compressed_size - ctx->byte_offset;
		uint32_t count = remaining > bufsize ? bufsize : remaining;
		if (count == -1) count = -1 - 1; // well this should be impossible lol...
		memcpy(void_buf, ctx->data + ctx->byte_offset, count);
		ctx->byte_offset += count;
		return count;
	} 
	if (ctx->compression_method != 8) return 0; // method 8 is deflate
	
	uint32_t count = 0;
	const unsigned char * data = ctx->data;
	uint32_t byte_offset = ctx->byte_offset;
	uint8_t bit_offset = ctx->bit_offset;

	while (1) {
		uint8_t bit = next_bit(data, &byte_offset, &bit_offset);
		if (bit == 0) break; // last block bit
		uint8_t block_type =
			(next_bit(data, &byte_offset, &bit_offset) << 1) |
			(next_bit(data, &byte_offset, &bit_offset));
		switch (block_type) {
		case 0: // stored block
		{
			// skip bits till next byte
			if (bit_offset != 7) byte_offset += 1;
			bit_offset = 0;
			break;
		}
		case 1: // fixed huffman codes for literal and distance codes
		case 2: // dynamic huffman codes
		case 3: // error in compressed data
			return -1;
		}
	}

	return count;
}

unsigned small_zip_init(small_zip_ctx * ctx_out, const void * input, uint32_t input_len) {
	small_zip_ctx ctx;
	if (!ctx_out || !input || input_len < 100) return 0;
	ctx.input = (const unsigned char *)input;
	ctx.input_len = input_len;
	//ctx.cached_file = NULL;
	ctx.file_index = 0;
	ctx.cache_file = 0;
	if (!scan_for_eocd(&ctx)) return 0;
	*ctx_out = ctx;
	return 1;
}

#ifdef __cplusplus
}
#endif
