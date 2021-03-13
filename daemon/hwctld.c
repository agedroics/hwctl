#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <hwctl/device.h>
#include <hwctl/loader.h>
#include <str_util.h>
#include <time_util.h>
#include <heap.h>
#include <profile.h>

#define PROFILES_DIR "/etc/hwctld/profiles/"

static void print_tree(unsigned depth, int connected) {
    if (depth == 1) {
        if (connected) {
            printf("+-");
        } else {
            printf("| ");
        }
    } else if (depth > 1) {
        printf("| ");
        for (unsigned i = 2; i < depth; ++i) {
            printf("  ");
        }
        print_tree(1, connected);
    }
}

static void print_dev(const struct hwctl_dev *dev, unsigned depth) {
    print_tree(depth, 0);
    putchar('\n');
    print_tree(depth, 1);
    printf("ID: %s\n", dev->get_id(dev));
    print_tree(depth, 0);
    printf("Description: %s\n", dev->get_desc(dev));
    print_tree(depth, 0);
    printf("Capabilities:");
    int first = 1;
    if (dev->read_sen) {
        printf(" read");
        first = 0;
    }
    if (dev->write_act) {
        if (!first) {
            putchar(',');
        } else {
            first = 0;
        }
        printf(" write");
    }
    if (first) {
        printf(" none");
    }
    putchar('\n');

    for (unsigned i = 0; i < vec_size(dev->subdevs); ++i) {
        print_dev(vec_at(dev->subdevs, i), depth + 1);
    }
}

static void det_devs(struct vec *devs) {
    for (unsigned i = 0; i < vec_size(get_hwctl_dev_dets()); ++i) {
        struct hwctl_dev_det *dev_det = vec_at(get_hwctl_dev_dets(), i);
        dev_det->det_devs(devs);
    }
}

static void destroy_devs(struct vec *devs) {
    for (unsigned i = 0; i < vec_size(devs); ++i) {
        hwctl_dev_destroy(vec_at(devs, i));
    }
}

static void list_devices() {
    struct vec *devs;
    vec_init(&devs, sizeof(struct hwctl_dev));
    det_devs(devs);

    for (unsigned i = 0; i < vec_size(devs); ++i) {
        print_dev(vec_at(devs, i), 0);
    }
    putchar('\n');

    destroy_devs(devs);
    vec_destroy(devs, 0);
}

struct profile_execution {
    struct profile profile;
    struct timespec scheduled_time;
};

static int profile_execution_cmp(const void *a, const void *b) {
    struct timespec *time1 = &((struct profile_execution*) a)->scheduled_time;
    struct timespec *time2 = &((struct profile_execution*) b)->scheduled_time;
    return time_cmp(time1, time2);
}

static void profile_executions_init(struct heap **profile_executions, const struct vec *devs) {
    heap_init(profile_executions, sizeof(struct profile_execution), &profile_execution_cmp);

    struct timespec cur_time;
    time_nanos(&cur_time);

    DIR *dir = opendir(PROFILES_DIR);
    if (dir != NULL) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type != DT_REG && ent->d_type != DT_LNK) {
                continue;
            }
            char *full_path = str_concat(2, PROFILES_DIR, ent->d_name);
            struct stat sb;
            if (stat(full_path, &sb) != -1 && S_ISREG(sb.st_mode)) {
                struct profile_execution profile_execution;
                int error = profile_open(&profile_execution.profile, full_path, devs);
                if (!error) {
                    profile_execution.scheduled_time = cur_time;
                    heap_push(*profile_executions, &profile_execution);
                }
            }
            free(full_path);
        }
        closedir(dir);
    }
}

static void start_daemon() {
    struct vec *devs;
    vec_init(&devs, sizeof(struct hwctl_dev));
    det_devs(devs);

    struct heap *profile_executions;
    profile_executions_init(&profile_executions, devs);

    struct profile_execution execution;
    struct timespec time;

    while (!heap_pop(profile_executions, &execution)) {
        int periodic = time_is_positive(&execution.profile.period);

        if (periodic) {
            time_nanos(&time);
            struct timespec wait_time = time_diff(&execution.scheduled_time, &time);
            if (time_is_positive(&wait_time)) {
                nanosleep(&wait_time, NULL);
                time_nanos(&time);
            }
        }

        int error = profile_exec(&execution.profile);
        if (error || !periodic) {
            profile_close(&execution.profile);
            continue;
        }

        time_add(&time, &execution.profile.period);
        execution.scheduled_time = time;
        heap_push(profile_executions, &execution);
    }

    heap_destroy(profile_executions);
    destroy_devs(devs);
    vec_destroy(devs, 0);
}

int main(int argc, char **argv) {
    if (argc == 1) {
        hwctl_load_plugins();
        start_daemon();
        hwctl_unload_plugins();
    } else if (argc == 2) {
        if (!strcmp(argv[1], "list")) {
            hwctl_load_plugins();
            list_devices();
            hwctl_unload_plugins();
        } else {
            fprintf(stderr, "%s: invalid option -- '%s'\n", argv[0], argv[1]);
            return 1;
        }
    } else {
        fprintf(stderr, "%s: too many arguments\n", argv[0]);
        return 1;
    }

    return 0;
}
