#include <stdio.h>
#include <inttypes.h>
#include <string.h>

int get_xtc_num_frames(const char *filename, int *nframes, int *natoms);
uint32_t u4_from_buffer(const uint8_t *b);
float float_from_buffer(const uint8_t *b);
long file_size(FILE *f);
void print_header(const uint8_t *header);
int round_up(int numToRound, int multiple);

int main(const int argc, const char *argv[]){
    if(argc < 2){
        printf("ERROR: Incorrect usage - must give input filename\n");
        return -1;
    }

    int nframes, natoms;
    if(get_xtc_num_frames(argv[1], &nframes, &natoms)){
        printf("ERROR: Error reading XTC file\n");
        return -1;
    }

    printf("Trajectory contains %d frames of %d atoms.\n", nframes, natoms);
    return 0;
}

int get_xtc_num_frames(const char *filename, int *nframes, int *natoms){
    FILE *xtc = fopen(filename, "rb");
    if(!xtc) return -1;

    uint8_t header[92];
    if(!fread(header, 92, 1, xtc)) return -1;
    uint32_t magic = u4_from_buffer(header);
    *natoms = u4_from_buffer(header+4);
    uint32_t frame_size = u4_from_buffer(header+88);

    *nframes = 1;
    while(1){
        int skip = round_up(frame_size, 4);
        fseek(xtc, skip, SEEK_CUR);
        if(!fread(header, 92, 1, xtc)) break;
//        print_header(header);
        frame_size = u4_from_buffer(header+88);
        (*nframes)++;
    }

    fclose(xtc);
    return 0;
}

void print_header(const uint8_t *header){
    printf("%4d ", u4_from_buffer(header));
    printf("%4d ", u4_from_buffer(header + 4));
    printf("%8d ", u4_from_buffer(header + 8));
    printf("%6.3f ", float_from_buffer(header + 16));
    printf("%6.3f ", float_from_buffer(header + 32));
    printf("%6.3f ", float_from_buffer(header + 48));
    printf("%6d ", u4_from_buffer(header + 88));
    printf("\n");
}

uint32_t u4_from_buffer(const uint8_t *b){
    return b[0] << 24 | b[1] << 16 | b[2] << 8 | b[3] << 0;
}

float float_from_buffer(const uint8_t *b){
    uint32_t tmp = u4_from_buffer(b);
    float result;
    memcpy(&result, &tmp, sizeof(tmp));
    return result;
}

int round_up(const int numToRound, const int multiple){
    if(multiple == 0) return numToRound;

    int remainder = numToRound % multiple;
    if(remainder == 0) return numToRound;
    return numToRound + multiple - remainder;
}
