#define _GNU_SOURCE

#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <hwctl/device.h>
#include <hwctl/loader.h>
#include <str_util.h>
#include <profile.h>

#define PROFILES_DIR "/etc/hwctld/profiles/"

static struct profile *next_profile;
static pthread_mutex_t next_profile_get_mutex;
static pthread_mutex_t next_profile_set_mutex;

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
        struct hwctl_dev *subdev = ((struct hwctl_dev*) vec_data(dev->subdevs)) + i;
        print_dev(subdev, depth + 1);
    }
}

static void det_devs(struct vec *devs) {
    for (unsigned i = 0; i < vec_size(get_hwctl_dev_dets()); ++i) {
        struct hwctl_dev_det *dev_det = ((struct hwctl_dev_det*) vec_data(get_hwctl_dev_dets())) + i;
        dev_det->det_devs(devs);
    }
}

static void destroy_devs(struct vec *devs) {
    for (unsigned i = 0; i < vec_size(devs); ++i) {
        struct hwctl_dev *dev = ((struct hwctl_dev*) vec_data(devs)) + i;
        hwctl_dev_destroy(dev);
    }
}

static void list_devices() {
    struct vec *devs;
    vec_init(&devs, sizeof(struct hwctl_dev));
    det_devs(devs);

    for (unsigned i = 0; i < vec_size(devs); ++i) {
        struct hwctl_dev *dev = ((struct hwctl_dev*) vec_data(devs)) + i;
        print_dev(dev, 0);
    }
    putchar('\n');

    destroy_devs(devs);
    vec_destroy(devs, 0);
}

static void *thread_runner(void *arg) {
    struct profile *profile = (struct profile*) arg;
    while (!profile_marked_for_stop(profile)) {
        pthread_mutex_lock(&next_profile_set_mutex);
        if (profile_marked_for_stop(profile)) {
            next_profile = NULL;
        } else {
            next_profile = profile;
        }
        pthread_mutex_unlock(&next_profile_get_mutex);
        struct timespec period = profile_get_period(profile);
        nanosleep(&period, NULL);
    }
    return NULL;
}

static void start_daemon() {
    struct vec *devs;
    vec_init(&devs, sizeof(struct hwctl_dev));
    det_devs(devs);

    struct vec *profiles;
    vec_init(&profiles, sizeof(struct profile*));

    pthread_mutex_init(&next_profile_get_mutex, NULL);
    pthread_mutex_init(&next_profile_set_mutex, NULL);

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
                struct profile *profile = profile_open(full_path, devs);
                if (profile) {
                    *((struct profile**) vec_push_back(profiles)) = profile;
                    pthread_t thread;
                    pthread_create(&thread, NULL, &thread_runner, profile);
                    char thread_name[16];
                    strncpy(thread_name, ent->d_name, 15);
                    pthread_setname_np(thread, thread_name);
                }
            }
            free(full_path);
        }
        closedir(dir);
    }

    while (vec_size(profiles)) {
        pthread_mutex_lock(&next_profile_get_mutex);
        struct profile *profile = next_profile;
        pthread_mutex_unlock(&next_profile_set_mutex);

        if (profile && profile_exec(profile)) {
            profile_mark_for_stop(profile);
            for (unsigned i = 0; i < vec_size(profiles); ++i) {
                struct profile *item = ((struct profile**) vec_data(profiles))[i];
                if (item == profile) {
                    vec_remove(profiles, item);
                    profile_close(profile);
                    break;
                }
            }
        }
    }

    vec_destroy(profiles, 0);
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
