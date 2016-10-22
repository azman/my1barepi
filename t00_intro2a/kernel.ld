SECTIONS {
	. = 0x8000;
	.text : { /** code segment */
		*(.boot)
		*(.text)
	}
	.data : { /** data segment */
		*(.data)
	}
	/DISCARD/ : { /** discard all other... */
		*(*)
	}
}
