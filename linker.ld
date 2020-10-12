SECTIONS {
	. = 0x8000;
	.text : { /** code segment */
		KEEP(*(.boot))
		KEEP(*(.load))
		KEEP(*(.text.startup))
		*(.text)
		*(.rodata*)
	}
	.data : { /** data segment */
		*(COMMON)
		*(.data)
		*(.bss)
	}
	/DISCARD/ : { /** discard all other... */
		*(*)
	}
}
