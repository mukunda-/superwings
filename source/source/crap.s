.global __get_bss_start, __get_bss_end, __get_eheap_start, __get_eheap_end

.thumb_func
__get_bss_start:
	ldr	r0,=__bss_start
	bx	lr

.thumb_func
__get_bss_end:
	ldr	r0,=__bss_end
	bx	lr

.thumb_func
__get_eheap_start:
	ldr	r0,=__eheap_start
	bx	lr

.thumb_func
__get_eheap_end:
	ldr	r0,=__eheap_end
	bx	lr
