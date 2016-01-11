#include <stdio.h>
#include <stdint.h>
#include <locale.h>
#include <string.h>

#include <sys/stat.h>

/*
 * Program for finding the number of frames and atoms in a GROMACS XTC file.
 * Inspired by a post on the gmx-developers mailing list in 2012:
 * https://mailman-1.sys.kth.se/pipermail/gromacs.org_gmx-developers/2012-June/005937.html
 */

long file_size(const char *filename){
    struct stat buffer;
    int rc = stat(filename, &buffer);
    return rc == 0 ? buffer.st_size : -1;
}

uint32_t u4_from_buffer(const uint8_t *b){
    return b[0] << 24 | b[1] << 16 | b[2] << 8 | b[3] << 0;
}

void print_header(const uint8_t header[92]){
    printf("%4d", u4_from_buffer(header+0));
    printf("%8d", u4_from_buffer(header+4));
    printf("%12d", u4_from_buffer(header+8));
    printf("\n");
}

int get_xtc_num_frames(const char *filename, int *nframes, int *natoms, float *psec){
    long size = file_size(filename);

    FILE *xtc = fopen(filename, "rb");
    if(!xtc) return -1;

    uint8_t header[92];

    *nframes = 0;
    double avg_frame_size = 0.;
    int est_nframes = 0;
    while(fread(header, 92, 1, xtc)){                       // Loop over frames
        //print_header(header);
        (*nframes)++;
        *natoms = u4_from_buffer(header+4);
        uint32_t frame_size = u4_from_buffer(header+88);    // Read frame size from header
        uint32_t skip = (frame_size+3) & ~((uint32_t)3);    // Round up to 4 bytes
        avg_frame_size += (skip - avg_frame_size + 92) / *nframes;
        if(*nframes % 10 == 0){
            est_nframes = size / avg_frame_size;
            uint32_t ps_tmp = u4_from_buffer(header+12);
            memcpy(psec, &ps_tmp, 4);
            *psec *= (double)est_nframes / *nframes;
            printf("\rEstimated %'d frames (%'d ns) of %'d atoms. - %d%%",
                    est_nframes, (int)(*psec/1000), *natoms, (int)(100. * *nframes) / est_nframes);
            fflush(stdout);
        }
        fseeko(xtc, skip, SEEK_CUR);                         // Skip to next header
    }
    uint32_t ps_tmp = u4_from_buffer(header+12);
    memcpy(psec, &ps_tmp, 4);

    fclose(xtc);
    return 0;
}

int main(const int argc, const char *argv[]){
    setlocale(LC_ALL, "");

    if(argc < 2){
        printf("ERROR: Incorrect usage - must give input filename\n");
        return -1;
    }

    int nframes, natoms;
    float psec;
    if(get_xtc_num_frames(argv[1], &nframes, &natoms, &psec)){
        printf("ERROR: Error reading XTC file\n");
        return -1;
    }

    printf("\rTrajectory contains %'d frames (%'.2f ns) of %'d atoms.\n", nframes, psec/1000, natoms);
    return 0;
}
