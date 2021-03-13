#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hwctl/device.h>
#include <time_util.h>
#include <profile.h>

extern int errno;

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
        vec_push_back(buf, &c);
        c = fgetc(file);
        if (feof(file)) {
            goto end;
        }
    } while (!isspace(c));

end:
    vec_push_back(buf, NULL);
    char *str = vec_data(buf);
    vec_destroy(buf, 1);
    return str;
}

static struct hwctl_dev *read_dev(FILE *file, const struct vec *devs) {
    struct hwctl_dev *result = NULL;
    for (;;) {
        char *str = read_string(file);
        int found = 0;
        if (str[0] && strcmp(str, ";")) {
            const struct vec *dev_list = result ? result->subdevs : devs;
            for (unsigned i = 0; i < vec_size(dev_list); ++i) {
                struct hwctl_dev *dev = vec_at(dev_list, i);
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

int profile_open(struct profile *profile, const char *path, const struct vec *devs) {
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Failed to open profile %s: %s\n", path, strerror(errno));
        return 1;
    }

    unsigned period;
    if (!fscanf(file, "%u", &period)) {
        fprintf(stderr, "Failed to read period from profile %s\n", path);
        fclose(file);
        return 1;
    }

    struct hwctl_dev *dev_in = read_dev(file, devs);
    if (!dev_in) {
        fprintf(stderr, "Error in profile %s: input device not found\n", path);
        fclose(file);
        return 1;
    } else if (!dev_in->read_sen) {
        fprintf(stderr, "Error in profile %s: input device does not have capability 'read'\n", path);
        fclose(file);
        return 1;
    }

    struct hwctl_dev *dev_out = read_dev(file, devs);
    if (!dev_out) {
        fprintf(stderr, "Error in profile %s: output device not found\n", path);
        fclose(file);
        return 1;
    } else if (!dev_out->write_act) {
        fprintf(stderr, "Error in profile %s: output device does not have capability 'write'\n", path);
        fclose(file);
        return 1;
    }

    struct vec *pairs;
    vec_init(&pairs, sizeof(double) << 1);
    while (!feof(file)) {
        double *values = vec_push_back(pairs, NULL);
        if (!fscanf(file, "%lf", values) || !fscanf(file, "%lf", values + 1)) {
            fprintf(stderr, "Error in profile %s: invalid value pair %lu\n", path, vec_index_of(pairs, values) + 1);
            vec_destroy(pairs, 0);
            fclose(file);
            return 1;
        }
    }

    vec_sort(pairs, &compare_pairs);

    time_from_millis(&profile->period, period);
    profile->dev_in = dev_in;
    profile->dev_out = dev_out;
    profile->pairs = pairs;

    fclose(file);
    return 0;
}

void profile_close(struct profile *profile) {
    vec_destroy(profile->pairs, 0);
}

int profile_exec(const struct profile *profile) {
    double value_in;
    int error = profile->dev_in->read_sen(profile->dev_in, &value_in);
    if (error) {
        return 1;
    }
    double value_out;
    int value_out_exists = 0;

    if (!vec_size(profile->pairs)) {
        value_out = value_in;
        value_out_exists = 1;
    } else {
        double *first_pair = vec_head(profile->pairs);
        if (value_in < first_pair[0]) {
            value_out = first_pair[1];
            value_out_exists = 1;
        } else {
            double *last_pair = vec_last(profile->pairs);
            if (value_in > last_pair[0]) {
                value_out = last_pair[1];
                value_out_exists = 1;
            } else {
                for (unsigned i = 0; i < vec_size(profile->pairs); ++i) {
                    double *pair = vec_at(profile->pairs, i);
                    if (value_in <= pair[0]) {
                        double *prev_pair = vec_at(profile->pairs, i - 1);
                        double dx = pair[0] - prev_pair[0];
                        double dy = pair[1] - prev_pair[1];
                        if (dx == 0) {
                            value_out = pair[1];
                        } else {
                            value_out = prev_pair[1] + (value_in - prev_pair[0]) * dy / dx;
                        }
                        value_out_exists = 1;
                        break;
                    }
                }
            }
        }
    }

    if (value_out_exists) {
        error = profile->dev_out->write_act(profile->dev_out, value_out);
        if (error) {
            return 1;
        }
    }

    if (!profile->period.tv_nsec && !profile->period.tv_sec) {
        return 1;
    }

    return 0;
}
