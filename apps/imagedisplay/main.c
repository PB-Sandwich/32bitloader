#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <format.h>
#include <stdio.h>
#include <estros.h>
#include <estros/file.h>
#include <stdio.h>

void getline(char *str, uint32_t max_len)
{
    char buffer[10];
    uint32_t read = sys_read(buffer, 10);
    
    while(read > 0)
    {
        read = sys_read(str, 10);
        str += read;
    }
}
int main()
{
    char filename[255];
    File *file = open_file("/test.gi", ESTROS_READ);
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