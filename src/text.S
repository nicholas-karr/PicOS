.globl tokTransparents

.syntax unified
.cpu cortex-m0plus
.thumb

.align 2
.section .time_critical.testfunc
.global testfunc
.type testfunc,%function
.thumb_func
// move 160 pointers to 8 pixels into buf
// relevant: sorted by x_start
// void testfunc(uint32_t* buf)
testfunc:
    ldr r2, [r2] // resolve to pointer to first text box
    ldr r3, [r2, #8] // get cursor (don't store back for now)
    ldr r2, [r2] // resolve to [x, x_max]

    eors r0, r0
    ldrb r0, [r3] // Get char to render

    //mov r0, r3

    bx lr // return


// r1 buf
// r2 font
// r3 relevant

.align 2
.section .time_critical.render_relevant_text_boxes
.global render_relevant_text_boxes
.type render_relevant_text_boxes,%function
.thumb_func
// move 160 pointers to 8 pixels into buf
// relevant: sorted by x_start
// void render_relevant_text_boxes(uint32_t* buf, uint32_t* font, TextBox** relevant)
render_relevant_text_boxes:
    // r0: buf, r1: ptr to font, r2: relevant
    // r3: cursor, r4: x, r5: char to render?
    // r6: end of buf r7: char offset
    push {r4, r5, r6, r7, lr}

    movs r6, #160
    lsls r6, 2 // get 160 * 4 for buf max
    adds r6, r0 // make r6 into loop max

    ldr r2, [r2] // resolve to pointer to first text box
    ldr r3, [r2, #8] // get cursor (don't store back for now)
    ldr r2, [r2] // resolve to [x, x_max]

    //ldr r2, =tokTransparents // Solid color
    ldr r2, =240


  loop:
    ldrb r5, [r3] // Get char to render
    adds r3, #1 // incr cursor

    mov r7, r2
    muls r7, r5
    adds r7, r1 // dbase + char_offset

    str r7, [r0] // store at buf[x]

    //str r2, [r0]

    adds r0, #4 // End of loop
    cmp r0, r6
    bne loop

    pop  {r4, r5, r6, r7, pc}

.align 2
aoffset: .word 0xF3C