cat <<EOF
OUTPUT_FORMAT("${OUTPUT_FORMAT}","${OUTPUT_FORMAT}","${OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})

MEMORY
{
  text   (rx)   : ORIGIN = 0, LENGTH = $TEXT_LENGTH
  bit    (rw!x) : ORIGIN = 0, LENGTH = 0x80
  data   (rw!x) : ORIGIN = 0, LENGTH = $DATA_LENGTH
  xdata  (rw!x) : ORIGIN = 0, LENGTH = $XDATA_LENGTH
  edata  (rw!x) : ORIGIN = 0, LENGTH = $EDATA_LENGTH
  eeprom (rw!x) : ORIGIN = 0, LENGTH = $EEPROM_LENGTH
}

SECTIONS
{
  /* Read-only sections, merged into text segment: */
  ${TEXT_DYNAMIC+${DYNAMIC}}
  .hash        ${RELOCATING-0} : { *(.hash)		}
  .dynsym      ${RELOCATING-0} : { *(.dynsym)		}
  .dynstr      ${RELOCATING-0} : { *(.dynstr)		}
  .gnu.version ${RELOCATING-0} : { *(.gnu.version)	}
  .gnu.version_d ${RELOCATING-0} : { *(.gnu.version_d)	}
  .gnu.version_r ${RELOCATING-0} : { *(.gnu.version_r)	}

  .rel.init    ${RELOCATING-0} : { *(.rel.init)	}
  .rela.init   ${RELOCATING-0} : { *(.rela.init)	}
  .rel.text    ${RELOCATING-0} :
    {
      *(.rel.text)
      ${RELOCATING+*(.rel.text.*)}
      ${RELOCATING+*(.rel.gnu.linkonce.t*)}
    }
  .rela.text   ${RELOCATING-0} :
    {
      *(.rela.text)
      ${RELOCATING+*(.rela.text.*)}
      ${RELOCATING+*(.rela.gnu.linkonce.t*)}
    }
  .rel.fini    ${RELOCATING-0} : { *(.rel.fini)	}
  .rela.fini   ${RELOCATING-0} : { *(.rela.fini)	}
  .rel.rodata  ${RELOCATING-0} :
    {
      *(.rel.rodata)
      ${RELOCATING+*(.rel.rodata.*)}
      ${RELOCATING+*(.rel.gnu.linkonce.r*)}
    }
  .rela.rodata ${RELOCATING-0} :
    {
      *(.rela.rodata)
      ${RELOCATING+*(.rela.rodata.*)}
      ${RELOCATING+*(.rela.gnu.linkonce.r*)}
    }
  .rel.data    ${RELOCATING-0} :
    {
      *(.rel.data)
      ${RELOCATING+*(.rel.data.*)}
      ${RELOCATING+*(.rel.gnu.linkonce.d*)}
    }
  .rela.data   ${RELOCATING-0} :
    {
      *(.rela.data)
      ${RELOCATING+*(.rela.data.*)}
      ${RELOCATING+*(.rela.gnu.linkonce.d*)}
    }
  .rel.ctors   ${RELOCATING-0} : { *(.rel.ctors)	}
  .rela.ctors  ${RELOCATING-0} : { *(.rela.ctors)	}
  .rel.dtors   ${RELOCATING-0} : { *(.rel.dtors)	}
  .rela.dtors  ${RELOCATING-0} : { *(.rela.dtors)	}
  .rel.got     ${RELOCATING-0} : { *(.rel.got)		}
  .rela.got    ${RELOCATING-0} : { *(.rela.got)		}
  .rel.bss     ${RELOCATING-0} : { *(.rel.bss)		}
  .rela.bss    ${RELOCATING-0} : { *(.rela.bss)		}
  .rel.plt     ${RELOCATING-0} : { *(.rel.plt)		}
  .rela.plt    ${RELOCATING-0} : { *(.rela.plt)		}

  /* Internal text space or external memory */

  .text :
  {
    *(.init)
    *(.progmem.gcc*)
    *(.progmem*)
    *(.text)
    *(.text.*)
    *(.fini)
    ${RELOCATING+ _etext = . ; }
  } ${RELOCATING+ > text}

  .reg	${RELOCATING+ 0} :
     ${RELOCATING+ AT (ADDR (.text) + SIZEOF (.text))}
  {
    *(.regbank)
    ${RELOCATING+ PROVIDE (__reg_start = .) ; }
    *(.rdata*)
    ${RELOCATING+ PROVIDE (__reg_end = .) ; }
  } ${RELOCATING+ > data}

  .rbss	${RELOCATING+ SIZEOF(.reg)} :
  {
    ${RELOCATING+ PROVIDE (__rbss_start = .) ; }
    *(.rbss*)
    ${RELOCATING+ PROVIDE (__rbss_end = .) ; }
  } ${RELOCATING+ > data}

  .bdata ${RELOCATING+ ((MAX (0x20, ( SIZEOF(.rbss) + ADDR(.rbss)))))} :
  {
    ${RELOCATING+ PROVIDE (__bdata_start = .) ; }
    *(.bdata*)
    ${RELOCATING+ PROVIDE (__bdata_end = .) ; }
  } ${RELOCATING+ > data}

  .bbss	${RELOCATING+ (SIZEOF(.bdata) + ADDR(.bdata))} :
  {
    ${RELOCATING+ PROVIDE (__bbss_start = .) ; }
    *(.bbss*)
    ${RELOCATING+ PROVIDE (__bbss_end = .) ; }
  } ${RELOCATING+ > data}

  .bit	${RELOCATING+ ((((SIZEOF(.bbss) + ADDR(.bbss)) - 0x20) * 8 ))} :
  {
    ${RELOCATING+ PROVIDE (__bit_start = .) ; }
    *(.bitdata*)
    ${RELOCATING+ PROVIDE (__bit_end = .) ; }
  } ${RELOCATING+ > bit}

  .bitbss ${RELOCATING+ SIZEOF(.bit) + ADDR(.bit)} :
  {
    ${RELOCATING+ PROVIDE (__bbss_start = .) ; }
    *(.bitbss*)
    ${RELOCATING+ PROVIDE (__bbss_end = .) ; }
  } ${RELOCATING+ > bit}

  .data	${RELOCATING+ (((SIZEOF(.bit) + SIZEOF(.bitbss) + 7) / 8) + SIZEOF(.bbss) + ADDR(.bbss))} :
  {
    ${RELOCATING+ PROVIDE (__data_start = .) ; }
    *(.data)
    *(.gnu.linkonce.d*)
    ${RELOCATING+ PROVIDE (__data_end = .) ; }
  } ${RELOCATING+ > data}

  .bss ${RELOCATING+ SIZEOF(.data) + ADDR(.data)} :
  {
    ${RELOCATING+ PROVIDE (__bss_start = .) ; }
    *(.bss*)
    *(COMMON)
    ${RELOCATING+ PROVIDE (__bss_end = .) ; }
  } ${RELOCATING+ > data}

  .idata ${RELOCATING+ SIZEOF(.bss) + ADDR(.bss)} :
  {
    ${RELOCATING+ PROVIDE (__idata_start = .) ; }
    *(.idata)
    ${RELOCATING+ PROVIDE (__idata_end = .) ; }
  } ${RELOCATING+ > data}

  .ibss ${RELOCATING+ SIZEOF(.idata) + ADDR(.idata)} :
  {
    ${RELOCATING+ PROVIDE (__ibss_start = .) ; }
    *(.ibss)
    ${RELOCATING+ PROVIDE (__ibss_end = .) ; }
    ${RELOCATING+ PROVIDE (stack = .) ; }
  } ${RELOCATING+ > data}

  .xdata ${RELOCATING-0}:
  {
    ${RELOCATING+ PROVIDE (__xdata_start = .) ; }
    *(.xdata*)
    ${RELOCATING+ PROVIDE (__xdata_end = .) ; }
  } ${RELOCATING+ > xdata}

  .ixdata ${RELOCATING+ SIZEOF(.xdata)}:
  {
    ${RELOCATING+ PROVIDE (__ixdata_start = .) ; }
    *(.xbss*)
    ${RELOCATING+ PROVIDE (__ixdata_end = .) ; }
  } ${RELOCATING+ > xdata}

  .edata ${RELOCATING-0}:
  {
    ${RELOCATING+ PROVIDE (__edata_start = .) ; }
    *(.edata*)
    ${RELOCATING+ PROVIDE (__edata_end = .) ; }
  } ${RELOCATING+ > edata}

  .iedata ${RELOCATING+ SIZEOF(.edata)}:
  {
    ${RELOCATING+ PROVIDE (__iedata_start = .) ; }
    *(.ebss*)
    ${RELOCATING+ PROVIDE (__iedata_end = .) ; }
  } ${RELOCATING+ > edata}

  .eeprom ${RELOCATING-0}:
  {
    ${RELOCATING+ PROVIDE (__eeprom_start = .) ; }
    *(.eeprom*)
    ${RELOCATING+ PROVIDE (__eeprom_end = .) ; }
  } ${RELOCATING+ > eeprom}

  /* Stabs debugging sections.  */
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment 0 : { *(.comment) }
 
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */

  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }

  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }

  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }

  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info) *(.gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
}
EOF

