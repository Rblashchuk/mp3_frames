#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "frames.h"

void print_frames(frames_list *frames) {
    printf("%d frames:\n", frames->length - 1);
    for (int i = 0; i < frames->length; i++) {
        if (frames->val[i].tag[0] && frames->val[i].info[0]) {

            printf("%d) %s: %s\n", i + 1, frames->val[i].tag, frames->val[i].info);
        }
    }
}

char *get_frame(frames_list *frames, char *tag) {
    for (int i = 0; i < frames->length; i++) {
        if (!strcmp(frames->val[i].tag, tag)) {
            return frames->val[i].info;
        }
    }
    printf("No such frame found\n");
    return NULL;
}

void set_frame(header *header_info, frames_list *frames, char *tag, char *set_val) {
    for (int i = 0; i < frames->length; i++) {
        if (!strcmp(frames->val[i].tag, tag)) {
            (*header_info).size += strlen(set_val);
            (*header_info).size -= strlen(frames->val[i].info);

            int size = strlen(set_val) + 1;
            sprintf(frames->val[i].info, "%s", set_val);
            frames->val[i].info[size] = '\0';

            return;
        }
    }
    printf("No such frame found\n");
}

void new_into_file(char *out_file, header header_info, frames_list *frames) {
    unsigned byte_num;
    if (header_info.version >= 3){
        byte_num = 3;
    }
    else{
        byte_num = 4;
    }
    FILE *out = fopen(out_file, "r+b");

    if (out == NULL) {
        printf("error occurred\n");
        return;
    }

    unsigned pow = 16;
    char curr;

    fprintf(out, "%s", header_info.sign);

    fprintf(out, "%c%c", header_info.version, 0);

    char flagbyte = 0;

    flagbyte += pow * header_info.flags.unsynchronisation;
    pow = 2 * pow;
    flagbyte += pow * header_info.flags.extended_header;
    pow = 2 * pow;
    flagbyte += pow * header_info.flags.exp_indicator;
    pow = 2 * pow;
    flagbyte += pow * header_info.flags.footer_present;
    fputc(flagbyte, out);
    pow = 2097152;

    for (int i = 0; i < 4; i++) {
        curr = header_info.size / pow;
        header_info.size -= curr;
        fputc(curr, out);
        pow /= 128;
    }

    for (int i = 0; i < frames->length; i++) {
        for (int j = 0; j < byte_num; j++) {
            fputc(frames->val[i].tag[j], out);
        }

        unsigned frame_size = strlen(frames->val[i].info) + 1;
        pow = 2097152;

        for (int j = 0; j < 4; j++) {
            curr = frame_size / pow;
            frame_size -= curr * pow;
            fputc(curr, out);
            pow /= 128;
        }

        fputc(frames->val[i].flag[0], out);
        fputc(frames->val[i].flag[1], out);
        fputc(frames->val[i].encoding, out);
        frame_size = strlen(frames->val[i].info);
        if (!strcmp(frames->val[i].tag, "COMM")) {
            frames->val[i].info[3] = 0;
        }

        for (int j = 0; j < frame_size; j++) {
            fputc(frames->val[i].info[j], out);
        }

    }
    fclose(out);
}

int main(int argc, char **argv) {
    char *filename = argv[argc - 1];
    frames_list *frames = new_frames_list();
    header header_info;
    FILE *in = fopen(filename, "r");
    parse_header_info(in, &header_info);
    while (ftell(in) < header_info.size) {
        frames = get_next(in, &header_info, frames);
    }

    fclose(in);
    short was_edited = 0;
    for (int i = 1; i < argc - 1; i++) {
        char *command = argv[i];
        if (!strcmp(command, "--show")) {
            print_frames(frames);
        } else {
            if (strlen(command) >= 6) {
                command[5] = 0;
            }
            if (!strcmp(command, "--set")) {
                command[5] = '=';
                char *next_command = argv[++i];
                char *prop_name = strchr(command, '=') + 1;
                char *prop_value = strrchr(next_command, '=') + 1;
                set_frame(&header_info, frames, prop_name, prop_value);
                was_edited = 1;
            } else if (!strcmp(command, "--get")) {
                command[5] = '=';
                char *prop_name = strchr(command, '=') + 1;
                printf("%s\n", get_frame(frames, prop_name));
            } else {
                printf("error\n");
            }
        }
    }
    if (was_edited) {
        new_into_file(filename, header_info, frames);
    }
    free(frames);
    frames = NULL;
    return 0;
}