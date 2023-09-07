.align 2
.section .time_critical.memcpyFast8
.global memcpyFast8
.type memcpyFast8,%function
.thumb_func
// void memcpyFast8(void* dest, void* src, size_t num);
memcpyFast8:
    add r2, r0, r2 // End ptr in r2

    memcpyFast8_loop:
        ldrb r3, [r1]
        str r3, [r0]

        movs r3, #1
        add r0, r3
        cmp r0, r2
        blt memcpyFast8_loop

    bx lr

.align 2
.section .time_critical.memcpyFast128
.global memcpyFast128
.type memcpyFast128,%function
.thumb_func
// void memcpyFast128(void* dest, void* src, size_t num);
memcpyFast128:
    add r2, r0, r2 // End ptr in r2

    memcpyFast128_loop:
        ldr r3, [r1]
        str r3, [r0]

        ldr r3, [r1, #4]
        str r3, [r0, #4]

        ldr r3, [r1, #8]
        str r3, [r0, #8]

        ldr r3, [r1, #12]
        str r3, [r0, #12]

        movs r3, #16
        add r0, r3
        cmp r0, r2
        blt memcpyFast128_loop

    bx lr