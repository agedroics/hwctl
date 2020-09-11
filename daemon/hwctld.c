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

static void print_dev(struct hwctl_dev *dev, unsigned depth) {
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
    profile_exec(profile);
    return NULL;
}

static void start_daemon() {
    struct vec *devs;
    vec_init(&devs, sizeof(struct hwctl_dev));
    det_devs(devs);

    struct vec *profiles;
    vec_init(&profiles, sizeof(struct profile*));

    struct vec *threads;
    vec_init(&threads, sizeof(pthread_t));

    DIR *dir = opendir(PROFILES_DIR);
    if (dir != NULL) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type != DT_REG) {
                continue;
            }
            char *full_path = str_concat(2, PROFILES_DIR, ent->d_name);
            struct stat sb;
            if (stat(full_path, &sb) != -1 && (sb.st_mode & S_IFMT) == S_IFREG) {
                struct profile *profile = profile_open(full_path, devs);
                if (profile) {
                    *((struct profile**) vec_push_back(profiles)) = profile;
                    pthread_t thread;
                    pthread_create(&thread, NULL, &thread_runner, profile);
                    *((pthread_t*) vec_push_back(threads)) = thread;
                }
            }
            free(full_path);
        }
        closedir(dir);
    }

    for (unsigned i = 0; i < vec_size(profiles); ++i) {
        struct profile *profile = ((struct profile**) vec_data(profiles))[i];
        pthread_t thread = ((pthread_t*) vec_data(threads))[i];
        pthread_join(thread, NULL);
        profile_close(profile);
    }

    vec_destroy(profiles, 0);
    vec_destroy(threads, 0);
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
