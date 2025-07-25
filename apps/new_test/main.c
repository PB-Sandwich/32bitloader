
#include <estros/file.h>
#include <estros/keyboard.h>
#include <estros/process.h>
#include <stdio.h>
#include <string.h>

int main()
{
    char path_to_apps[] = "/apps/";

    Process* p = get_current_process();

    while (1) {
        write_file(p->stdout, ">", 1);
        char input_buffer[64] = { 0 };
        read_file(p->stdin, input_buffer, 62);
        while (read_file(p->stdin, &input_buffer[63], 1) != 0)
            ;

        char full_path[strlen(path_to_apps) + strlen(input_buffer) + 1];
        memcpy(full_path, path_to_apps, strlen(path_to_apps));
        memcpy(full_path + strlen(path_to_apps), input_buffer, strlen(input_buffer));
        full_path[strlen(path_to_apps) + strlen(input_buffer)] = '\0';

        File* tty = open_file("/dev/tty", ESTROS_READ | ESTROS_WRITE);
        Process* child = launch_file(full_path, tty, tty, tty);
        while (child != (void*)0 && child->state != PROCESS_TERMINATED)
            ;
    }

    return 0;
}
