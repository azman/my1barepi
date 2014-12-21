SECTIONS {
	. = 0x8000;
	.text : { /** code segment */
		KEEP(*(.boot))
		KEEP(*(.text.startup))
		*(.text)
		*(.rodata*)
	}
	.data : { /** data segment */
		*(COMMON)
		*(.data)
	}
	/DISCARD/ : { /** discard all other... */
		*(*)
	}
}
