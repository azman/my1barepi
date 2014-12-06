SECTIONS {
	. = 0x8000;
	.text : { /** code segment */
		KEEP(*(.text.startup))
		*(.text)
	}
	/* . = ALIGN(4096); align to 4K page size */
	.data : { /** data segment */
		*(.data)
		*(COMMON)
	}
	/* . = ALIGN(4096); */
	/DISCARD/ : { /** discard all other... */
		*(*)
	}
}
