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
    push {r4, r5, r6, r7, lr}
    LDR r4, [r0]
    ADDS r4, #0x01
    STR r4, [r0]
    pop  {r4, r5, r6, r7, pc}


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
    // r0: buf, r1: ptr to font, r2: none
    push {r4, r5, r6, r7, lr}
    
    eors r4, r4 // x counter = 0

    //ldr r5, =tokTransparents // Solid color

    movs r6, #160
    lsls r6, 2 // get 160 * 4 for buf max

    ldr r5, =0x3CF0 // FONT_HEIGHT * FONT_WIDTH_WORDS * 'a' * (4 bytes per uint32_t)


  loop:
    adds r7, r5, r1

    str r7, [r0, r4] // store at buf[x]

    adds r4, #4 // End of loop
    cmp r4, r6
    bne loop

    pop  {r4, r5, r6, r7, pc}

.align 2
aoffset: .word 0xF3C