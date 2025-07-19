
# Terminal

the terminal allows for simple io via a file pointer

using `write` will print data to the terminal
using `read` will get `'\n'` terminated user input
    read expects you to call it until it returns 0, or else it will not fetch user input on next call

```
    TTY_CLEAR = 0,
    TTY_SET_BG_COLOR = 1,
    TTY_SET_FG_COLOR = 2,
    TTY_SET_CURSOR_POS = 3,
    TTY_GET_CURSOR_POS = 4,
```
