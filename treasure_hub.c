#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() 
{
    char input[256];
    pid_t monitor_pid = -1;
    FILE *cmd_fp;
    printf("Treasure Hunt Hub\n");
    printf("- 'start_monitor' to start the process.\n");
    while (1) 
    {
        printf("> ");
        fgets(input, 256, stdin);
        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "start_monitor") == 0) {
            monitor_pid = fork();
            if (monitor_pid == 0) 
            {
                execl("./monitor", "monitor", NULL);
                perror("Failed to start monitor");
                exit(1);
            } 
            else if (monitor_pid < 0) 
            {
                perror("fork failed");
                exit(1);
            }
            cmd_fp = fopen("cmd.txt", "w");
            if (cmd_fp == NULL) 
            {
                perror("fopen");
                exit(1);
            }
            fclose(cmd_fp);
            printf("Monitor started! All commands:\n");
            printf("- list_hunts\n");
            printf("- list_treasures <hunt_id>\n");
            printf("- view_treasure <hunt_id> <treasure_id>\n");
            printf("- stop_monitor\n");
            printf("- exit\n");
        } 
        else if (strcmp(input, "stop_monitor") == 0) 
        {
            cmd_fp = fopen("cmd.txt", "w");
            if (cmd_fp) 
            {
                fprintf(cmd_fp, "stop_monitor\n");
                fclose(cmd_fp);
            }
            if (monitor_pid > 0) 
            {
                waitpid(monitor_pid, NULL, 0);
                printf("Monitor stopped.\n");
                monitor_pid = -1;
            }
        } 
        else if (strcmp(input, "exit") == 0) 
        {
            if (monitor_pid > 0) 
            {
                printf("Please stop the monitor 'stop_monitor' before exiting.\n");
            } 
            else 
            {
                printf("Exiting Treasure Hunt Hub.\n");
                break;
            }
        } 
        else if (monitor_pid > 0) 
        {
            cmd_fp = fopen("cmd.txt", "w");
            if (!cmd_fp) 
            {
                perror("fopen");
                continue;
            }
            fprintf(cmd_fp, "%s\n", input);
            fclose(cmd_fp);
            sleep(1);
        } 
        else 
        {
            printf("Monitor not running. Start it using 'start_monitor'.\n");
        }
    }
    return 0;
}
