/*{REPLACEMEWITHLICENSE}*/
#pragma once

/*
bool extract_file()
{
	unsigned char buf[2048];
	uint32_t bytes_decompressed;
	small_zip_ctx ctx;
	small_zip_file_ctx file_ctx;
	small_zip_decompress_ctx decompress_ctx;

	if (!small_zip_init(&ctx, GENERATED_Changelog_zip, GENERATED_Changelog_zip_size)) return false;
	if (!small_zip_file(&ctx, &file_ctx, 0)) return false; // grab the first file....
	if (!small_zip_decompress_init(&file_ctx, &decompress_ctx)) return false;
	while ((bytes_decompressed = small_zip_decompress(&decompress_ctx, buf, 2048))) {
		if (bytes_decompressed == -1) // error in compressed data
			return false;
#ifdef _WIN32
		DWORD bytes_written;
		if (!WriteFile(file_handle, buf, bytes_decompressed, &bytes_written, NULL))
			return false;
#else

#endif
	}
}
*/

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <string.h> // memcpy()

// BIG NOTE:
// uint16_t's and uint32_t's in small_zip_cdfh are in little-endian order...
// don't use directly...

#ifdef _MSC_VER
#define __attribute__(...)
#pragma pack(push, 1)
#endif

// End of central directory record
// PACKED
typedef struct __attribute__((__packed__)) {
	uint32_t sig;
	uint16_t number_of_disks;
	uint16_t disk_with_cd_start;
	uint16_t cd_records_on_disk;
	uint16_t total_cd_records;
	uint32_t cd_size;
	uint32_t cd_offset;
	uint16_t comment_len;
} small_zip_eocd;

// without sig
// PACKED
typedef struct __attribute__((__packed__)) {
	uint32_t crc32;
	uint32_t compressed_size;
	uint32_t decompressed_size;
} small_zip_data_descriptor;

// local file header
// PACKED
typedef struct __attribute__((__packed__)) {
	uint32_t sig;
	uint16_t min_version;
	uint16_t flags;
	uint16_t compression_method;
	uint16_t last_modified_time;
	uint16_t last_modified_date;
	uint32_t crc32;
	uint32_t compressed_size;
	uint32_t decompressed_size;
	uint16_t filename_len;
	uint16_t extra_field_len;
	// file name
	// extra field
} small_zip_lfh;

// central directory file header
// PACKED
typedef struct __attribute__((__packed__)) {
	uint32_t sig;
	uint16_t version_made_by;
	uint16_t min_version;
	uint16_t flags;
	uint16_t compression_method;
	uint16_t last_modified_time;
	uint16_t last_modified_date;
	uint32_t crc32;
	uint32_t compressed_size;
	uint32_t decompressed_size;
	uint16_t filename_len;
	uint16_t extra_field_len;
	uint16_t file_comment_len;
	uint16_t file_disk;
	uint16_t file_attr_internal;
	uint32_t file_attr_external;
	uint32_t lfh_offset;
	// file name
	// extra field
	// file comment
} small_zip_cdfh;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef struct {
	const small_zip_cdfh * cdfh;
	const unsigned char * input;
	uint32_t input_len;
} small_zip_file_ctx;

typedef struct {
	const unsigned char * data;
	uint32_t byte_offset;
	uint32_t compressed_size;
	uint16_t compression_method;
	uint16_t block_offset; // how many bytes into a stored block we are...
	uint8_t bit_offset;
} small_zip_decompress_ctx;

typedef struct {
	const unsigned char * input;
	uint32_t input_len;
	uint16_t file_count;
	uint16_t cd_offset;
	small_zip_file_ctx cached_file;
	uint16_t file_index; // aka cdfh index
	uint16_t cache_file; // set to 1 to cache cdfh... is 0 by default
} small_zip_ctx;

inline unsigned small_zip_is_be(void) {
	const uint16_t x = 0x0100;
	return *(const uint8_t *)&x;
}

inline uint16_t small_zip_from_le16(const uint16_t * input) {
	uint16_t x;
	memcpy(&x, input, sizeof(x));
	if (small_zip_is_be()) return (x << 8) | (x >> 8);
	return x;
}

inline uint32_t small_zip_from_le32(const uint32_t * input) {
	uint32_t x;
	memcpy(&x, input, sizeof(x));
	uint16_t a = x >> 16, b = x & 0xFFFF;
	return small_zip_from_le16(&a) | small_zip_from_le16(&b);
}

unsigned small_zip_init(small_zip_ctx * ctx, const void * input, uint32_t input_len);

unsigned small_zip_file(small_zip_ctx * ctx, small_zip_file_ctx * file_ctx, uint16_t file_index);

void small_zip_file_sizes(const small_zip_file_ctx * file, uint32_t * compressed, uint32_t * decompressed);

const char * small_zip_file_name(const small_zip_file_ctx * file, uint16_t * filename_len);

unsigned small_zip_decompress_init(const small_zip_file_ctx * file, small_zip_decompress_ctx * ctx);

// returns 0 when everything has been decompressed
// on error returns (uint32_t)-1
// on success returns number of bytes written
uint32_t small_zip_decompress(small_zip_decompress_ctx * ctx, void * void_buf, uint32_t bufsize);

#ifdef __cplusplus
}
#endif
