OUTPUT_FORMAT("elf32-i51","elf32-i51","elf32-i51")
OUTPUT_ARCH(i51)
ENTRY(_resetvec)
PROVIDE(l_IRAM = 0x100);

MEMORY
{
  text   (rwx)   : ORIGIN = 0, LENGTH = 64K
  bit    (rw!x) : ORIGIN = 0, LENGTH = 0x80
  data   (rw!x) : ORIGIN = 0, LENGTH = 256
  pdata   (rw!x) : ORIGIN = 0, LENGTH = 256
/*  xdata  (rw!x) : ORIGIN = 0x8000, LENGTH = 32K */
  edata  (rw!x) : ORIGIN = 0, LENGTH = 64K
  eeprom (rw!x) : ORIGIN = 0, LENGTH = 2K
}
SECTIONS
{

  
  /* Read-only sections, merged into text segment: */
  .hash          : { *(.hash)		}
  .dynsym        : { *(.dynsym)		}
  .dynstr        : { *(.dynstr)		}
  .gnu.version   : { *(.gnu.version)	}
  .gnu.version_d   : { *(.gnu.version_d)	}
  .gnu.version_r   : { *(.gnu.version_r)	}
  .rel.init      : { *(.rel.init)	}
  .rela.init     : { *(.rela.init)	}
  .rel.text      :
    {
      *(.rel.text)
      *(.rel.text.*)
      *(.rel.gnu.linkonce.t*)
    }
  .rela.text     :
    {
      *(.rela.text)
      *(.rela.text.*)
      *(.rela.gnu.linkonce.t*)
    }
  .rel.fini      : { *(.rel.fini)	}
  .rela.fini     : { *(.rela.fini)	}
  .rel.rodata    :
    {
      *(.rel.rodata)
      *(.rel.rodata.*)
      *(.rel.gnu.linkonce.r*)
    }
  .rela.rodata   :
    {
      *(.rela.rodata)
      *(.rela.rodata.*)
      *(.rela.gnu.linkonce.r*)
    }
  .rel.data      :
    {
      *(.rel.data)
      *(.rel.data.*)
      *(.rel.gnu.linkonce.d*)
    }
  .rela.data     :
    {
      *(.rela.data)
      *(.rela.data.*)
      *(.rela.gnu.linkonce.d*)
    }
  .rel.ctors     : { *(.rel.ctors)	}
  .rela.ctors    : { *(.rela.ctors)	}
  .rel.dtors     : { *(.rel.dtors)	}
  .rela.dtors    : { *(.rela.dtors)	}
  .rel.got       : { *(.rel.got)		}
  .rela.got      : { *(.rela.got)		}
  .rel.bss       : { *(.rel.bss)		}
  .rela.bss      : { *(.rela.bss)		}
  .rel.plt       : { *(.rel.plt)		}
  .rela.plt      : { *(.rela.plt)		}
  /* Internal text space or external memory */
  .reset :
  {
    PROVIDE(_resetvec = .);
    KEEP (*(.reset))
  } > text =0x0
  .isr00 0x03 :  { KEEP (*(.isr00)) } > text
  .isr01 0x0b :  { KEEP (*(.isr01)) } > text
  .isr02 0x13 :  { KEEP (*(.isr02)) } > text
  .isr03 0x1b:  { KEEP (*(.isr03)) } > text
  .isr04 0x23:  { KEEP (*(.isr04)) } > text
  .isr05 0x2b:  { KEEP (*(.isr05)) } > text
  .isr06 0x33:  { KEEP (*(.isr06)) } > text
  .isr07 0x3b:  { KEEP (*(.isr07)) } > text
  .isr08 0x43:  { KEEP (*(.isr08)) } > text 
  .isr09 0x4b:  { KEEP (*(.isr09)) } > text 
  .isr10 0x53:  { KEEP (*(.isr10)) } > text 
  .isr11 0x5b:  { KEEP (*(.isr11)) } > text
  .isr12 0x63:  { KEEP (*(.isr12)) } > text
  .isr13 0x6B :  { KEEP (*(.isr13)) } > text 
  .isr14 0x73 :  { KEEP (*(.isr14)) } > text 
  .gsinit 0x100 :
  {
    PROVIDE(__init_table = .);
    KEEP(*(.text.gsinit))
    BYTE(0x22);
    SHORT(ADDR(.gsinit));
  }  > text
  
  .text SIZEOF(.gsinit) + ADDR(.gsinit) :
  {
    *(.init)
    *(.text.gsinit0)
    *(.text.gsinit1)
    *(.text.gsinit2)
    *(.text.gsinit3)
    *(.text.gsinit4)
    *(.text.gsinit5)
    *(.text.gsfinal)
    *(.text)
    *(.text.*)
    *(.fini)
/*    PROVIDE(__init_table = .) ;
    SHORT(ADDR(.gsinit));
    SHORT(-1); */
     _etext = . ; 
  }  > text
  
  .roxdata SIZEOF(.text) + ADDR(.text):
  {
    __roxdata_start = .;
    KEEP(*(.roxdata.*))
    *(.constnamed.*)
    __roxdata_end = .;
  }  > text
  
    .pdata ALIGN(0x100) (NOLOAD):
  {
    . = . + 1;
    __pdata_start = . ; 
    *(.pdata*)
     __pdata_end = . ; 
     PROVIDE (__xstack_start = .) ; 
  }  > text
  .xdata ALIGN(0x100) (NOLOAD):
  {
     __xdatac_start = . ; 
    *(.xdata.i51*)
    *(.datanamed.*)
    *(.xdata.abs*)
     __xdatac_end = . ; 
     __xdatai_start = . ; 
    KEEP(*(.xdata.init*))
     __xdatai_end = . ; 
  }  > text
  .ixdata  (NOLOAD):
  {
     PROVIDE (__ixdata_start = .) ; 
    *(.xbss*)
     PROVIDE (__ixdata_end = .) ; 
  }  > text

  regsize = DEFINED(__RB3__) ? 0x20 : (DEFINED(__RB2__) ? 0x18 : (DEFINED(__RB1__) ? 0x10 : 0x8));
  .reg	 0 (NOLOAD): 
  {
     PROVIDE (__reg_start = .) ; 
    KEEP(*(.regbank))
     . = regsize;
     PROVIDE (__reg_end = .) ; 
  }  > data
  .bits	0x0  (NOLOAD):
  {
     PROVIDE (__bit_start = .) ; 
	*(.bits*)
     PROVIDE (__bit_end = .) ; 
  }  > bit
  .dbit 0x20 (NOLOAD):
  {
  *(.dbit*)
  } > data
  .bit	(MAX(SIZEOF(.bits),SIZEOF(.dbit)*8))  (NOLOAD):
  {
     PROVIDE (__bit_start = .) ; 
	*(.bitdata*)
     PROVIDE (__bit_end = .) ; 
  }  > bit
  .ddata  (((SIZEOF(.bit) + MAX(SIZEOF(.bits),SIZEOF(.dbit)*8)  + 7) / 8) + ((SIZEOF(.dbit) + SIZEOF(.bit) + SIZEOF(.bits)) ? 0x20 : SIZEOF(.reg))) (NOLOAD):
  {
     PROVIDE (__ddata_start = .) ; 
    *(.ddata*)
    *(.gnu.linkonce.d*)
     __ddata_end = . ; 
  }  > data
  .bss  SIZEOF(.ddata) + ADDR(.ddata) (NOLOAD):
  {
     PROVIDE (__bss_start = .) ; 
    *(.bss*)
    *(COMMON)
     PROVIDE (__bss_end = .) ; 
  }  > data
  .idata  SIZEOF(.bss) + ADDR(.bss) (NOLOAD):
  {
     PROVIDE (__idata_start = .) ; 
    *(.idata*)
     PROVIDE (__idata_end = .) ; 
  }  > data
  .ibss  SIZEOF(.idata) + ADDR(.idata) (NOLOAD):
  {
     PROVIDE (__ibss_start = .) ; 
    *(.ibss*)
     PROVIDE (__ibss_end = .) ; 
     PROVIDE (__stack_start = .) ; 
  }  > data
  .edata  (NOLOAD):
  {
     PROVIDE (__edata_start = .) ; 
    *(.edata*)
     PROVIDE (__edata_end = .) ; 
  }  > edata
  s_PSEG = __pdata_start;
  l_PSEG = __pdata_end - __pdata_start;
  s_XSEG = __xdatac_start;
  l_XSEG = __xdatac_end - __xdatac_start;
  s_XISEG = __xdatai_start;
  l_XISEG = __xdatai_end - __xdatai_start;
  s_XINIT = __roxdata_start;
  l_XINIT = __roxdata_end - __roxdata_start;
  _. = ASSERT ((__stack_start < 0xF0), "Stack Iram above limit 0xF0") ;
  _. = ASSERT ((__ddata_end < 0x80), "Direct Iram Data above Limit 0x80") ;
  .iedata  SIZEOF(.edata) (NOLOAD):
  {
     PROVIDE (__iedata_start = .) ; 
    *(.ebss*)
     PROVIDE (__iedata_end = .) ; 
  }  > edata
  .eeprom  (NOLOAD):
  {
     PROVIDE (__eeprom_start = .) ; 
    *(.eeprom*)
     PROVIDE (__eeprom_end = .) ; 
  }  > eeprom
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
