#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <format.h>
#include <stdio.h>
#include <estros.h>
#include <estros/file.h>
#include <estros/keyboard.h>
#include <stdio.h>


typedef enum Keycode EstrosKeycode;

void getline(char *str, uint32_t max_len)
{
    char buffer[10];
    uint32_t read = sys_read(buffer, max_len);
    if (read <= 0)
    {
        return;
    }

    while (read > 0)
    {
        read = sys_read(buffer, 10);
    }
}



int main()
{
    File *kdb = open_file("/dev/kdb", ESTROS_READ);

    char filename[255];
    File *file = NULL;
    while (file == NULL)
    {
        sys_clear();
        sys_print("Enter filename: \n", 18);
        getline(filename, sizeof(filename));
        file = open_file(filename, ESTROS_READ | ESTROS_WRITE);
        if (file == NULL)
        {
            sys_print("File not found!\n", 17);
            char input[10];
            do
            {
                sys_print("Exit? (y/n)\n", 13);
                getline(input, sizeof(input));
            } while (strcmp(input, "y") != 0 && strcmp(input, "n") != 0);
            if (strcmp(input, "y") == 0)
            {
                return 0;
            }
        }
    }

    return 0;
    
    if (file == 0)
    {
        sys_print("Error! File 'test.gi' not found!", 33);
        return 1;
    }
    uint32_t width = 0;
    uint32_t height = 0;
    read_file(file, &width, 1);
    read_file(file, &height, 1);
    if (width > 80 || height > 25)
    {
        char text[100];
        gob_sprintf(text, "Invalid image dimensions. Width: %d, Height: %d", width, height);
        sys_print(text, strlen(text));
        close_file(file);
        return 0;
    }

    uint8_t *image = malloc(width * height);

    read_file(file, image, width * height);

    uint16_t *text_buffer = get_text_buffer_address();
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            text_buffer[i + j * 80] = (image[i + j * width] << (8 + 4)) | ' ';
        }
    }
    close_file(file);
    return 0;
}