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
		uint32_t another_overflow_check = ctx->cd_offset + sizeof(small_zip_cdfh);
		if (another_overflow_check > input_len || another_overflow_check < ctx->cd_offset)
			return 0;
		return 1;
	} while (p >= input && (p-input) > sizeof(small_zip_eocd));
	return 0;
}

unsigned small_zip_file(small_zip_ctx * ctx, small_zip_file_ctx * file_ctx, uint16_t file_index) {
	const small_zip_cdfh * cdfh;
	uint32_t input_len, i;
	if (!ctx || !file_ctx) return 0;
	if (file_index && file_index-1 > ctx->file_count) return 0;
	if (ctx->cache_file && ctx->file_index <= file_index && ctx->cached_file.cdfh) {
		if (ctx->file_index == file_index) {
			*file_ctx = ctx->cached_file;
			return 1;
		}
		i = file_index - ctx->file_index;
		cdfh = ctx->cached_file.cdfh;
	} else {
		i = file_index;
		cdfh = (const small_zip_cdfh *)(ctx->input + ctx->cd_offset);
	}
	input_len = ctx->input_len;
	while (i--) {
		uint32_t offset = small_zip_from_le16(&cdfh->filename_len)
		                + small_zip_from_le16(&cdfh->extra_field_len)
		                + small_zip_from_le16(&cdfh->file_comment_len);
		// TODO: Can an optimizing compiler remove the overflow check?
		if (offset > input_len || offset + input_len < offset)
			return 0;
		cdfh = (const small_zip_cdfh *)((const unsigned char *)cdfh + offset);
	}
	small_zip_file_ctx asdf = {0};
	asdf.cdfh = cdfh;
	asdf.input = ctx->input;
	asdf.input_len = ctx->input_len;
	*file_ctx = asdf;
	if (ctx->cache_file) {
		ctx->file_index = file_index;
		ctx->cached_file = asdf;
	}
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
	if (!file || !ctx) return 0;
	const small_zip_cdfh * cdfh = file->cdfh;
	uint16_t compression_method = small_zip_from_le16(&cdfh->compression_method);
	if (compression_method != 0 && compression_method != 8) return 0;
	small_zip_decompress_ctx decctx = {0};
	decctx.compression_method = compression_method;
	decctx.byte_offset = 0;
	decctx.bit_offset = 0;
	decctx.compressed_size = small_zip_from_le32(&cdfh->compressed_size);
	uint32_t offset = small_zip_from_le32(&cdfh->lfh_offset);
	uint32_t input_len = file->input_len;
	if (offset > input_len) return 0;
	uint16_t flen = small_zip_from_le16(&cdfh->filename_len);
	if (offset+flen > input_len || offset+flen < offset) return 0;
	offset += flen;
	uint16_t elen = small_zip_from_le16(&cdfh->extra_field_len);
	if (offset+elen > input_len || offset+elen < offset) return 0;
	offset += elen;
	if (offset > input_len || offset + decctx.compressed_size < offset) return 0;
	const unsigned char * data = file->input + offset;
	if (small_zip_from_le16(&cdfh->flags) & 4) { // need to deal with the data descriptor...
		uint32_t sig;
		memcpy(&sig, data, 4);
		uint32_t x = (small_zip_from_le32(&sig) == 0x08074b50) ? 4 : 0;
		x += sizeof(small_zip_data_descriptor);
		if (offset + x > input_len || offset + x < offset) return 0;
		data += x;
	}
	decctx.data = data;
	*ctx = decctx;
	return 1;
}

// returns the number of bytes decompressed
// returns 0 when done decompressing
// returns -1 on error
uint32_t small_zip_decompress(small_zip_decompress_ctx * ctx, void * void_buf, uint32_t bufsize) {
	if (!ctx || !void_buf || !bufsize) return 0;
	if (ctx->compression_method == 0) { // stored
		uint32_t remaining = ctx->compressed_size - ctx->byte_offset;
		if (!remaining) return 0;
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
	uint16_t block_offset = ctx->block_offset;

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
		default:
		//case 3: // error in compressed data
			return -1;
		}
	}

	return count;
}

unsigned small_zip_init(small_zip_ctx * ctx_out, const void * input, uint32_t input_len) {
	small_zip_ctx ctx;
	// how about a reasonable input_len...
	if (!ctx_out || !input || input_len < 100 || input_len > 0x7FFFFFFF) return 0;
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
