[bits 32]
start:
    mov eax, 0x01
    int 0x40

    rdtsc
    mov [rand_vals.prev], eax
    
    mov eax, 0x03
    int 0x40
    mov [graphics.address], ebx


    mov ebx, game_start_text
    mov ecx, colors.status_good
    mov eax, 2
    call write_to_status_bar
    call clear
    call generate
    
game_loop:
input:
    cmp byte [game.state], game.state_playing
    jne .skip_controls
    
    call repaint_cells
    mov eax, [cursor.x]
    mov [cursor.prev_x], eax
    mov eax, [cursor.y]
    mov [cursor.prev_y], eax
   
    mov eax, 0x5
    int 0x40
    
    cmp bl, keyboard.exit
    jne .check_up
    ret
    .check_up:
        cmp bl, keyboard.up
        jne .check_down
        cmp dword [cursor.y], 0
        jle .check_down
        dec dword [cursor.y]
    .check_down:
        cmp bl, keyboard.down
        jne .check_right
        cmp dword [cursor.y], minefield.h - 1
        jge .check_right
        inc dword [cursor.y]
    .check_right:
        cmp bl, keyboard.right
        jne .check_left
        cmp dword [cursor.x], minefield.w - 1
        jge .check_left
        inc dword [cursor.x]
    .check_left:
        cmp bl, keyboard.left
        jne .check_reveal
        cmp dword [cursor.x], 0
        jle .check_reveal
        dec dword [cursor.x]
    .check_reveal:
        cmp bl, keyboard.reveal
        jne .check_mark
        mov eax, [cursor.x]
        mov ebx, [cursor.y]
        call try_reveal
    .check_mark:
        cmp bl, keyboard.mark
        jne .end_check
        mov eax, [cursor.x]
        mov ebx, [cursor.y]
        call mark_as_bomb 
    .end_check:
    jmp game_loop    

.skip_controls:
    mov eax, 0x5
    int 0x40
    cmp bl, keyboard.exit
    jne .check_restart
    ret
    .check_restart:
    cmp bl, keyboard.restart
    jne .skip_controls

    mov byte [game.state], game.state_playing
    mov dword [cursor.x], 0
    mov dword [cursor.y], 0
    mov dword [cursor.prev_x], 0
    mov dword [cursor.prev_y], 0

    mov ecx, (minefield.h + 2) * (minefield.w + 2)
    mov ebx, minefield.cells
    .data_loop:
        mov byte [ebx], 0
        inc ebx
    loop .data_loop
    jmp start


.halt:
    hlt
    jmp .halt

; eax - x
; ebx - y
; modifies : ecx, edx
mark_as_bomb:
    call calculate_grid_pos
    call calculate_buffer_pos
    test byte [ecx], bomb_flags.revealed                                ; revealed cells can't be marked
    jnz .end
    xor byte [ecx], bomb_flags.marked                                   ; toggle the marked bit
    test byte [ecx], bomb_flags.marked                                  ; check the state of that bit
    jnz .display_question
        mov word [edx], (colors.hidden << 8) | ' '                      ; just in case clear the displayed value to space
        jmp .end
    .display_question:
        mov word  [edx], (colors.marked << 8) |  '?'
    .end:
    call check_all_bombs
    ret

; checks if all bombs were marked and no other cell is marked
; modifies: eax, ebx, ecx, edx 
check_all_bombs:
    xor eax, eax
    xor ebx, ebx
    .hor:                
        call calculate_buffer_pos                                   ; calculate position in data arrays
        test byte [ecx], bomb_flags.bomb                            ; check if it's  a bomb, skip everything otherwise
        jz .hor_end
            test byte [ecx], bomb_flags.marked                      ; check if bomb was marked, otherwise no point in checking the rest
            jz .no_victory_end
            jmp .next_cell
        .hor_end:
            test byte [ecx], bomb_flags.marked                      ; if it's not a bomb, then we have to check if it's not marked
            jnz .no_victory_end                                     ; if it's not a bomb and marked, quit the check
        .next_cell:
            inc eax
            cmp eax, minefield.w
            jl .hor
            xor eax, eax
            inc ebx
            cmp ebx, minefield.h
            jge .victory_end
        jmp .hor

    .victory_end:
    call clear_status_bar
    mov ebx, game_victory_text
    mov ecx, colors.status_good
    mov eax, 2
    call write_to_status_bar

    mov byte [game.state], game.state_won

    .no_victory_end:
    ret

try_reveal:
    call calculate_buffer_pos
    test byte [ecx], bomb_flags.marked
    jnz .end
    test byte [ecx], bomb_flags.bomb
    jz .usual_reveal
    mov byte [game.state], game.state_lost

    call clear_status_bar
    mov ebx, game_over_text
    mov ecx, colors.bad_bomb
    mov eax, 2
    call write_to_status_bar

    xor eax, eax
    xor ebx, ebx
    .hor:
        call calculate_grid_pos
        call calculate_buffer_pos

        test byte [ecx], bomb_flags.bomb
        jz .hor_end
        or byte [ecx], bomb_flags.revealed
        mov byte [edx + 1], colors.bad_bomb
        .hor_end:
        inc eax
        cmp eax, minefield.w
        jl .hor
        xor eax, eax
        inc ebx
        cmp ebx, minefield.h
        jge .end_bomb_reveal
        jmp .hor
    .end_bomb_reveal:
   

    ret
    .usual_reveal:
    call reveal

    .end:
    ret

; ebx - string
; ecx - color
; eax - initial offset
; modifies: eax, ebx, edx
write_to_status_bar:
    mov edx, [graphics.address]
    add edx, eax
    mov ah, cl
    .loop:
        mov al, [ebx]
        test al, al
        jz .end
        mov word [edx], ax
        add edx, 2
        inc ebx
        jmp .loop
    .end:
    ret

; clear the whole status bar with black colors
; modifies: ecx, edx
clear_status_bar:
    mov edx, [graphics.address]
    mov ecx, screen.w
    .loop:
        mov word [edx], 0
        add edx, 2
    loop .loop
    ret

; eax - x
; ebx - y
; modifies: all
reveal:
    cmp eax, -1
    jle .end
    cmp ebx, -1
    jle .end
    cmp eax, minefield.w
    jge .end
    cmp ebx, minefield.h
    jge .end

    call calculate_grid_pos
    call calculate_buffer_pos
    test byte [ecx], bomb_flags.bomb
    jnz .end
    test byte [ecx], bomb_flags.revealed
    jnz .end
    test byte [ecx], bomb_flags.marked
    jnz .end

    mov byte [edx + 1], colors.revealed
    or byte [ecx], bomb_flags.revealed
   
    push ebx 
    push eax
    mov edx, ecx
    mov ecx, cell_offset_count
    xor esi, esi
        mov ebx, cell_check_offsets
        .count_loop:
            push edx
            add edx, [ebx]
            mov al, [edx]
            pop edx
            test al, bomb_flags.bomb
            jz .end_check
            inc esi
            .end_check:
            add ebx, 4
        loop .count_loop
    pop eax
    pop ebx
    push eax
    
    test esi, esi
    jz .dont_set
        push eax                                ; preserve x
        mov eax, esi                            ; load bomb count
        or byte [edx], al                       ; or with the data byte to add the bomb count
        pop eax                                 ; restore x
        call calculate_grid_pos                 ; calculate index into the display
        mov eax, esi                            ; load bomb count
        add eax, '0'                            ; add '0' to convert to ascii
        mov [edx], al                           ; write the value into the display
    .dont_set:
    
    ;pop eax
    pop eax

    
    mov ecx, 4
    mov esi, reveal_offsets_x
    mov edx, reveal_offsets_y
    .reveal_others:
        push eax
        push ebx
        add eax, [esi]
        add ebx, [edx]
        push ecx
        push esi
        push edx
            call reveal
        pop edx
        pop esi
        add edx, 4
        add esi, 4
        pop ecx
        pop ebx
        pop eax
    loop .reveal_others    
    .end:
    ret

cell_check_offsets: dd -1, 1, minefield.w, -minefield.w, -minefield.w - 1, -minefield.w + 1, minefield.w - 1, minefield.w + 1
cell_offset_count equ 8
reveal_offsets_x: dd 0, -1, 0, 1
reveal_offsets_y: dd -1, 0, 1, 0

; calculate address of the cell in the character array
; eax - x
; ebx - y
; result is in edx
; modifies: ecx, edx
calculate_grid_pos:
    lea edx, [eax * 2 + minefield.start_address]                        ; ptr =  x * 2 + screen.w * 2 + 2
    imul ecx, ebx, screen.w * 2                                         
    add edx, ecx                                                        ; ptr += y * screen.w * 2
    add edx, [graphics.address]                                         ; ptr += &vga
    ret

; calculate address of the cell in the data array
; eax - x
; ebx - y
; result in ecx
; modifies: ecx
calculate_buffer_pos:
    imul ecx, ebx, minefield.w                                          ; i = y * width
    add ecx, minefield.w                                                ; i = y * (width + 1)
    add ecx, eax                                                        ; i = y * (width + 1) + x
    inc ecx                                                             ; i = y * (width + 1) + x + 1
    add ecx, minefield.cells                                            ; return &cells[i]
    ret                                                                 ; y and x have + 1 to account for deadzones

repaint_cells:
    mov eax, [cursor.prev_x]
    mov ebx, [cursor.prev_y]

    call calculate_grid_pos
    call calculate_buffer_pos
    test byte [ecx], bomb_flags.revealed                               ; check if was revealed
    jz .hidden
        mov byte [edx + 1], colors.revealed
        ;mov byte [edx], 'b'
        jmp .paint_selected
    .hidden:
        test byte [ecx], bomb_flags.marked
        jnz .marked
        mov byte [edx + 1], colors.hidden
        jmp .paint_selected
    .marked:
        mov byte [edx + 1], colors.marked
   
    .paint_selected:
        mov eax, [cursor.x]
        mov ebx, [cursor.y]
        call calculate_grid_pos
        call calculate_buffer_pos
        test byte [ecx], bomb_flags.revealed
        jz .not_revealed
            mov byte [edx + 1], colors.selected_revealed
            jmp .end
        .not_revealed:
            test byte [ecx], bomb_flags.marked
            jnz .marked_unselected
            mov byte [edx + 1], colors.selected
            jmp .end
        .marked_unselected:
            mov byte [edx + 1], colors.selected_marked
    .end:
    ret


clear:
    mov ecx, minefield.h
    .ver:
        push ecx
        dec ecx                                                 ; we iterate in reverse starting with height
        mov ebx, ecx                                            ; so we have to subtract 1 to account for arrays starting at 0
        mov ecx, minefield.w
        .hor:
            push ecx
            lea eax, [ecx - 1]                                  ; x = i - 1
            call calculate_grid_pos
            mov word [edx], (colors.hidden << 8) | ' '          ; vga[calculate_grid_pos(x, y)] = ' '
            pop ecx
        loop .hor
    pop ecx
    loop .ver
    ret

; generate a new play field
; modifies: eax, ebx, ecx, edx
generate:
    mov ecx, minefield.bomb_count
    .loop:
        push ecx
        mov ecx, minefield.h
        call rand_mod                                           ; generate first value -> y
        mov ebx, eax                                            ; save y in ebx
        push ebx
        mov ecx, minefield.w
        call rand_mod                                           ; generate second value -> x
        and eax, 0xff
        pop ebx
        .bomb_save_loop:
            call calculate_buffer_pos                           ; calculate offset into data array
            test byte [ecx], bomb_flags.bomb                    ; test if we placed bomb here already
            jz .place_bomb                                      ; if not, place
            cmp eax, minefield.w                                ; because we have deadzones we have to check whether we are at the edge
            jge .advance_ver
            .advance_hor:
                inc eax                                         ; x++
                jmp .bomb_save_loop
            .advance_ver:
                mov eax, 1                                      ; reset to first column
                inc ebx                                         ; y++
                jmp .bomb_save_loop
            .place_bomb:
                mov byte [ecx], bomb_flags.bomb
                call calculate_grid_pos
                mov word [edx], (colors.hidden << 8) | 'B'
                call calculate_buffer_pos
        pop ecx
    loop .loop
    ret


graphics:
    .address dd 0


; num is in eax
num_to_string:
    mov ebx, 10
    mov ecx, num_str_temp
    .loop:
        test eax, eax                               ; check if number ended
        jz .end
        xor edx, edx
        div ebx
        add edx, '0'
        mov byte [ecx], dl
        inc ecx
        
    jmp .loop
    .end:
    dec ecx
    mov ebx, num_str_result                             ; load the result address
    .cpy:
        cmp ecx, num_str_temp - 1
        je .return
        mov dl, [ecx]                                   ; move from temp to res
        mov [ebx], dl
        dec ecx
        inc ebx
        jmp .cpy
    .return:
    mov byte [ebx], 0
    ret

; we will write the temp string here
num_str_temp: times 15 db 0
num_str_result: times 15 db 0

; generate next random value
; result is stored in eax
; modifies: eax, ebx, edx
rand:
    mov eax, [rand_vals.prev]
    imul eax, rand_vals.a
    add eax, rand_vals.c
    xor edx, edx
    mov ebx, rand_vals.m
    div ebx
    mov [rand_vals.prev], edx
    mov eax, edx
    ret

; ecx should contain the mod
; eax stores the result
; modifies: eax, ebx, edx
rand_mod:
    call rand
    xor edx, edx
    mov ebx, ecx
    div ebx
    mov eax, edx
    ret

rand_vals:
    .prev dd 0
    .m equ 65537
    .c equ 0
    .a equ 75

cursor:
    .prev_x dd 0
    .prev_y dd 0
    .x dd 0
    .y dd 0

game:
    .state_playing equ 0
    .state_won equ 1
    .state_lost equ 2
    .state db 0

minefield:
    .w equ 78
    .h equ 23
    .bomb_count equ .cell_count / 10
    .cell_count equ .h * .w
    .size equ (.h << 8) | .w
    .start_address equ screen.w * 2 + 2                                 ; skip the first line and column
    .line_offset equ screen.w * 2
    .cells times (.h + 2) * (.w + 2) db 0                              ; create a buffer that can fit whole grid, plus extra buffering 


bomb_flags:
    .bomb equ 0x80
    .revealed equ 0x40
    .marked equ 0x20

screen:
    .address equ 0xb800
    .w equ 80
    .h equ 25
    .frame_delay equ 8192 * 4

keyboard:
    .up equ 0x48
    .down equ 0x50
    .left equ 0x4b
    .right equ 0x4d
    .reveal equ 0x1c
    .mark equ 0x0f
    .restart equ 0x39
    .exit equ 0x1

colors:
    .black equ 0
    .blue equ 1
    .white equ 7
    .green equ 2
    .red equ 4
    .yellow equ 14

    .revealed           equ (.white << 4)           | .black
    .marked             equ (.green << 4)           | .black
    .selected           equ (.red << 4)             | .red
    .selected_revealed  equ (.red << 4)             | .black
    .selected_marked    equ (.yellow << 4)          | .black
    .hidden             equ (colors.blue << 4)      | .blue
    .bad_bomb           equ (colors.red << 4)       | .black    
    .status_good        equ (colors.green << 4)     | .black 


game_over_text:
    db "You lost! ESC: Exit. Space: Restart", 0

game_victory_text:
    db "You win! ESC: Exit. Space: Restart", 0

game_start_text:
    db "There are ", minefield.bomb_count / 100  + '0', (minefield.bomb_count / 10) % 10  + '0', minefield.bomb_count % 10 + '0',  " bombs. Arrow keys: move. Enter: reveal. TAB: mark. ESC: exit"