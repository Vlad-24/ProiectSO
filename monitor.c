#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct {
    int treasure_id;
    char username[20];
    float latitude;
    float longitude;
    char clue[100];
    int value;
} Treasure;

void handle_list_hunts() 
{
    DIR *dir = opendir(".");
    if (!dir) 
    {
        perror("opendir");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir))) 
    {
        if (entry->d_type == DT_DIR && strncmp(entry->d_name, ".", 1) != 0 && strcmp(entry->d_name, "..") != 0) 
        {
            char path[512];
            snprintf(path, sizeof(path), "%s/treasures.dat", entry->d_name);
            FILE *fp = fopen(path, "rb");
            if (!fp) continue;
            int count = 0;
            Treasure t;
            while (fread(&t, sizeof(Treasure), 1, fp) == 1)
                count++;
            fclose(fp);
            printf("Hunt: %s - %d treasures\n", entry->d_name, count);
        }
    }
    closedir(dir);
}

void handle_list_treasures(const char *hunt_id) 
{
    char path[256];
    snprintf(path, sizeof(path), "%s/treasures.dat", hunt_id);
    FILE *fp = fopen(path, "rb");
    if (!fp) 
    {
        printf("Hunt '%s' not found or cannot open file.\n", hunt_id);
        return;
    }
    Treasure t;
    while (fread(&t, sizeof(Treasure), 1, fp) == 1) 
    {
        printf("Treasure ID: %d\n", t.treasure_id);
    }
    fclose(fp);
}

void handle_view_treasure(const char *hunt_id, int treasure_id) 
{
    char path[512];
    snprintf(path, sizeof(path), "%s/treasures.dat", hunt_id);
    FILE *fp = fopen(path, "rb");
    if (!fp) 
    {
        printf("Hunt '%s' not found.\n", hunt_id);
        return;
    }
    Treasure t;
    int found = 0;
    while (fread(&t, sizeof(Treasure), 1, fp) == 1) 
    {
        if (t.treasure_id == treasure_id) 
        {
            printf("Treasure ID: %d\n", t.treasure_id);
            printf("Username: %s\n", t.username);
            printf("Latitude: %.2f\n", t.latitude);
            printf("Longitude: %.2f\n", t.longitude);
            printf("Clue: %s\n", t.clue);
            printf("Value: %d\n", t.value);
            found = 1;
            break;
        }
    }
    if (!found)
        printf("Treasure ID %d not found in hunt '%s'.\n", treasure_id, hunt_id);

    fclose(fp);
}

int main() 
{
    while (1) 
    {
        FILE *fp = fopen("cmd.txt", "r");
        if (!fp) 
        {
            sleep(1);
            continue;
        }
        char line[256];
        if (fgets(line, 256, fp)) 
        {
            line[strcspn(line, "\n")] = 0;
            char cmd[32], arg1[64];
            int arg2;
            int num_tokens = sscanf(line, "%s %s %d", cmd, arg1, &arg2);
            if (strcmp(cmd, "list_hunts") == 0) 
            {
                handle_list_hunts();
            } 
            else if (strcmp(cmd, "list_treasures") == 0 && num_tokens == 2) 
            {
                handle_list_treasures(arg1);
            } 
            else if (strcmp(cmd, "view_treasure") == 0 && num_tokens == 3) 
            {
                handle_view_treasure(arg1, arg2);
            } 
            else if (strcmp(cmd, "stop_monitor") == 0) 
            {
                printf("Stopping monitor...\n");
                break;
            } 
            else if (strcmp(cmd, "exit") == 0) 
            {
                printf("Cannot exit: Monitor is still active. Please stop it first using 'stop_monitor'.\n");
            } 
            else 
            {
                printf("Invalid command: %s\n", line);
            }
            fclose(fp);
            fp = fopen("cmd.txt", "w");
            if (fp) fclose(fp);
        } 
        else 
        {
            fclose(fp);
        }
        sleep(1);
    }

    return 0;
}
