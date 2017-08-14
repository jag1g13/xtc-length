#include <stdio.h>
#include <stdint.h>
#include <locale.h>
#include <string.h>
#include <stdbool.h>

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

int get_xtc_num_frames(const char *filename, int *nframes, int *natoms, float *psec, bool quiet){
    uint8_t header[92];
    uint32_t frame_size;
    uint32_t skip;
    uint32_t ps_tmp;
    long size = file_size(filename);
    double avg_frame_size = 0.;
    int est_nframes = 0;

    FILE *xtc = fopen(filename, "rb");
    if(!xtc) return -1;


    *nframes = 0;
    while(fread(header, 92, 1, xtc)){                       /* Loop over frames */
        (*nframes)++;
        *natoms = u4_from_buffer(header+4);
        frame_size = u4_from_buffer(header+88);    /* Read frame size from header */
        skip = (frame_size+3) & ~((uint32_t)3);    /* Round up to 4 byte boundary */
        avg_frame_size += (skip - avg_frame_size + 92) / *nframes;
        if(!quiet && *nframes % 10 == 0){
            est_nframes = (int)(size / avg_frame_size);
            ps_tmp = u4_from_buffer(header+12);
            memcpy(psec, &ps_tmp, 4);
            *psec *= (double)est_nframes / *nframes;
            printf("\rEstimated %'d frames (%'d ns) of %'d atoms. - %d%%",
                    est_nframes, (int)(*psec/1000), *natoms, (int)(100. * *nframes) / est_nframes);
            fflush(stdout);
        }
        fseeko(xtc, skip, SEEK_CUR);                         /* Skip to next header */
    }
    ps_tmp = u4_from_buffer(header+12);
    memcpy(psec, &ps_tmp, 4);

    fclose(xtc);
    return 0;
}

int main(const int argc, const char *argv[]){
    const char *help_text = "Count number of frames and simulation time of GROMACS XTC files\n"
                            "Usage: xtc-length [-q] xtc [xtc]...\n\n"
                            "Default behaviour is to provide running estimate of file length\n"
                            "This can be suppressed using the '-q' flag\n";
    int i;
    bool quiet = false;
    int nframes, natoms;
    float psec;

    setlocale(LC_ALL, "");

    if(argc < 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        printf("%s", help_text);
        return 0;
    }
    if(argc >= 2 && !strcmp(argv[1], "-q")) quiet = true;

    for(i = quiet ? 2 : 1; i < argc; i++){
        printf("%s\n", argv[i]);
        if(get_xtc_num_frames(argv[i], &nframes, &natoms, &psec, quiet)){
            printf("ERROR: Error reading XTC file\n");
            return -1;
        }

        if(!quiet) printf("\r");
        printf("Trajectory contains %'d frames (%'.2f ns) of %'d atoms.\n\n", nframes, psec / 1000, natoms);
    }
    return 0;
}
