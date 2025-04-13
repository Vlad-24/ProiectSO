#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

typedef struct {
    int treasure_id;
    char username[20];
    float latitude;
    float longitude;
    char clue[100];
    int value;
} Treasure;

void log_operation(const char *hunt_id, const char *message) 
{
    char path[100];
    snprintf(path, sizeof(path), "%s/logged_hunt", hunt_id);
    int f = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (f == -1) 
    {
        printf("error opening file\n");
        exit(-1);
    }
    if(write(f,message,strlen(message))<0)
    {
        printf("error writing\n");
        exit(-1);
    }
    if(close(f)<0)
    {
        printf("error closing the file\n");
        exit(-1);
    }
}

void create_symlink(const char *hunt_id) 
{
    char path[100], link[100];
    snprintf(path, sizeof(path), "%s/logged_hunt", hunt_id);
    snprintf(link, sizeof(link), "logged_hunt-%s", hunt_id);
    unlink(link); 
    if (symlink(path, link) == -1) 
    {
        printf("error creating symlink\n");
        exit(-1);
    }
}

void add_treasure(const char *hunt_id) 
{
    char dir[100], file[100];
    snprintf(dir, sizeof(dir), "%s", hunt_id);
    snprintf(file, sizeof(file), "%s/treasures.dat", dir);
    mkdir(dir, 0755);
    Treasure t;
    printf("Enter Treasure ID: "); 
    scanf("%d", &t.treasure_id); 
    getchar();
    printf("Enter Username: "); 
    fgets(t.username, sizeof(t.username), stdin);
    t.username[strcspn(t.username, "\n")] = 0;
    printf("Enter Latitude: "); 
    scanf("%f", &t.latitude);
    printf("Enter Longitude: "); 
    scanf("%f", &t.longitude); 
    getchar();
    printf("Enter Clue: "); 
    fgets(t.clue, sizeof(t.clue), stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;
    printf("Enter Value: "); 
    scanf("%d", &t.value);
    int f = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (f == -1)
    {
        printf("error opening file\n");
        exit(-1);
    }
    write(f, &t, sizeof(t));
    if(close(f)<0)
    {
        printf("Error closing the file\n");
        exit(-1);
    } 
    char log[128];
    snprintf(log, sizeof(log), "-add: treasure ID %d by user %s\n", t.treasure_id, t.username);
    log_operation(hunt_id, log);
    create_symlink(hunt_id);
    printf("Treasure added successfully.\n");
}

void list_treasures(const char *hunt_id) 
{
    char file[100];
    snprintf(file, sizeof(file), "%s/treasures.dat", hunt_id);
    struct stat st;
    if (stat(file, &st) == -1) {
        perror("No hunt data found");
        exit(-1);
    }
    printf("Hunt: %s\nSize: %lld bytes\nModified: %s", hunt_id, (long long)st.st_size, ctime(&st.st_mtime));
    int f = open(file, O_RDONLY);
    if (f < 0) 
    {
        printf("error opening file\n");
        exit(-1);
    }
    Treasure t;
    while (read(f, &t, sizeof(t)) == sizeof(t)) {
        printf("\nID: %d | User: %s | Location: %.2f, %.2f | Clue: %s | Value: %d\n",t.treasure_id, t.username, t.latitude, t.longitude, t.clue, t.value);
    }
    if(close(f)<0)
    {
        printf("error closing the file\n");
        exit(-1);
    }
    char log[128];
    snprintf(log, sizeof(log), "-list: treasures from Hunt %s\n", hunt_id);
    log_operation(hunt_id, log);
}

void view_treasure(const char *hunt_id, int treasure_id) 
{
    char file[100];
    snprintf(file, sizeof(file), "%s/treasures.dat", hunt_id);
    int f = open(file, O_RDONLY);
    if (f < 0) 
    {
        printf("Error opening file\n");
        exit(-1);
    }
    Treasure t;
    while (read(f, &t, sizeof(t)) == sizeof(t)) {
        if (t.treasure_id == treasure_id) {
            printf("ID: %d\nUser: %s\nLat: %.2f\nLong: %.2f\nClue: %s\nValue: %d\n", t.treasure_id, t.username, t.latitude, t.longitude, t.clue, t.value);
            if(close(f)<0)
            {
                printf("error closing the file\n");
                exit(-1);
            }
            char log[128];
            snprintf(log, sizeof(log), "-view: treasure ID %d by user %s\n", treasure_id, t.username);
            log_operation(hunt_id, log);        
            return;
        }
    }
    printf("Treasure not found.\n");
    if(close(f)<0)
    {
        printf("error closing the file\n");
        exit(-1);
    }   
    char log[128];
    snprintf(log, sizeof(log), "-view: treasure ID %d not found\n", treasure_id);
    log_operation(hunt_id, log);
}

void remove_treasure(const char *hunt_id, int treasure_id) 
{
    char file[100], temp[100];
    snprintf(file, sizeof(file), "%s/treasures.dat", hunt_id);
    snprintf(temp, sizeof(temp), "%s/temp.dat", hunt_id);
    int f = open(file, O_RDONLY);
    if (f == -1) 
    {
        printf("error opening file\n");
        exit(-1);
    }
    int tmp = open(temp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (tmp == -1) 
    {
        printf("error opening file\n");
        exit(-1);
    }
    Treasure t;
    int found = 0;
    char name[20];
    while (read(f, &t, sizeof(t)) == sizeof(t)) {
        if (t.treasure_id == treasure_id) {
            found = 1;
            strcpy(name, t.username);
            continue;
        }
        write(tmp, &t, sizeof(t));
    }
    if(close(f)<0)
    {
        printf("error closing the file\n");
        exit(-1);
    }
    if(close(tmp)<0)
    {
        printf("error closing the file\n");
        exit(-1);
    }
    if (found) 
    {
        remove(file);
        rename(temp, file);
        printf("Treasure %d removed.\n", treasure_id);
    } 
    else 
    {
        remove(temp);
        printf("Treasure %d not found.\n", treasure_id);
    }
    char log[128];
    snprintf(log, sizeof(log), "-remove_treasure: treasure ID %d by user %s\n", treasure_id, name);
    log_operation(hunt_id, log);
}

void remove_hunt(const char *hunt_id) 
{
    char path[100];
    const char *files[] = {"treasures.dat", "logged_hunt", "temp.dat"};
    for (int i = 0; i < 3; ++i) {
        snprintf(path, sizeof(path), "%s/%s", hunt_id, files[i]);
        unlink(path);
    }
    if (rmdir(hunt_id) == -1) 
    {
        printf("Error rmdir");
        exit(-1);
    }
    snprintf(path, sizeof(path), "logged_hunt-%s", hunt_id);
    unlink(path);
    printf("Hunt '%s' removed.\n", hunt_id);
}

enum Command {
    CMD_UNKNOWN, CMD_ADD, CMD_LIST, CMD_VIEW, CMD_REMOVE_TREASURE, CMD_REMOVE_HUNT
};

enum Command get_command(const char *arg) {
    if (strcmp(arg, "--add") == 0) return CMD_ADD;
    if (strcmp(arg, "--list") == 0) return CMD_LIST;
    if (strcmp(arg, "--view") == 0) return CMD_VIEW;
    if (strcmp(arg, "--remove_treasure") == 0) return CMD_REMOVE_TREASURE;
    if (strcmp(arg, "--remove_hunt") == 0) return CMD_REMOVE_HUNT;
    return CMD_UNKNOWN;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage:\n"
               "  --add <hunt_id>\n"
               "  --list <hunt_id>\n"
               "  --view <hunt_id> <treasure_id>\n"
               "  --remove_treasure <hunt_id> <treasure_id>\n"
               "  --remove_hunt <hunt_id>\n");
        return 1;
    }
    switch (get_command(argv[1])) {
        case CMD_ADD: add_treasure(argv[2]); break;
        case CMD_LIST: list_treasures(argv[2]); break;
        case CMD_VIEW:
            if (argc < 4) {
                printf("Missing treasure ID.\n");
                return 1;
            }
            view_treasure(argv[2], atoi(argv[3]));
            break;
        case CMD_REMOVE_TREASURE:
            if (argc < 4) {
                printf("Missing treasure ID.\n");
                return 1;
            }
            remove_treasure(argv[2], atoi(argv[3]));
            break;
        case CMD_REMOVE_HUNT: remove_hunt(argv[2]); break;
        default:
            printf("Unknown command.\n");
            return 1;
    }
    return 0;
}
