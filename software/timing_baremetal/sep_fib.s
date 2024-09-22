.syntax unified

.global fib
fib:
    push	{r4, r5, r6, lr}
    movs	r4, r0
    cmp	r0, #1
    ble.n   endfib
    subs	r0, #1
    bl      fib
    movs    r5, r0
    subs	r0, r4, #2
    bl      fib
    adds	r0, r5, r0
endfib:
    pop	{r4, r5, r6, pc}


.global useless
useless:
    push	{lr}
useless_loop:
    bl	useless_inner
    bl	useless_inner
    bl	useless_inner
    bl	useless_inner
    bl	useless_inner
    bl	useless_inner
    bl	useless_inner
    bl	useless_inner
    bl	useless_inner
    bl	useless_inner
    subs	r0, #1
    cmp	r0, #0
    bgt.n	useless_loop
    pop	{pc}

useless_inner:
    push	{r4, r5, r6, r7, lr}
    pop     {r4, r5, r6, r7, pc}



.global sumsq
sumsq:
    movs	r3, #0
sumsq_loop:
    movs	r2, r3
    muls	r2, r3
    adds	r3, #1
    adds	r0, r0, r2
    cmp	r3, r1
    blt.n	sumsq_loop
    bx	lr


.global matmul
matmul:
    push	{r4, r5, r6, r7, lr}
    mov	lr, sl
    mov	r7, r9
    mov	r6, r8
    push	{r6, r7, lr}
    mov	ip, r1
    movs	r7, r2
    mov	sl, r3
    movs	r5, #0
    movs	r3, #0
    mov	r8, r3
    b.n	matmul_a
matmul_b:
    movs	r3, r0
    muls	r3, r5
    adds	r3, r3, r2
    lsls	r3, r3, #2
    ldr	r3, [r3, r7]
    mov	r9, r3
    movs	r3, r0
    muls	r3, r2
    adds	r3, r3, r4
    lsls	r3, r3, #2
    mov	r6, sl
    ldr	r3, [r3, r6]
    mov	r6, r9
    muls	r3, r6
    ldr	r6, [r1, #0]
    mov	r9, r6
    add	r3, r9
    str	r3, [r1, #0]
    adds	r2, #1
matmul_d:
    cmp	r2, r0
    blt.n	matmul_b
    movs	r3, #1
    mov	r9, r3
    add	r8, r9
    adds	r4, #1
matmul_f:
    cmp	r4, r0
    bge.n	matmul_c
    mov	r3, r8
    lsls	r1, r3, #2
    add	r1, ip
    movs	r3, #0
    str	r3, [r1, #0]
    movs	r2, #0
    b.n	matmul_d
matmul_c:
    adds	r5, #1
matmul_a:
    cmp	r5, r0
    bge.n	matmul_e
    movs	r4, #0
    b.n matmul_f
matmul_e:
    pop	{r5, r6, r7}
    mov	sl, r7
    mov	r9, r6
    mov	r8, r5
    pop	{r4, r5, r6, r7, pc}



