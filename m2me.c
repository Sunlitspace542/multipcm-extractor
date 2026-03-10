//
// Created by Jonas on 20.12.2015.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "byte_order.h"
#include "wave.h"

#define INSTRUMENTSIZE 12

const int INSTRUMENT_SIZE = INSTRUMENTSIZE;

const unsigned char invalid_row[INSTRUMENTSIZE] = {0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0xF0,0x00,0x0F,0x00};
const unsigned char end_row[INSTRUMENTSIZE] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

typedef struct {
    uint32_t start;
    unsigned short loop, end;
    uint8_t lfo, vib, ar, d1r, dl, d2r, rate_correction, rr, am;
} Instrument;

int check_row(FILE *f, int id) {
    fseek(f, id * INSTRUMENTSIZE, SEEK_SET);
    unsigned char buffer[INSTRUMENTSIZE];
    fread(buffer, INSTRUMENTSIZE, 1, f);
    if (memcmp(buffer, end_row, INSTRUMENTSIZE) == 0) {
        printf("%3d | Instrument table end hit!\n", id);
        return 1; // end
    }
    if (memcmp(buffer, invalid_row, INSTRUMENTSIZE) == 0) {
        printf("%3d | Invalid row!\n", id);
        return 2; // invalid
    }
    return 0; // valid
}

Instrument *make_instrument() {
    Instrument *i = malloc(sizeof(Instrument));
    return i;
}


void read_instrument(int id, FILE *f, Instrument *i) {
    fseek(f, id * INSTRUMENT_SIZE, SEEK_SET);
    int buffer = 0;
    fread(&i->start, 3, 1, f);
    i->start = (uint32_t) reverseInt24(i->start);
    fread(&i->loop, 2, 1, f);
    i->loop = (unsigned short) reverseShort(i->loop);
    fread(&buffer, 2, 1, f);
    i->end = (unsigned short) (buffer);
    i->end = (unsigned short) (~reverseShort(i->end) + 1);
    fread(&buffer, 1, 1, f);
    i->lfo = (uint8_t) ((buffer >> 3) & 0x07);
    i->vib = (uint8_t) (buffer & 0x07);
    fread(&buffer, 1, 1, f);
    i->ar = (uint8_t) ((buffer >> 4) & 0x0F);
    i->d1r = (uint8_t) (buffer & 0x0F);
    fread(&buffer, 1, 1, f);
    i->dl = (uint8_t) ((buffer >> 4) & 0x0F);
    i->d2r = (uint8_t) (buffer & 0x0F);
    fread(&buffer, 1, 1, f);
    i->rate_correction = (uint8_t) ((buffer >> 4) & 0x0F);
    i->rr = (uint8_t) (buffer & 0x0F);
    fread(&buffer, 1, 1, f);
    i->am = (uint8_t) (buffer & 0x07);
}

void write_instrument(int id, FILE *f, Instrument *i) {
    char str[20];
    sprintf(str, "%03i.wav", id);
    FILE *out = fopen(str, "wb");

    char *data = malloc(i->end);
    fseeko64(f, i->start, SEEK_SET);
    fread(data, i->end, 1, f);
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

int main(int argc, char *argv[]) {
    printf("SEGA MultiPCM Sample Extractor\n");
    if (!argv[1]) {
        printf("No file provided\n");
        return 1;
    }
    FILE *mpr;
    mpr = fopen(argv[1], "rb");
    if (mpr == NULL) {
        printf("Input file does not exist!\n");
        return 1;
    }

    Instrument *instr1 = make_instrument();
    printf("File: %s\n", argv[1]);
    printf(" id |   start  | loop  |  end  |lfo |vib | ar |d1r | dl |d2r | rc | rr |am\n");
    int sample_number = 0; // Output file's sample number
    for (int i = 0; ; i++) {
        int status = check_row(mpr, i);
        if (status == 1) break;
        if (status == 2) continue;
        read_instrument(i, mpr, instr1);
        printf("%3i | ", i);
        print_instrument(instr1);
        write_instrument(sample_number, mpr, instr1);
        sample_number++;
    }
    free(instr1);
    fclose(mpr);
    return 0;
}