#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <stdint.h>

#define CHUNK_SIZE 4096
#define LINE_WIDTH 80

// CRC32 table
uint32_t crc32_table[256];
void init_icrc32() {
    uint32_t crc;
    for (int i = 0; i < 256; i++) {
        crc = i;
        for (int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        crc32_table[i] = crc;
    }
}
uint32_t icrc32(const unsigned char *buf, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++)
        crc = (crc >> 8) ^ crc32_table[(crc ^ buf[i]) & 0xFF];
    return ~crc;
}

// ASCII85 encode 4 bytes → 5 chars
void ascii85_encode_block(uint32_t val, char out[5]) {
    for (int i = 4; i >= 0; i--) {
        out[i] = (val % 85) + 33;
        val /= 85;
    }
}

char* ascii85_encode(const unsigned char *input, size_t len, size_t *out_len) {
    size_t enc_len = ((len + 3) / 4) * 5;
    char *encoded = malloc(enc_len + 1);
    if (!encoded) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i += 4) {
    	uint32_t val = 0;
    	int real_bytes = (i + 4 <= len) ? 4 : (len - i);

 	for (int k = 0; k < 4; k++) {
       		val <<= 8;
        	if (k < real_bytes)
            		val |= input[i + k];
        	else
            		val |= 0x00; // pad with zero
    	}

    	char block[5];
    	ascii85_encode_block(val, block);

    	for (int k = 0; k < 5; k++) {
        	encoded[j++] = block[k];
    	}
    }

    encoded[j] = '\0';
    *out_len = j;
    return encoded;
}

// Zlib compression
unsigned char* compress_data(const unsigned char *data, size_t len, size_t *out_len) {
    uLong bound = compressBound(len);
    unsigned char *out = malloc(bound);
    if (!out) return NULL;

    if (compress(out, &bound, data, len) != Z_OK) {
        free(out);
        return NULL;
    }

    *out_len = bound;
    return out;
}

void write_scroll(const char *encoded, size_t len, const char *outname, size_t input_len) {
    FILE *out = fopen(outname, "w");
    if (!out) {
        perror("fopen");
        return;
    }
    fprintf(out, "00000: SIZE=%zu\n",input_len);

    int line_num = 1;
    for (size_t i = 0; i < len; i += LINE_WIDTH) {
        size_t chunk_len = (i + LINE_WIDTH > len) ? (len - i) : LINE_WIDTH;
        const char *chunk = encoded + i;
        uint32_t hash = icrc32((const unsigned char*)chunk, chunk_len);

        fprintf(out, "%05d: ", line_num++);
        fwrite(chunk, 1, chunk_len, out);
        fprintf(out, "  [c:%04X]\n", (hash & 0xFFFF));
    }

    fclose(out);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s input_file output_scroll.txt\n", argv[0]);
        return 1;
    }

    init_icrc32();

    // Read input file
    FILE *in = fopen(argv[1], "rb");
    if (!in) {
        perror("fopen");
        return 1;
    }

    fseek(in, 0, SEEK_END);
    long fsize = ftell(in);
    fseek(in, 0, SEEK_SET);

    unsigned char *data = malloc(fsize);
    fread(data, 1, fsize, in);
    fclose(in);

    // Compress
    size_t comp_len;
    unsigned char *compressed = compress_data(data, fsize, &comp_len);
    free(data);

    if (!compressed) {
        fprintf(stderr, "Compression failed.\n");
        return 1;
    }

    // Encode
    size_t enc_len;
    char *encoded = ascii85_encode(compressed, comp_len, &enc_len);
    free(compressed);

    if (!encoded) {
        fprintf(stderr, "Encoding failed.\n");
        return 1;
    }

    // Write scroll
    write_scroll(encoded, enc_len, argv[2], fsize);
    free(encoded);

    printf("Scroll written to %s\n", argv[2]);
    return 0;
}
