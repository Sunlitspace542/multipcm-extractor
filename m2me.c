//
// Created by Jonas on 20.12.2015.
// Sunlit 10.03.2026 Skip invalid rows, read instrument table properly, make platform-independent
// Sunlit 10.03.2026 Add arguments, only read up to 256 instruments, fail on readfile() object number mismatch
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "byte_order.h"
#include "wave.h"

#define INSTRUMENTSIZE 12 // Length of an instrument table entry in bytes

const unsigned char invalid_row[INSTRUMENTSIZE] = {0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0xF0,0x00,0x0F,0x00}; // virtua racing has this but other games may not
const unsigned char end_row[INSTRUMENTSIZE] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

typedef struct {
    uint32_t start;
    unsigned short loop, end;
    uint8_t lfo, vib, ar, d1r, dl, d2r, rate_correction, rr, am;
} Instrument;

// Handle fread errors to make the compiler happy
void readfile(void *ptr, size_t size, size_t nobj, FILE *stream) {
    if (fread(ptr, size, nobj, stream) != nobj) {
        printf("Error: readfile() failed to fetch all objects\n");
        exit(1);
    }
}

// Try to open a file and handle errors
FILE *openfile(const char *filename, const char *mode) {
    if (fopen(filename, mode) == NULL){
        printf("Could not open file %s!\n", filename);
        exit(1);
    } return fopen(filename, mode);
}

int check_row(FILE *f, int id) {
    fseek(f, id * INSTRUMENTSIZE, SEEK_SET);
    unsigned char buffer[INSTRUMENTSIZE];
    readfile(buffer, INSTRUMENTSIZE, 1, f);
    // Check for instrument table end
    if (memcmp(buffer, end_row, INSTRUMENTSIZE) == 0) {
        printf("END |  Instrument table end\n");
        return 1; // end
    }
    // Check for invalid entry by known byte pattern
    if (memcmp(buffer, invalid_row, INSTRUMENTSIZE) == 0) {
        printf("%3d |  Invalid instrument\n", id);
        return 2; // invalid
    }
    // If sum of offset bytes >=760 or <=0, invalid
    int sum = buffer[0] + buffer[1] + buffer[2];
    if (sum >= 760 || sum <= 0) {
        printf("%3d |  Invalid instrument (offset sum check)\n", id);
        return 2; // invalid
    }
    return 0; // valid
}

Instrument *make_instrument() {
    Instrument *i = malloc(sizeof(Instrument));
    return i;
}


void read_instrument(int id, FILE *f, Instrument *i) {
    fseek(f, id * INSTRUMENTSIZE, SEEK_SET);
    int buffer = 0;
    readfile(&i->start, 3, 1, f);
    i->start = (uint32_t) reverseInt24(i->start);
    readfile(&i->loop, 2, 1, f);
    i->loop = (unsigned short) reverseShort(i->loop);
    readfile(&buffer, 2, 1, f);
    i->end = (unsigned short) (buffer);
    i->end = (unsigned short) (~reverseShort(i->end) + 1);
    readfile(&buffer, 1, 1, f);
    i->lfo = (uint8_t) ((buffer >> 3) & 0x07);
    i->vib = (uint8_t) (buffer & 0x07);
    readfile(&buffer, 1, 1, f);
    i->ar = (uint8_t) ((buffer >> 4) & 0x0F);
    i->d1r = (uint8_t) (buffer & 0x0F);
    readfile(&buffer, 1, 1, f);
    i->dl = (uint8_t) ((buffer >> 4) & 0x0F);
    i->d2r = (uint8_t) (buffer & 0x0F);
    readfile(&buffer, 1, 1, f);
    i->rate_correction = (uint8_t) ((buffer >> 4) & 0x0F);
    i->rr = (uint8_t) (buffer & 0x0F);
    readfile(&buffer, 1, 1, f);
    i->am = (uint8_t) (buffer & 0x07);
}

void write_instrument(int id, FILE *f, Instrument *i, char* outdir) {
    char str[PATH_MAX];
    if(!outdir) {
        sprintf(str, "%03i.wav", id);
    } else {
        sprintf(str, "%s/%03i.wav", outdir, id);
        mkdir(outdir);
    }
    FILE *out = fopen(str, "wb");
    
    char *data = malloc(i->end);
    fseeko(f, i->start, SEEK_SET);
    readfile(data, i->end, 1, f);
    for (int j = 0; j < i->end; j++) {
        if (data[j] >= 128) {
            data[j] += 127;
        } else {
            data[j] -= 128;
        }
    }
    
    WaveHeader *header = make_WaveHeader(1, 44100, 8, i->end);
    WavePadding *padding = make_WavePadding();
    WaveSmplChunk *smpl = make_WaveSmplChunk();
    WaveSampleLoop *loop = make_WaveSampleLoop(0, 0, i->loop, i->end, 0, 0);
    
    fwrite(header, sizeof(WaveHeader), 1, out);
    fwrite(data, i->end, 1, out);
    if (ftell(out) & 1) fwrite(padding, sizeof(char), 1, out);
    fwrite(smpl, sizeof(WaveSmplChunk), 1, out);
    fwrite(loop, sizeof(WaveSampleLoop), 1, out);
    
    free(header);
    free(data);
    free(padding);
    free(smpl);
    free(loop);
    
    fclose(out);
}

void print_instrument(Instrument *i) {
    printf("%8i | %5i | %5i | %2i | %2i | %2i | %2i | %2i | %2i | %2i | %2i | %2i\n",
        i->start,
        i->loop,
        i->end,
        i->lfo,
        i->vib,
        i->ar,
        i->d1r,
        i->dl,
        i->d2r,
        i->rate_correction,
        i->rr,
        i->am
    );
}

// Possible command line arguments
const char *arguments[] =  {
    "-help",    // 0 - help text
    "-i",       // 1 - input file
    "-o",       // 2 - output dir
    "-t"        // 3 - test mode
};

int main(int argc, char *argv[]) {
    char infile[PATH_MAX];
    int testmode = 0;
    printf("SEGA MultiPCM Sample Extractor\n");
    
    // Parse Arguments
    if (argc == 1) {
        printf("No arguments specified.\nRun m2me -help for help.\n");
        return 1;
    }
    
    if (argc > 5) {
        printf("Too many arguments specified.\nRun m2me -help for help.\n");
        return 1;
    }
    
    // handle "-i" switch
    if (argc >= 2 && (!strcmp(argv[1],arguments[1]))) {
        if (!argv[2]) {
            printf("No input file provided\n");
            return 1;
        }
        strcpy(infile,argv[2]);
    } else { // drag-and-drop
        strcpy(infile,argv[1]);
        argv[4] = '\0'; // force argv[4] to be NUL
    }
    
    // handle "-o" switch
    if (argc >= 4 && (!strcmp(argv[3],arguments[2]))) {
        if (!argv[4]) {
            printf("No output directory provided\n");
            return 1;
        }
    }
    
    // handle "-t" switch
    if (argc >= 2 && (!strcmp(argv[1],arguments[3]))) {
        testmode = 1;
        printf("TEST MODE\n");
        if (!argv[2]) {
            printf("No input file provided\n");
            return 1;
        }
        strcpy(infile,argv[2]);
    }
    
    // handle "-help" switch
    if (!strcmp(argv[1],arguments[0])) {
        printf(
            "Extracts samples from a SEGA MultiPCM sample ROM.\n"
            "Usage: m2me [options]\n"
            "Or just drag and drop!\n"
            "Options:\n"
            "-i [infile]    Specify input ROM (Required!)\n"
            "-o [outdir]    Specify output directory (Optional, cannot be used with -t)\n"
            "-t [infile]    Test mode (No files written)\n"
            "-help          This text\n"
        );
        return 1;
    }
    
    // Everything Else
    FILE *mpr;
    mpr = fopen(infile, "rb");
    if (mpr == NULL) {
        printf("Bad command or filename\nRun m2me -help for help.\n");
        return 1;
    }
    
    Instrument *instr1 = make_instrument();
    printf("File: %s\n", infile);
    printf(" id |   start  | loop  |  end  |lfo |vib | ar |d1r | dl |d2r | rc | rr |am\n");
    int sample_number = 0; // Output file's sample number
    for (int i = 0; ; i++) {
        //if (i > 256){printf("Warning: More than 256 instruments read! Stopping...\n"); break;}
        int status = check_row(mpr, i);
        if (status == 1) break; // hit end of instrument table
        if (status == 2) continue; // instrument is invalid
        read_instrument(i, mpr, instr1);
        printf("%3i | ", i);
        print_instrument(instr1);
        if (!testmode) write_instrument(sample_number, mpr, instr1, argv[4]);
        sample_number++;
    }
    free(instr1);
    fclose(mpr);
    if (!testmode) printf("Extracted %d samples\n", sample_number);
    return 0;
}