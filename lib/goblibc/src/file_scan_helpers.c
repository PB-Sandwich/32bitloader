#include <file.h>
#include <stdio.h>
#include <string.h>

// this is just a placeholder. remove this ifdef and write actual file functions here

void scan_help_file_from_string(FILE *file, const char *str)
{
    file->buf = str;
    file->rpos = str;
    file->rend = (void *)-1;
}

void scan_help_lim(FILE *file, off_t lim)
{
    file->shlim = lim;
    file->shcnt = file->buf - file->rpos;
    /* If lim is nonzero, rend must be a valid pointer. */
    if (lim && file->rend - file->rpos > lim)
    {
        file->shend = file->rpos + lim;
    }
    else
    {
        file->shend = file->rend;
    }
}

char __scan_help_getc(FILE *file)
{
    char c;
    off_t cnt = scan_help_cnt(file);
    if (file->shlim > 0 && cnt >= file->shlim || (c = __uflow(file)) < 0)
    {
        file->shcnt = file->buf - file->rpos + cnt;
        file->shend = file->rpos;
        file->shlim = -1;
        return EOF;
    }
    cnt++;
    if (file->shlim && file->rend - file->rpos > file->shlim - cnt)
    {
        file->shend = file->rpos + (file->shlim - cnt);
    }
    else
    {
        file->shend = file->rend;
    }
    file->shcnt = file->buf - file->rpos + cnt;
    if (file->rpos <= file->buf)
    {
        file->rpos[-1] = c;
    }
    return c;
}

char scan_help_getc(FILE *file)
{
    return (file->rpos != file->shend) ? *(file)->rpos++ : __scan_help_getc(file);
}

void scan_help_unget(FILE *file)
{
    if (file->shlim >= 0)
    {
        file->rpos--;
    }
}

int scan_help_cnt(FILE *file)
{
    return file->shcnt + (file->rpos - file->buf);
}

int __uflow(FILE *file)
{
    // TODO: Update this to read from file when syscalls are added
    return EOF;
}

uint32_t scan_help_write_str(const unsigned char *restrict str, uint32_t len, FILE *restrict file)
{
    for (uint32_t i = 0; i < len; i++)
    {
        *file->rpos = str[i];
        file->rpos++;
    }
    return 0;
}