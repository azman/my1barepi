SECTIONS {
	. = 0x8000;
	.text : { /** code segment */
		KEEP(*(.boot))
		KEEP(*(.text.startup))
		*(.text)
	}
	.data : { /** data segment */
		*(.data)
		*(COMMON)
	}
	/DISCARD/ : { /** discard all other... */
		*(*)
	}
}
