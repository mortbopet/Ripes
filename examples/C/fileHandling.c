#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>

int main() {

    // File created in write mode.
    // File will be created on the same root as the executable.
    int fptr = open("sample.txt", O_WRONLY | O_CREAT, 0644);
    char buff[33] = "Hello Ripes! This is a test file.";
    if (fptr < 0) {
        printf("Error in writing!\n");
    }
    write(fptr, buff, sizeof(buff));
    printf("Written successfully!\n");

    // File stats diplayed here using fstat.
    struct stat file_info;
    int status = fstat(fptr, &file_info);
    if (status < 0) {
        printf("Error in accessing the file info!\n");
    }

    off_t file_size = file_info.st_size;
    uid_t user = file_info.st_uid;
    dev_t device = file_info.st_dev;
    mode_t file_mode = file_info.st_mode;

    printf("File size: %ld bytes\n", file_size);
    printf("User Id: %d\n", user);
    printf("Device: %ld\n", device);
    printf("File mode: %d\n", file_mode);

    close(fptr);

    // File opened in read.
    fptr = open("sample.txt", O_RDONLY);
    int bytes = 30;
    char read_buff[bytes];
    if (fptr < 0) {
        printf("Error in reading!\n");
    }
    read(fptr, &read_buff, bytes);
    printf("Read %i bytes: %s\n", bytes, read_buff);

    // Using lseek to change the cursor position.
    lseek(fptr, -20, SEEK_CUR);
    read(fptr, &read_buff, bytes);
    printf("Read after lseek: %s", read_buff);

    close(fptr);

    return 0;
}