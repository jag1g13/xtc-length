#include <stdio.h>
#include <inttypes.h>

/*
 * Program for finding the number of frames and atoms in a GROMACS XTC file.
 * Inspired by a post on the gmx-developers mailing list in 2012:
 * https://mailman-1.sys.kth.se/pipermail/gromacs.org_gmx-developers/2012-June/005937.html
 */

uint32_t u4_from_buffer(const uint8_t *b){
    return b[0] << 24 | b[1] << 16 | b[2] << 8 | b[3] << 0;
}

int get_xtc_num_frames(const char *filename, int *nframes, int *natoms){
    FILE *xtc = fopen(filename, "rb");
    if(!xtc) return -1;

    uint8_t header[92];

    *nframes = 1;
    while(fread(header, 92, 1, xtc)){                       // Loop over frames
        *natoms = u4_from_buffer(header+4);
        uint32_t frame_size = u4_from_buffer(header+88);
        uint32_t skip = (frame_size+3) & ~((uint32_t)4);    // Round up to 4 bytes
        fseek(xtc, skip, SEEK_CUR);                         // Skip to next header
        (*nframes)++;
    }

    fclose(xtc);
    return 0;
}

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
