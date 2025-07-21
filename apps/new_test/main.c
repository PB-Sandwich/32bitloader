
#include <estros/app_data.h>
#include <estros/file.h>
#include <estros/keyboard.h>
#include <stdio.h>
#include <string.h>

char err_msg[] = "unable to open file\n";
char msg[] = "hello world!\n";

int main()
{
    EstrosAppData ad = get_app_data();
    write_file(ad.stdout, msg, strlen(msg));

    File* file = open_file("/testfile", ESTROS_READ | ESTROS_WRITE);
    if (file == 0) {
        write_file(ad.stdout, err_msg, strlen(err_msg));
        return 0;
    }

    File* kdb = open_file("/dev/kdb", ESTROS_READ);
    write_file(ad.stdout, "Press any key to continue\n", strlen("Press any key to continuen\n"));
    wait_for_keypress(kdb);
    write_file(ad.stdout, "what would you like to put in the file\n", strlen("what would you like to put in the file\n"));

    char buffer[512]; // yes this is very large but who cares
    read_file(ad.stdin, buffer, 512);

    write_file(file, buffer, strlen(buffer));

    uint32_t cmd = 0;
    ioctl(ad.stdout, &cmd, &cmd);

    return 0;
}
