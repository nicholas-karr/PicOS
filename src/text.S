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




.align 2
.section .time_critical.render_relevant_text_boxes
.global render_relevant_text_boxes
.type render_relevant_text_boxes,%function
.thumb_func
// move 160 pointers to 8 pixels into buf
// void render_relevant_text_boxes(uint32_t* buf, uint32_t* font, TextLineEntry* entries)
render_relevant_text_boxes:
    // r0: buf, r1: ptr to font, r2: entries
    push {r4, r5, r6, r7, r1} // This is a leaf function, don't touch lr

    subs r2, #12 // undo moving end of loop to front

    loop_entries:
      adds r2, #12 // redundant end of loop

      // Resolve first entry
      // r4: loop end, r5: characters
      ldr r4, [r2, #4]

      // Exit early for sentinel entry c_end == 0
      cmp r4, #0
      beq end

      // Resolve cbuf to char*
      ldr r5, [r2]

      // Go to transparent for cbuf == 0
      cmp r5, #0
      beq transparent_entry

      // adjust r1
      //ldr r3, [r2, #8] // Load y of string (4 * (y % 15))
      //ldr r1, [sp] // Load back font pointer that got clobbered
      ldr r1, [r2, #8] // Load font ptr
      
      //adds r1, r3

      movs r3, #240 // Font scalar
      
      text_entry:
        loop_text:
          ldr r6, [r5] // Get 4 chars to render
          adds r5, #4 // incr cursor

          mov r7, r6 // Grab all chars
          lsrs r6, #8 // Shift array over for next time
          uxtb r7, r7 // Get only first byte
          muls r7, r3, r7 // Multiply by font scalar
          adds r7, r1 // Add font address
          str r7, [r0] // store at buf[x]

          mov r7, r6
          lsrs r6, #8
          uxtb r7, r7
          muls r7, r3, r7
          adds r7, r1
          str r7, [r0, #4]

          mov r7, r6
          lsrs r6, #8
          uxtb r7, r7
          muls r7, r3
          adds r7, r1
          str r7, [r0, #8]

          muls r6, r3
          adds r6, r1
          str r6, [r0, #12]

          adds r0, #16
          cmp r0, r4
          bne loop_text // could be bge

        b loop_entries

      transparent_entry:
        ldr r6, =tokTransparents // Get compiler to load the 32-bit address of tokTransparents

        loop_transparent:
          str r6, [r0]
          //str r6, [r0, #4]
          //str r6, [r0, #8]
          //str r6, [r0, #12]

          adds r0, #4
          cmp r0, r4
          bne loop_transparent

        b loop_entries


    end:
    pop  {r1, r4, r5, r6, r7}
    bx lr