// Manually vectorized memcpy

.align 2
.section .time_critical.memcpyFast8
.global memcpyFast8
.type memcpyFast8,%function
.thumb_func
// void memcpyFast8(void* __restrict__ dest, const void* __restrict__ src, size_t num);
memcpyFast8:
    push { r4 }
    movs r4, #16 // Bytes copied per iteration

    add r2, r0, r2 // End ptr in r2

    memcpyFast8_loop:
        ldrb r3, [r1]
        strb r3, [r0]

        add r0, r0, r4
        add r1, r1, r4

        cmp r0, r2
        blt memcpyFast8_loop

    pop { r4 }
    bx lr

.align 2
.section .time_critical.memcpyFast32
.global memcpyFast32
.type memcpyFast32,%function
.thumb_func
// void memcpyFast32(void* __restrict__ dest, const void* __restrict__ src, size_t num);
memcpyFast32:
    push { r4 }
    movs r4, #16 // Bytes copied per iteration

    add r2, r0, r2 // End ptr in r2

    memcpyFast32_loop:
        ldr r3, [r1]
        str r3, [r0]

        add r0, r0, r4
        add r1, r1, r4

        cmp r0, r2
        blt memcpyFast32_loop

    pop { r4 }
    bx lr

.align 2
.section .time_critical.memcpyFast128
.global memcpyFast128
.type memcpyFast128,%function
.thumb_func
// void memcpyFast128(void* __restrict__ dest, const void* __restrict__ src, size_t num);
memcpyFast128:
    push { r4 }
    movs r4, #16 // Bytes copied per iteration

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

        add r0, r0, r4
        add r1, r1, r4

        cmp r0, r2
        blt memcpyFast128_loop

    pop { r4 }
    bx lr