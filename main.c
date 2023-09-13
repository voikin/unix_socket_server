#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

#define SERVER_FIFO "/tmp/server_fifo"
#define MAX_BUF_SIZE 256

int main() {
    umask(0);
    mkfifo(SERVER_FIFO, S_IFIFO | 0666);

    int server_fd = open(SERVER_FIFO, O_RDONLY);
    if (server_fd == -1) {
        perror("Failed to open server FIFO");
        exit(EXIT_FAILURE);
    }

    char client_fifo[MAX_BUF_SIZE];
    char buf[MAX_BUF_SIZE];

    while (1) {
        int bytes_read = read(server_fd, client_fifo, sizeof(client_fifo));
        if (bytes_read > 0) {
            int client_fd = open(client_fifo, O_WRONLY);

            // Create a child process to handle the client
            pid_t child_pid = fork();
            if (child_pid == 0) {  // Child process
                close(server_fd);  // Close server FIFO in child

                struct timeval start_time, current_time;
                gettimeofday(&start_time, NULL);

                while (1) {
                    gettimeofday(&current_time, NULL);
                    int elapsed_time = current_time.tv_sec - start_time.tv_sec;

                    // Send random data to the client
                    char random_char = 'A' + rand() % 26;
                    write(client_fd, &random_char, sizeof(random_char));

                    // Periodically change the process priority
                    if (rand() % 100 > 80) {
                        int new_priority = rand() % 11;
                        setpriority(PRIO_PROCESS, 0, new_priority);
                    }

                    if (elapsed_time >= 30) {
                        break;
                    }
                }

                close(client_fd);
                exit(EXIT_SUCCESS);
            } else if (child_pid < 0) {
                perror("Failed to create child process");
            }

            close(client_fd);
        }
    }

    close(server_fd);
    unlink(SERVER_FIFO);  // Remove the server FIFO
    return 0;
}
