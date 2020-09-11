#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <hwctl/device.h>
#include <profile.h>

extern int errno;

struct profile {
    unsigned period;
    struct hwctl_dev *dev_in;
    struct hwctl_dev *dev_out;
    struct vec *pairs;
};

static char *read_string(FILE *file) {
    struct vec *buf;
    vec_init(&buf, sizeof(char));

    if (feof(file)) {
        goto end;
    }

    char c;
    do {
        c = fgetc(file);
        if (feof(file)) {
            goto end;
        }
    } while (isspace(c));

    do {
        *((char*) vec_push_back(buf)) = c;
        c = fgetc(file);
        if (feof(file)) {
            goto end;
        }
    } while (!isspace(c));

end:
    *((char*) vec_push_back(buf)) = 0;
    char *str = vec_data(buf);
    vec_destroy(buf, 1);
    return str;
}

static struct hwctl_dev *read_dev(FILE *file, struct vec *devs) {
    struct hwctl_dev *result = NULL;
    for (;;) {
        char *str = read_string(file);
        int found = 0;
        if (str[0] && strcmp(str, ";")) {
            struct vec *dev_list = result ? result->subdevs : devs;
            for (unsigned i = 0; i < vec_size(dev_list); ++i) {
                struct hwctl_dev *dev = ((struct hwctl_dev *) vec_data(dev_list)) + i;
                if (!strcmp(str, dev->get_id(dev))) {
                    result = dev;
                    found = 1;
                    break;
                }
            }
        }
        free(str);
        if (!found) {
            break;
        }
    }
    return result;
}

static int compare_pairs(const void *a, const void *b) {
    double *pair1 = (double*) a;
    double *pair2 = (double*) b;
    return pair1[0] > pair2[0] ? 1 : (pair1[0] == pair2[0] ? 0 : -1);
}

struct profile *profile_open(char *path, struct vec *devs) {
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Failed to open profile %s: %s\n", path, strerror(errno));
        return NULL;
    }

    struct profile *profile = NULL;

    unsigned period;
    if (!fscanf(file, "%u", &period)) {
        fprintf(stderr, "Failed to read period from profile %s\n", path);
        goto end;
    }

    struct hwctl_dev *dev_in = read_dev(file, devs);
    if (!dev_in) {
        fprintf(stderr, "Error in profile %s: input device not found\n", path);
        goto end;
    } else if (!dev_in->read_sen) {
        fprintf(stderr, "Error in profile %s: input device does not have capability 'read'\n", path);
        goto end;
    }

    struct hwctl_dev *dev_out = read_dev(file, devs);
    if (!dev_out) {
        fprintf(stderr, "Error in profile %s: output device not found\n", path);
        goto end;
    } else if (!dev_out->write_act) {
        fprintf(stderr, "Error in profile %s: output device does not have capability 'write'\n", path);
        goto end;
    }

    struct vec *pairs;
    vec_init(&pairs, sizeof(double) * 2);
    int i = 1;
    while (!feof(file)) {
        double values[2];
        if (!fscanf(file, "%lf", values) || !fscanf(file, "%lf", values + 1)) {
            fprintf(stderr, "Error in profile %s: invalid value pair %d\n", path, i);
            vec_destroy(pairs, 0);
            goto end;
        }
        ++i;
    }

    qsort(vec_data(pairs), vec_size(pairs), sizeof(double) * 2, &compare_pairs);

    profile = malloc(sizeof(struct profile));
    profile->period = period;
    profile->dev_in = dev_in;
    profile->dev_out = dev_out;
    profile->pairs = pairs;
end:
    fclose(file);
    return profile;
}

void profile_close(struct profile *profile) {
    vec_destroy(profile->pairs, 0);
    free(profile);
}

void profile_exec(struct profile *profile) {
    for (;;) {
        double value_in = profile->dev_in->read_sen(profile->dev_in);
        double value_out;

        if (!vec_size(profile->pairs)) {
            value_out = value_in;
        } else {
            double *first_pair = ((double **) vec_data(profile->pairs))[0];
            if (value_in < first_pair[0]) {
                value_out = first_pair[1];
            } else {
                double *last_pair = ((double **) vec_data(profile->pairs))[vec_size(profile->pairs) - 1];
                if (value_in > last_pair[0]) {
                    value_out = last_pair[1];
                } else {
                    for (unsigned i = 0; i < vec_size(profile->pairs); ++i) {
                        double *pair = ((double *) vec_data(profile->pairs)) + i;
                        double x0 = pair[0];
                        if (value_in >= x0) {
                            double *next_pair = ((double *) vec_data(profile->pairs)) + i + 1;
                            double y0 = pair[1];
                            double dx = next_pair[0] - x0;
                            double dy = next_pair[1] - y0;
                            value_out = y0 + (value_in - x0) * dy / dx;
                            break;
                        }
                    }
                }
            }
        }

        profile->dev_out->write_act(profile->dev_out, value_out);
        usleep(profile->period * 1000);
    }
}