
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <stdint.h>

#define MAX_LINE_LEN 256

// Base85 decode 5 chars -> 4 bytes
int ascii85_decode_block(const char in[5], uint32_t *out) {
    uint32_t val = 0;
    for (int i = 0; i < 5; i++) {
        if (in[i] < 33 || in[i] > 117) return -1;
        val = val * 85 + (in[i] - 33);
    }
    *out = val;
    return 0;
}

unsigned char* ascii85_decode(const char *input, size_t len, size_t *out_len) {
    size_t blocks = len / 5;
    size_t alloc_len = blocks * 4;
    unsigned char *decoded = malloc(alloc_len);
    if (!decoded) return NULL;

    for (size_t i = 0; i < blocks; i++) {
        uint32_t val;
        if (ascii85_decode_block(&input[i * 5], &val) != 0) {
            free(decoded);
            return NULL;
        }
        decoded[i * 4 + 0] = (val >> 24) & 0xFF;
        decoded[i * 4 + 1] = (val >> 16) & 0xFF;
        decoded[i * 4 + 2] = (val >> 8) & 0xFF;
        decoded[i * 4 + 3] = val & 0xFF;
    }

    *out_len = alloc_len;
    return decoded;
}

unsigned char* decompress_data(const unsigned char *data, size_t len, size_t *out_len) {
    size_t alloc = len * 20;
    unsigned char *out = malloc(alloc);
    if (!out) return NULL;

    z_stream strm = {0};
    strm.next_in = (Bytef*)data;
    strm.avail_in = len;
    strm.next_out = out;
    strm.avail_out = alloc;

    if (inflateInit(&strm) != Z_OK) {
        free(out);
        return NULL;
    }

    int res = inflate(&strm, Z_FINISH);
    if (res != Z_STREAM_END) {
        inflateEnd(&strm);
        free(out);
        return NULL;
    }

    *out_len = strm.total_out;
    inflateEnd(&strm);
    return out;
}

char* extract_base85(const char *filename, size_t *out_len, size_t *original_size) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return NULL;
    }

    char *buffer = malloc(1024 * 1024);
    size_t total = 0;
    char line[MAX_LINE_LEN];

    while (fgets(line, MAX_LINE_LEN, f)) {
        if (strncmp(line, "00000: SIZE=", 12) == 0) {
            *original_size = strtoul(line + 12, NULL, 10);
            continue;
        }

        char *colon = strchr(line, ':');
        if (!colon) continue;
        char *start = colon + 2;
        char *end = strstr(start, "  [c:");
        if (!end) continue;

        size_t len = end - start;
        memcpy(buffer + total, start, len);
        total += len;
    }

    fclose(f);
    *out_len = total;
    return buffer;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s scroll.txt output_file\n", argv[0]);
        return 1;
    }

    size_t enc_len, declared_size = 0;
    char *base85 = extract_base85(argv[1], &enc_len, &declared_size);
    if (!base85) {
        fprintf(stderr, "Failed to extract Base85 data.\n");
        return 1;
    }

    size_t bin_len;
    unsigned char *binary = ascii85_decode(base85, enc_len, &bin_len);
    free(base85);
    if (!binary) {
        fprintf(stderr, "Base85 decoding failed.\n");
        return 1;
    }

    size_t out_len;
    unsigned char *decompressed = decompress_data(binary, bin_len, &out_len);
    free(binary);
    if (!decompressed) {
        fprintf(stderr, "Decompression failed.\n");
        return 1;
    }

    FILE *out = fopen(argv[2], "wb");
    if (!out) {
        perror("fopen");
        return 1;
    }

    size_t final_len = (declared_size && declared_size < out_len) ? declared_size : out_len;
    fwrite(decompressed, 1, final_len, out);
    fclose(out);
    free(decompressed);

    printf("Decoded and decompressed to %s (%zu bytes)\n", argv[2], final_len);
    return 0;
}
