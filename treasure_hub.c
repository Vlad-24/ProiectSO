 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <termios.h>

int pipefd[2];             // pipefd[0] = citire
pid_t monitor_pid = -1;
struct termios orig_termios;

void disable_terminal_input() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &orig_termios); // salvează starea curentă
    new_termios = orig_termios;

    new_termios.c_lflag &= ~(ICANON | ECHO); // oprește inputul canonic și afișarea
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios);
}

void enable_terminal_input() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); // restaurăm starea originală
}

void print(const char *msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}

void readPipe() {
    char buffer[1024];
    ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        //print("Monitor Response:\n");
        print(buffer);
    }
}

void sendCommand(const char *cmd_line) {
    int fd = open("cmd.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        print("Error: could not write cmd.txt\n");
        return;
    }
    write(fd, cmd_line, strlen(cmd_line));
    write(fd, "\n", 1);
    close(fd);

    kill(monitor_pid, SIGUSR1);
    readPipe();
}

void startMonitor() {
    if (pipe(pipefd) == -1) {
        print("Error: pipe creation failed\n");
        exit(1);
    }

    monitor_pid = fork();
    if (monitor_pid == -1) {
        print("Error: fork failed\n");
        exit(1);
    }

    if (monitor_pid == 0) {
        close(pipefd[0]); // copilul nu citește
        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", pipefd[1]);
        execl("./monitor", "monitor", fd_str, NULL);
        exit(1);
    }

    close(pipefd[1]); // părintele nu scrie
    print("Monitor started! All commands:\n");
    print("- list_hunts\n");
    print("- list_treasures <hunt_id>\n");
    print("- view_treasure <hunt_id> <treasure_id>\n");
    print("- calculate_score\n");
    print("- stop_monitor\n");
    print("- exit\n");
}

int main() {
    char input[256];
    print("Treasure Hunt Hub\nUse 'start_monitor' to begin.\n");
    while (1) {
        printf("> ");

        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "start_monitor") == 0) {
            if (monitor_pid != -1) {
                print("Monitor already running.\n");
            } else {
                startMonitor();
            }
        }
        else if (strcmp(input, "stop_monitor") == 0) {
            if (monitor_pid == -1) {
                print("Monitor not running.\n");
            } else {
                disable_terminal_input();
                sendCommand("stop_monitor");
                waitpid(monitor_pid, NULL, 0);
                monitor_pid = -1;
                print("Monitor stopped.\n");
                enable_terminal_input();
            }
        }
        else if (strcmp(input, "exit") == 0) {
            if (monitor_pid != -1) {
                print("Please stop the monitor first.\n");
            } else {
                print("Exiting hub.\n");
                break;
            }
        }
        else if (monitor_pid == -1) {
            print("Monitor not running. Use 'start_monitor'.\n");
        }
        else {
            sendCommand(input);
        }
    }

    return 0;
    
}
