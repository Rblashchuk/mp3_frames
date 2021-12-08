#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "frames.h"

frames_list *new_frames_list() {
    frames_list *new_list = malloc(sizeof(frames_list));
    new_list->length = 0;
    new_list->vol = 9;
    new_list->val = malloc(new_list->vol * sizeof(frame));
    return new_list;
}

frames_list *push_into_framelist(frames_list *to_list, frame new_frame) {
    int len = to_list->length;
    if (len == to_list->vol) {
        to_list->vol = 2 * to_list->vol;
        to_list->val = realloc(to_list->val, to_list->vol * sizeof(frame));
    }
    to_list->val[len] = new_frame;
    to_list->length = len + 1;
    return to_list;
}

void parse_header_info(FILE *in, header *header_info) {
    (*header_info).sign[0] = fgetc(in);
    (*header_info).sign[1] = fgetc(in);
    (*header_info).sign[2] = fgetc(in);
    (*header_info).sign[3] = 0;
    (*header_info).version = fgetc(in);
    fgetc(in);
    short flagbyte = fgetc(in);
    flagbyte /= 16;

    (*header_info).flags.footer_present = flagbyte % 2;
    flagbyte /= 2;
    (*header_info).flags.exp_indicator = flagbyte % 2;
    flagbyte /= 2;
    (*header_info).flags.extended_header = flagbyte % 2;
    flagbyte /= 2;
    (*header_info).flags.unsynchronisation = flagbyte % 2;
    (*header_info).size = 0;

    unsigned pow = 2097152;
    (*header_info).size += fgetc(in) * pow;
    pow = pow / 128;
    (*header_info).size += fgetc(in) * pow;
    pow = pow / 128;
    (*header_info).size += fgetc(in) * pow;
    pow = pow / 128;
    (*header_info).size += fgetc(in) * pow;
}

frames_list *get_next(FILE *in, header *header_info, frames_list *frames) {
    unsigned byte_num = ((*header_info).version >= 3) ? 4 : 3;

    frame new_frame;

    for (int i = 0; i < byte_num; i++) {
        new_frame.tag[i] = fgetc(in);
    }

    new_frame.tag[byte_num] = 0;

    int frame_size = 0;
    unsigned pow = 2097152;
    frame_size += fgetc(in) * pow;
    pow = pow / 128;
    frame_size += fgetc(in) * pow;
    pow = pow / 128;
    frame_size += fgetc(in) * pow;
    pow = pow / 128;
    frame_size += fgetc(in) * pow;

    frame_size--;

    new_frame.flag[0] = fgetc(in);
    new_frame.flag[1] = fgetc(in);
    new_frame.encoding = fgetc(in);

    for (int i = 0; i < frame_size; i++) {
        new_frame.info[i] = fgetc(in);
    }

    if (!strcmp(new_frame.tag, "COMM")) {
        new_frame.info[3] = ' ';
    }

    new_frame.info[frame_size] = 0;

    return push_into_framelist(frames, new_frame);
}