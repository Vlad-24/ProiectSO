#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

int pipe_fd = -1;

typedef struct {
    int treasure_id;
    char username[20];
    float latitude;
    float longitude;
    char clue[100];
    int value;
} Treasure;

void handle_signal(int sig) {
    int fd = open("cmd.txt", O_RDONLY);
    if (fd == -1) return;

    char line[256] = {0};
    ssize_t n = read(fd, line, sizeof(line) - 1);
    close(fd);
    unlink("cmd.txt");

    if (n <= 0) return;

    char cmd[32], arg1[64] = "", arg2[64] = "";
    int arg_count = 0;

    sscanf(line, "%s %s %s", cmd, arg1, arg2);
    arg_count = (strlen(arg1) > 0) + (strlen(arg2) > 0);

    int fd_copy = dup(pipe_fd);
    if (fd_copy == -1) return;

    FILE *out = fdopen(fd_copy, "w");
    if (!out) {
        close(fd_copy);
        return;
    }

    if (strcmp(cmd, "list_hunts") == 0) {
        DIR *dir = opendir(".");
        if (!dir) {
            fprintf(out, "Cannot open current directory.\n");
        } else {
            struct dirent *entry;
            while ((entry = readdir(dir))) {
                if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
                    char path[256];
                    snprintf(path, sizeof(path), "%s/treasures.dat", entry->d_name);
                    int tfd = open(path, O_RDONLY);
                    if (tfd == -1) continue;

                    Treasure t;
                    int count = 0;
                    while (read(tfd, &t, sizeof(Treasure)) == sizeof(Treasure)) count++;
                    close(tfd);

                    fprintf(out, "Hunt: %s - %d treasures\n", entry->d_name, count);
                }
            }
            closedir(dir);
        }
    }
    else if (strcmp(cmd, "list_treasures") == 0 && arg_count >= 1) {
        char path[256];
        snprintf(path, sizeof(path), "%s/treasures.dat", arg1);
        int tfd = open(path, O_RDONLY);
        if (tfd == -1) {
            fprintf(out, "Hunt '%s' not found.\n", arg1);
        } else {
            Treasure t;
            while (read(tfd, &t, sizeof(Treasure)) == sizeof(Treasure))
                fprintf(out, "Treasure ID: %d\n", t.treasure_id);
            close(tfd);
        }
    }
    else if (strcmp(cmd, "view_treasure") == 0 && arg_count >= 2) {
        int tid = atoi(arg2);
        char path[256];
        snprintf(path, sizeof(path), "%s/treasures.dat", arg1);
        int tfd = open(path, O_RDONLY);
        if (tfd == -1) {
            fprintf(out, "Hunt '%s' not found.\n", arg1);
        } else {
            Treasure t;
            int found = 0;
            while (read(tfd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
                if (t.treasure_id == tid) {
                    fprintf(out, "Treasure ID: %d\nUsername: %s\nLat: %.2f\nLon: %.2f\nClue: %s\nValue: %d\n",
                            t.treasure_id, t.username, t.latitude, t.longitude, t.clue, t.value);
                    found = 1;
                    break;
                }
            }
            if (!found)
                fprintf(out, "Treasure ID %d not found in hunt '%s'.\n", tid, arg1);
            close(tfd);
        }
    }
    else if (strcmp(cmd, "calculate_score") == 0) {
        DIR *dir = opendir(".");
        if (!dir) {
            fprintf(out, "Cannot open current directory.\n");
        } else {
            struct dirent *entry;
            while ((entry = readdir(dir))) {
                if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
                    char path[256];
                    snprintf(path, sizeof(path), "%s/treasures.dat", entry->d_name);
                    int tfd = open(path, O_RDONLY);
                    if (tfd == -1) continue;

                    typedef struct {
                        char username[20];
                        int score;
                    } UserScore;

                    UserScore users[100];
                    int user_count = 0;

                    Treasure t;
                    while (read(tfd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
                        int found = 0;
                        for (int i = 0; i < user_count; i++) {
                            if (strcmp(users[i].username, t.username) == 0) {
                                users[i].score += t.value;
                                found = 1;
                                break;
                            }
                        }
                        if (!found && user_count < 100) {
                            strcpy(users[user_count].username, t.username);
                            users[user_count].score = t.value;
                            user_count++;
                        }
                    }

                    close(tfd);
                    for (int i = 0; i < user_count; i++) {
                        fprintf(out, "Hunt: %s | User: %s | Score: %d\n",
                                entry->d_name, users[i].username, users[i].score);
                    }
                }
            }
            closedir(dir);
        }
    }
    else if (strcmp(cmd, "stop_monitor") == 0) {
        fprintf(out, "Stopping monitor...\n");
        fflush(out); 
        fclose(out);
        usleep(1000000);
        exit(0);
    }
    else {
        fprintf(out, "Invalid command: %s\n", cmd);
    }

    fclose(out);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <write_pipe_fd>\n", argv[0]);
        return 1;
    }

    pipe_fd = atoi(argv[1]);

    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    while (1) pause();
    return 0;
}
