
#include <estros/app_data.h>
#include <estros/file.h>
#include <string.h>

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
    char buffer[512];
    read_file(file, buffer, 512);
    write_file(ad.stdout, buffer, 512);

    return 0;
}
