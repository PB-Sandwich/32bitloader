
#include <estros/app_data.h>
#include <estros/file.h>
#include <string.h>
#include <stdio.h>

char err_msg[] = "unable to open file\n";
char msg[] = "hello world!\n";

int main()
{
    EstrosAppData ad = get_app_data();
    write_file(ad.stdout, msg, strlen(msg));

    File* file = open_file("/testfile", ESTROS_READ);
    if (file == 0) {
        write_file(ad.stdout, err_msg, strlen(err_msg));
        return 0;
    }
    char buffer[1524];
    read_file(file, buffer, 1524);
    write_file(ad.stdout, buffer, strlen(buffer));
    for (int i = 0; i < 1024; i++ ) {
        write_file(file, &i, 1);
    }
    write_file(file, msg, strlen(msg));

    seek_file(file, 13, ESTROS_END);
    char* text[13];
    read_file(file, text, 13);
    write_file(ad.stdout, "s: ", 3);
    write_file(ad.stdout, text, 13);
    char* out[10];
    gob_sprintf((char*)out, "%d", tell_file(file));
    write_file(ad.stdout, out, strlen((char*)out));

    return 0;
}
