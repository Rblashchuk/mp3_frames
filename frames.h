#ifndef PROG_LAB_4_FRAMES_H
#define PROG_LAB_4_FRAMES_H

typedef struct {
    short unsynchronisation;
    short extended_header;
    short exp_indicator;
    short footer_present;
} flags;

typedef struct {
    char sign[4];
    short version;
    flags flags;
    unsigned size;
} header;

typedef struct {
    char tag[5];
    char encoding;
    char info[1000];
    char flag[2];
} frame;

typedef struct {
    frame *val;
    int length;
    int vol;
} frames_list;

void parse_header_info(FILE *in, header *header_info);

frames_list *new_frames_list();

frames_list *push_into_framelist(frames_list *to_list, frame new_frame);

frames_list *get_next(FILE *in, header *header_info, frames_list *frames);

#endif //PROG_LAB_4_FRAMES_H