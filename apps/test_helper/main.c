#include <stdint.h>
#include <estros/keyboard.h>
#include <stdio.h>
#include <stdlib.h>
#include <estros.h>
int main()
{
    File *kdb = open_file("/dev/kdb", ESTROS_READ);

    char buffer[4];
    while (true)
    {
        enum Keycode keycode = wait_for_keypress(kdb);
        gob_sprintf(buffer, "%x ", keycode);
        sys_print(buffer, 2);
    }
}