/* MCS-51 specific support for 32-bit ELF
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Radek Benedikt <benedikt@lphard.cz>

This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "bfd.h"
#include "sysdep.h"
#include "ctype.h"
#include "libbfd.h"
#include "bfdlink.h"
#include "genlink.h"
#include "elf-bfd.h"
#include "elf/i51.h"

#if !defined(HTC_TRICORE) && !defined(HTC_PPCVLE)
#include "elf32-htc.c"
#endif

//static reloc_howto_type *bfd_elf32_bfd_reloc_type_lookup
//  PARAMS ((bfd *abfd, bfd_reloc_code_real_type code));
static void i51_info_to_howto_rela
  PARAMS ((bfd *, arelent *, Elf_Internal_Rela *));
static asection *elf32_i51_gc_mark_hook
  PARAMS ((asection *, struct bfd_link_info *, Elf_Internal_Rela *,
	   struct elf_link_hash_entry *, Elf_Internal_Sym *));
static bfd_boolean elf32_i51_gc_sweep_hook
  PARAMS ((bfd *, struct bfd_link_info *, asection *,
	   const Elf_Internal_Rela *));
static bfd_boolean elf32_i51_check_relocs
  PARAMS ((bfd *, struct bfd_link_info *, asection *,
	   const Elf_Internal_Rela *));
static bfd_reloc_status_type i51_final_link_relocate
  PARAMS ((reloc_howto_type *, bfd *, asection *, bfd_byte *,
	   Elf_Internal_Rela *, bfd_vma));
static bfd_boolean elf32_i51_relocate_section
  PARAMS ((bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
	   Elf_Internal_Rela *, Elf_Internal_Sym *, asection **));
static void bfd_elf_i51_final_write_processing 
  PARAMS ((bfd *, bfd_boolean));
static bfd_boolean elf32_i51_object_p
  PARAMS ((bfd *));
void elf32_i51_symbol_processing
  PARAMS ((bfd *, asymbol *));
bfd_boolean elf32_i51_section_from_bfd_section
  PARAMS ((bfd *, asection *, int *));
static bfd_boolean elf32_i51_add_symbol_hook
  PARAMS ((bfd *, struct bfd_link_info *, Elf_Internal_Sym *,
	   const char **, flagword *, asection **, bfd_vma *));
static bfd_boolean i51_elf_section_from_shdr (bfd *, Elf_Internal_Shdr *,const char *, int);
static bfd_boolean i51_elf_fake_sections (bfd *,Elf_Internal_Shdr *,asection *asect);
/* Use RELA instead of REL */
#undef USE_REL

static reloc_howto_type elf_i51_howto_table[] =
{
  HOWTO (R_I51_NONE,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_I51_NONE",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 1 bit indirect register relocation. */
  HOWTO (R_I51_R1,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_I51_R1",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 3 bit register relocation. */
  HOWTO (R_I51_R3,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_I51_R3",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 7 bit PC relative relocation. */
  HOWTO (R_I51_7_PCREL,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_I51_7_PCREL",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0000,		/* src_mask */
	 0x00ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 11 bit absolute inpage relocation. */
  HOWTO (R_I51_11,		/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_I51_11",		/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 8 bit bit register relocation. */
  HOWTO (R_I51_8_BIT,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_I51_8_BIT",		/* name */
	 FALSE,			/* partial_inplace */
	 0x00ff,		/* src_mask */
	 0x00ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 8 bit absolute relocation. */
  HOWTO (R_I51_8,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_I51_8",		/* name */
	 FALSE,			/* partial_inplace */
	 0x00ff,		/* src_mask */
	 0x00ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 8 bit absolute LOW(word) relocation. */
  HOWTO (R_I51_L,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_I51_L",		/* name */
	 FALSE,			/* partial_inplace */
	 0x00ff,		/* src_mask */
	 0x00ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 8 bit absolute HIGH(word) relocation. */
  HOWTO (R_I51_H,		/* type */
	 8,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_I51_H",		/* name */
	 FALSE,			/* partial_inplace */
	 0x00ff,		/* src_mask */
	 0x00ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 16 bit absolute relocation.  */
  HOWTO (R_I51_16,		/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_I51_16",		/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 8 bit Byte2Bit relocation. */
  HOWTO (R_I51_8_B2B,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 8,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_I51_8_B2B",		/* name */
	 FALSE,			/* partial_inplace */
	 0x00ff,		/* src_mask */
	 0x00ff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* A 13 bit absolute pcode relocation. */
  HOWTO (R_I51_13_PCODE,	/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_I51_13_PCODE",		/* name */
	 FALSE,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 FALSE)		        /* pcrel_offset */
};

/* Map BFD reloc types to I51 ELF reloc types.  */

struct i51_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int elf_reloc_val;
};

 static const struct i51_reloc_map i51_reloc_map[] =
{
  { BFD_RELOC_NONE,                 R_I51_NONE },
  { BFD_RELOC_I51_R1,               R_I51_R1 },
  { BFD_RELOC_I51_R3,               R_I51_R3 },
  { BFD_RELOC_I51_7_PCREL,          R_I51_7_PCREL },
  { BFD_RELOC_I51_11,		    R_I51_11 },
  { BFD_RELOC_I51_8_BIT,	    R_I51_8_BIT },
  { BFD_RELOC_8,                    R_I51_8 },
  { BFD_RELOC_I51_8_LOW,            R_I51_L },
  { BFD_RELOC_I51_8_HIGH,           R_I51_H },
  { BFD_RELOC_16,                   R_I51_16 },
  { BFD_RELOC_I51_8_B2B,	    R_I51_8_B2B },
  { BFD_RELOC_I51_13_PCODE,	    R_I51_13_PCODE }
};


 static reloc_howto_type *
 i51_elf_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
                const char *r_name)
 {
   unsigned int i;

   for (i = 0;
        i < sizeof (elf_i51_howto_table) / sizeof (elf_i51_howto_table[0]);
        i++)
     if (elf_i51_howto_table[i].name != NULL
     && strcasecmp (elf_i51_howto_table[i].name, r_name) == 0)
       return &elf_i51_howto_table[i];

   return NULL;
 }

static reloc_howto_type *
i51_elf32_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
                                       bfd_reloc_code_real_type code)
 {
   unsigned int i;

//   if (code == BFD_RELOC_8)
//     code = BFD_RELOC_TRICORE_8ABS;
//   else if (code == BFD_RELOC_16)
//     code = BFD_RELOC_TRICORE_16ABS;
//   else if (code == BFD_RELOC_32)
//     code = BFD_RELOC_TRICORE_32ABS;
//   else if (code == BFD_RELOC_32_PCREL)
//     code = BFD_RELOC_TRICORE_32REL;
//   else if (code == BFD_RELOC_16_PCREL)
//     code = BFD_RELOC_TRICORE_PCREL16;
//   else if (code == BFD_RELOC_8_PCREL)
//     code = BFD_RELOC_TRICORE_PCREL8;

   for (i = 0; i < sizeof (i51_reloc_map) / sizeof (i51_reloc_map[0]); ++i)
     if (i51_reloc_map[i].bfd_reloc_val == code)
       return &elf_i51_howto_table[i51_reloc_map[i].elf_reloc_val];

   bfd_set_error (bfd_error_bad_value);
   return (reloc_howto_type *) 0;
 }

//static reloc_howto_type *
//bfd_elf32_bfd_reloc_type_lookup (abfd, code)
//     bfd *abfd ATTRIBUTE_UNUSED;
//     bfd_reloc_code_real_type code;
//{
//  unsigned int i;
//
//  for (i = 0;
//       i < sizeof (i51_reloc_map) / sizeof (struct i51_reloc_map);
//       i++)
//    {
//      if (i51_reloc_map[i].bfd_reloc_val == code)
//	return &elf_i51_howto_table[i51_reloc_map[i].elf_reloc_val];
//    }
//  return NULL;
//}

/* Set the howto pointer for an I51 ELF reloc.  */

static void
i51_info_to_howto_rela (abfd, cache_ptr, dst)
     bfd *abfd ATTRIBUTE_UNUSED;
     arelent *cache_ptr;
     Elf_Internal_Rela *dst;
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  BFD_ASSERT (r_type < (unsigned int) R_I51_max);
  cache_ptr->howto = &elf_i51_howto_table[r_type];
}

static asection *
elf32_i51_gc_mark_hook (sec, info, rel, h, sym)
     asection *sec;
     struct bfd_link_info *info ATTRIBUTE_UNUSED;
     Elf_Internal_Rela *rel;
     struct elf_link_hash_entry *h;
     Elf_Internal_Sym *sym;
{
//  if (h != NULL)
//    {
//      switch (ELF32_R_TYPE (rel->r_info))
//	{
//	default:
//	  switch (h->root.type)
//	    {
//	    case bfd_link_hash_defined:
//	    case bfd_link_hash_defweak:
//	      return h->root.u.def.section;
//
//	    case bfd_link_hash_common:
//	      return h->root.u.c.p->section;
//
//	    default:
//	      break;
//	    }
//	}
//    }
//  else
//    {
//
////      if (!(elf_bad_symtab (abfd)
////	    && ELF_ST_BIND (sym->st_info) != STB_LOCAL)
////	  && !((sym->st_shndx <= 0 || sym->st_shndx >= SHN_LORESERVE)
////	       && sym->st_shndx != SHN_COMMON))
////	{
////	  return bfd_section_from_elf_index (abfd, sym->st_shndx);
////	}
//    }
//  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);

       return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);

}

static bfd_boolean
elf32_i51_gc_sweep_hook (abfd, info, sec, relocs)
     bfd *abfd ATTRIBUTE_UNUSED;
     struct bfd_link_info *info ATTRIBUTE_UNUSED;
     asection *sec ATTRIBUTE_UNUSED;
     const Elf_Internal_Rela *relocs ATTRIBUTE_UNUSED;
{
  /* We don't use got and plt entries for i51.  */
  return TRUE;
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

static bfd_boolean
elf32_i51_check_relocs (bfd *abfd,
                        struct bfd_link_info *info,
                        asection *sec,
                        const Elf_Internal_Rela *relocs)
{
#if 1
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes, **sym_hashes_end;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;

  if (info->relocatable)
    return TRUE;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  sym_hashes_end = sym_hashes + symtab_hdr->sh_size/sizeof (Elf32_External_Sym);
  if (!elf_bad_symtab (abfd))
    sym_hashes_end -= symtab_hdr->sh_info;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
        h = NULL;
      else
        h = sym_hashes[r_symndx - symtab_hdr->sh_info];
    }

  return TRUE;
#endif
#if 0
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;

  if (info->relocatable)
    return TRUE;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
        h = NULL;
      else
        {
          h = sym_hashes[r_symndx - symtab_hdr->sh_info];
          while (h->root.type == bfd_link_hash_indirect
                 || h->root.type == bfd_link_hash_warning)
            h = (struct elf_link_hash_entry *) h->root.u.i.link;
        }
    }

  return TRUE;
#endif
}

/* Perform a single relocation.  By default we use the standard BFD
   routines, but a few relocs, we have to do them ourselves.  */
#if 1
static bfd_reloc_status_type
i51_final_link_relocate (reloc_howto_type *                 howto,
                         bfd *                              input_bfd,
                         asection *                         input_section,
                         bfd_byte *                         contents,
                         Elf_Internal_Rela *                rel,
                         bfd_vma                            relocation)
{
  bfd_reloc_status_type r = bfd_reloc_ok;
  bfd_vma               x;
  bfd_signed_vma	srel;

  switch (howto->type)
    {
    case R_I51_7_PCREL:
      contents += rel->r_offset;
      srel = (bfd_signed_vma) relocation;
      srel += rel->r_addend;
      srel -= rel->r_offset;
      srel -= (input_section->output_section->vma +
	       input_section->output_offset);
/*
      fprintf (stderr, "7PCREL contents: %lx rel->r_offset: %lx relocation: %lx\n",
	       contents, rel->r_offset, (bfd_signed_vma)relocation);
      fprintf (stderr, "7PCREL rel->r_addend: %lx srel: %lx\n",
	       rel->r_addend, srel);
      fprintf (stderr, "7PCREL input_section->output_section->vma: %lx input_section->output_offset: %lx\n",
	       input_section->output_section->vma, input_section->output_offset);
*/

      /* Check for overflow.  */
      if (srel > ((1 << 7) - 1) || (srel < - (1 << 7)))
	return bfd_reloc_overflow;

      //      x = bfd_get_8 (input_bfd, contents);
      bfd_put_8 (input_bfd, srel, contents);
      break;

    case R_I51_11:
      contents   += rel->r_offset;
      srel = (bfd_signed_vma) relocation;
      srel += rel->r_addend;
      /*
      fprintf (stderr, "R11 contents: %lx rel->r_offset: %lx relocation: %lx\n",
	       contents, rel->r_offset, (bfd_signed_vma)relocation);
      fprintf (stderr, "R11 rel->r_addend: %lx srel: %lx\n",
	       rel->r_addend, srel);
      fprintf (stderr, "R11 input_section->output_section->vma: %lx input_section->output_offset: %lx\n",
	       input_section->output_section->vma, input_section->output_offset);
      */
      /* Check for overflow.  */
      if (((srel ^ (rel->r_offset + input_section->output_section->vma + input_section->output_offset)) & 0x0000F800l) !=0)
	{
	  /*
	  fprintf ("Err: %lx %lx %lx\n", srel,
		   (rel->r_offset + input_section->output_section->vma + input_section->output_offset),
		   ((srel ^ (rel->r_offset + input_section->output_section->vma + input_section->output_offset)) & 0x0000F800l));
	  */
	  return bfd_reloc_overflow;
	}

      srel = (((srel & 0x0700) << 5) | (srel & 0x00FF));
      x = bfd_get_16 (input_bfd, contents);
      x = (x & 0x1F00) | srel;
      bfd_put_16 (input_bfd, x, contents);
      break;

    case R_I51_8_B2B:
      contents += rel->r_offset;
      srel = (bfd_signed_vma) relocation;
      srel += rel->r_addend;
      x = bfd_get_8 (input_bfd, contents);
      /* Check for overflow.  */
      if (srel < 0x20) return bfd_reloc_outofrange;
      if ((srel < 0x30) && (((srel - 0x20) * 8 + x) > 0x80)) return bfd_reloc_overflow;
      if ((srel >= 0x30) && (srel < 0x80)) return bfd_reloc_outofrange;
      if ((srel + x) > 0x100) return bfd_reloc_overflow;
      if (srel < 0x30)
	bfd_put_8 (input_bfd, ((srel - 0x20) * 8 + x), contents);
      else
	bfd_put_8 (input_bfd, (srel + x), contents);
      break;

    case R_I51_13_PCODE:
      contents   += rel->r_offset;
      srel = (bfd_signed_vma) relocation;
      srel += rel->r_addend;
      /* Check for overflow.  */
      if (srel < 0x0100) return bfd_reloc_outofrange;
      if (srel > 0x1FFF) return bfd_reloc_overflow;

      x = bfd_get_16 (input_bfd, contents);
      x = (x & 0xE000) | srel;
      bfd_put_16 (input_bfd, x, contents);
      break;

    default:
      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				    contents, rel->r_offset,
				    relocation, rel->r_addend);
    }

  return r;
}
#endif
#if 1
/* Relocate an I51 ELF section.  */
static bfd_boolean
elf32_i51_relocate_section (bfd *output_bfd ATTRIBUTE_UNUSED,
                            struct bfd_link_info *info,
                            bfd *input_bfd,
                            asection *input_section,
                            bfd_byte *contents,
                            Elf_Internal_Rela *relocs,
                            Elf_Internal_Sym *local_syms,
                            asection **local_sections)
{
  Elf_Internal_Shdr *           symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  Elf_Internal_Rela *           rel;
  Elf_Internal_Rela *           relend;

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend     = relocs + input_section->reloc_count;

  for (rel = relocs; rel < relend; rel ++)
    {
      reloc_howto_type *           howto;
      unsigned long                r_symndx;
      Elf_Internal_Sym *           sym;
      asection *                   sec;
      struct elf_link_hash_entry * h;
      bfd_vma                      relocation;
      bfd_reloc_status_type        r;
      const char *                 name = NULL;
      int                          r_type;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);

      if (info->relocatable)
	{
	  /* This is a relocateable link.  We don't have to change
             anything, unless the reloc is against a section symbol,
             in which case we have to adjust according to where the
             section symbol winds up in the output section.  */
	  if (r_symndx < symtab_hdr->sh_info)
	    {
	      sym = local_syms + r_symndx;

	      if (ELF_ST_TYPE (sym->st_info) == STT_SECTION)
		{
		  sec = local_sections [r_symndx];
		  rel->r_addend += sec->output_offset + sym->st_value;
		}
	    }

	  continue;
	}

      /* This is a final link.  */
      howto  = elf_i51_howto_table + ELF32_R_TYPE (rel->r_info);
      h      = NULL;
      sym    = NULL;
      sec    = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections [r_symndx];
	  relocation = (sec->output_section->vma
			+ sec->output_offset
			+ sym->st_value);

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = (name == NULL) ? bfd_section_name (input_bfd, sec) : name;
	}
      else
	{
	  h = sym_hashes [r_symndx - symtab_hdr->sh_info];

	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  name = h->root.root.string;

	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    {
	      sec = h->root.u.def.section;
	      relocation = (h->root.u.def.value
			    + sec->output_section->vma
			    + sec->output_offset);
	    }
	  else if (h->root.type == bfd_link_hash_undefweak)
	    {
	      relocation = 0;
	    }
	  else
	    {
	      if (! ((*info->callbacks->undefined_symbol)
		     (info, h->root.root.string, input_bfd,
		      input_section, rel->r_offset, TRUE)))
		return FALSE;
	      relocation = 0;
	    }
	}

      r = i51_final_link_relocate (howto, input_bfd, input_section,
				   contents, rel, relocation);

      if (r != bfd_reloc_ok)
	{
	  const char * msg = (const char *) NULL;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      r = info->callbacks->reloc_overflow
		(info, (h ? &h->root: NULL), name, howto->name, (bfd_vma) 0,
		 input_bfd, input_section, rel->r_offset);
	      break;
	    case bfd_reloc_undefined:
	      r = info->callbacks->undefined_symbol
		(info, name, input_bfd, input_section, rel->r_offset, TRUE);
	      break;

	    case bfd_reloc_outofrange:
	      msg = _("internal error: out of range error");
	      break;

	    case bfd_reloc_notsupported:
	      msg = _("internal error: unsupported relocation error");
	      break;

	    case bfd_reloc_dangerous:
	      msg = _("internal error: dangerous relocation");
	      break;

	    default:
	      msg = _("internal error: unknown error");
	      break;
	    }

	  if (msg)
	    r = info->callbacks->warning
	      (info, msg, name, input_bfd, input_section, rel->r_offset);

	  if (! r)
	    return FALSE;
	}
    }

  return TRUE;
}
#endif
#if 0
static bfd_boolean
elf32_i51_relocate_section (bfd *output_bfd ATTRIBUTE_UNUSED,
                            struct bfd_link_info *info,
                            bfd *input_bfd,
                            asection *input_section,
                            bfd_byte *contents,
                            Elf_Internal_Rela *relocs,
                            Elf_Internal_Sym *local_syms,
                            asection **local_sections)
{
  Elf_Internal_Shdr *           symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  Elf_Internal_Rela *           rel;
  Elf_Internal_Rela *           relend;

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend     = relocs + input_section->reloc_count;

  for (rel = relocs; rel < relend; rel ++)
    {
      reloc_howto_type *           howto;
      unsigned long                r_symndx;
      Elf_Internal_Sym *           sym;
      asection *                   sec;
      struct elf_link_hash_entry * h;
      bfd_vma                      relocation;
      bfd_reloc_status_type        r;
      const char *                 name;
      int                          r_type;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);
      howto  = elf_i51_howto_table + ELF32_R_TYPE (rel->r_info);
      h      = NULL;
      sym    = NULL;
      sec    = NULL;

      if (r_symndx < symtab_hdr->sh_info)
        {
          sym = local_syms + r_symndx;
          sec = local_sections [r_symndx];
          relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);

          name = bfd_elf_string_from_elf_section
            (input_bfd, symtab_hdr->sh_link, sym->st_name);
          name = (name == NULL) ? bfd_section_name (input_bfd, sec) : name;
        }
      else
        {
          bfd_boolean unresolved_reloc, warned;

          RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
                                   r_symndx, symtab_hdr, sym_hashes,
                                   h, sec, relocation,
                                   unresolved_reloc, warned);

          name = h->root.root.string;
        }

      if (sec != NULL && elf_discarded_section (sec))
        {
          /* For relocs against symbols from removed linkonce sections,
             or sections discarded by a linker script, we just want the
             section contents zeroed.  Avoid any special processing.  */
          _bfd_clear_contents (howto, input_bfd, contents + rel->r_offset);
          rel->r_info = 0;
          rel->r_addend = 0;
          continue;
        }

      if (info->relocatable)
        continue;

      r = i51_final_link_relocate (howto, input_bfd, input_section,
                                   contents, rel, relocation);

      if (r != bfd_reloc_ok)
        {
          const char * msg = (const char *) NULL;

          switch (r)
            {
            case bfd_reloc_overflow:
              r = info->callbacks->reloc_overflow
                (info, (h ? &h->root: NULL), name, howto->name, (bfd_vma) 0,
                 input_bfd, input_section, rel->r_offset);
              break;
            case bfd_reloc_undefined:
              r = info->callbacks->undefined_symbol
                (info, name, input_bfd, input_section, rel->r_offset, TRUE);
              break;

            case bfd_reloc_outofrange:
              msg = _("internal error: out of range error");
              break;

            case bfd_reloc_notsupported:
              msg = _("internal error: unsupported relocation error");
              break;

            case bfd_reloc_dangerous:
              msg = _("internal error: dangerous relocation");
              break;

            default:
              msg = _("internal error: unknown error");
              break;
            }

          if (msg)
            r = info->callbacks->warning
              (info, msg, name, input_bfd, input_section, rel->r_offset);

          if (! r)
            return FALSE;
        }
    }

  return TRUE;
}
#endif

/* The final processing done just before writing out a I51 ELF object
   file.  This gets the I51 architecture right based on the machine
   number.  */

static void
bfd_elf_i51_final_write_processing (abfd, linker)
     bfd *abfd;
     bfd_boolean linker ATTRIBUTE_UNUSED;
{
  unsigned long val;

  elf_elfheader (abfd)->e_machine = EM_I51;
  elf_elfheader (abfd)->e_flags |= bfd_mach_i51;
}

/* Set the right machine number.  */
#if 0
static bfd_boolean
elf32_i51_object_p (abfd)
     bfd *abfd;
{
  return TRUE;
}
#endif

static bfd_boolean
elf32_i51_object_p (abfd)
     bfd *abfd;
{
  unsigned int e_set = bfd_mach_i51;
  return bfd_default_set_arch_mach (abfd, bfd_arch_i51,
                                    e_set);
}

static bfd_boolean
i51_elf_section_from_shdr (abfd, hdr, name, shindex)
     bfd *abfd;
     Elf_Internal_Shdr *hdr;
     const char *name;
     int shindex;
{
  asection *newsect;
  flagword flags;

  if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name,shindex))
    return FALSE;

  newsect = hdr->bfd_section;
  flags = bfd_get_section_flags (abfd, newsect);

  if (hdr->sh_flags & SHF_CDATA) flags |= SEC_IS_COMMON;

  bfd_set_section_flags (abfd, newsect, flags);

  return TRUE;
}



static bfd_boolean
i51_elf_fake_sections (abfd, shdr, asect)
     bfd *abfd ATTRIBUTE_UNUSED;
     Elf_Internal_Shdr *shdr;
     asection *asect;
{
  register const char * name;

  name = bfd_get_section_name (abfd, asect);

  if (strcmp (name, ".rbbs") == 0)
    {
      shdr->sh_flags |= SHF_RDATA;
    }
  else if (strcmp (name, ".bbbs") == 0)
    {
      shdr->sh_flags |= SHF_BDATA;
    }
  else if (strcmp (name, ".ibbs") == 0)
    {
      shdr->sh_flags |= SHF_IDATA;
    }
  else if (strcmp (name, ".xbbs") == 0)
    {
      shdr->sh_flags |= SHF_XDATA;
    }
  else if (strcmp (name, ".ebbs") == 0)
    {
      shdr->sh_flags |= SHF_EDATA;
    }

  return TRUE;
}



bfd_boolean
elf32_i51_section_from_bfd_section (abfd, sec, retval)
     bfd *abfd ATTRIBUTE_UNUSED;
     asection *sec;
     int *retval;
{

  if (strcmp (bfd_get_section_name (abfd, sec), ".regbank") == 0)
    {
      *retval = SHN_I51_REGBANK;
      return TRUE;
    }
  else if (strcmp (bfd_get_section_name (abfd, sec), ".rbss") == 0)
    {
      *retval = SHN_I51_RDATA_C;
      return TRUE;
    }
  else if (strcmp (bfd_get_section_name (abfd, sec), ".bbss") == 0)
    {
      *retval = SHN_I51_BDATA_C;
      return TRUE;
    }
  else if (strcmp (bfd_get_section_name (abfd, sec), ".ibss") == 0)
    {
      *retval = SHN_I51_IDATA_C;
      return TRUE;
    }
  else if (strcmp (bfd_get_section_name (abfd, sec), ".xbss") == 0)
    {
      *retval = SHN_I51_XDATA_C;
      return TRUE;
    }
  else if (strcmp (bfd_get_section_name (abfd, sec), ".ebss") == 0)
    {
      *retval = SHN_I51_EDATA_C;
      return TRUE;
    }
  else if (strcmp (bfd_get_section_name (abfd, sec), ".bitbss") == 0)
    {
      *retval = SHN_I51_BITDATA_C;
      return TRUE;
    }
  return FALSE;
}


static asection i51_elf_scom_section;
static asymbol i51_elf_scom_symbol;
static asymbol *i51_elf_scom_symbol_ptr;

static asection i51_elf_rcom_section;
static asymbol i51_elf_rcom_symbol;
static asymbol *i51_elf_rcom_symbol_ptr;

static asection i51_elf_bcom_section;
static asymbol i51_elf_bcom_symbol;
static asymbol *i51_elf_bcom_symbol_ptr;

static asection i51_elf_icom_section;
static asymbol i51_elf_icom_symbol;
static asymbol *i51_elf_icom_symbol_ptr;

static asection i51_elf_xcom_section;
static asymbol i51_elf_xcom_symbol;
static asymbol *i51_elf_xcom_symbol_ptr;

static asection i51_elf_ecom_section;
static asymbol i51_elf_ecom_symbol;
static asymbol *i51_elf_ecom_symbol_ptr;

static asection i51_elf_bitcom_section;
static asymbol i51_elf_bitcom_symbol;
static asymbol *i51_elf_bitcom_symbol_ptr;

/* Handle the special I51 section numbers that a symbol may use.  */

void
elf32_i51_symbol_processing (abfd, asym)
     bfd *abfd ATTRIBUTE_UNUSED;
     asymbol *asym;
{
  elf_symbol_type *elfsym;

  elfsym = (elf_symbol_type *) asym;

  switch (elfsym->internal_elf_sym.st_shndx)
    {
    case SHN_I51_REGBANK:
      if (i51_elf_scom_section.name == NULL)
	{
	  /* Initialize the register common section.  */
          i51_elf_scom_section.name = ".regbank";
	  i51_elf_scom_section.flags = SEC_IS_COMMON;
	  i51_elf_scom_section.output_section = &i51_elf_scom_section;
	  i51_elf_scom_section.alignment_power = 0;
	  i51_elf_scom_section.symbol = &i51_elf_scom_symbol;
	  i51_elf_scom_section.symbol_ptr_ptr = &i51_elf_scom_symbol_ptr;
	  i51_elf_scom_symbol.name = ".regbank";
	  i51_elf_scom_symbol.flags = BSF_SECTION_SYM;
	  i51_elf_scom_symbol.section = &i51_elf_scom_section;
	  i51_elf_scom_symbol_ptr = &i51_elf_scom_symbol;
	}
      asym->section = &i51_elf_scom_section;
      asym->value = elfsym->internal_elf_sym.st_size;
      break;
    case SHN_I51_RDATA_C:
      if (i51_elf_rcom_section.name == NULL)
	{
	  /* Initialize the rdata common section.  */
	  i51_elf_rcom_section.name = ".rbss";
	  i51_elf_rcom_section.flags = SEC_IS_COMMON;
	  i51_elf_rcom_section.output_section = &i51_elf_rcom_section;
	  i51_elf_rcom_section.alignment_power = 0;
	  i51_elf_rcom_section.symbol = &i51_elf_rcom_symbol;
	  i51_elf_rcom_section.symbol_ptr_ptr = &i51_elf_rcom_symbol_ptr;
	  i51_elf_rcom_symbol.name = ".rbss";
	  i51_elf_rcom_symbol.flags = BSF_SECTION_SYM;
	  i51_elf_rcom_symbol.section = &i51_elf_rcom_section;
	  i51_elf_rcom_symbol_ptr = &i51_elf_rcom_symbol;
	}
      asym->section = &i51_elf_rcom_section;
      asym->value = elfsym->internal_elf_sym.st_size;
      break;
    case SHN_I51_BDATA_C:
      if (i51_elf_bcom_section.name == NULL)
	{
	  /* Initialize the bdata common section.  */
	  i51_elf_bcom_section.name = ".bbss";
	  i51_elf_bcom_section.flags = SEC_IS_COMMON;
	  i51_elf_bcom_section.output_section = &i51_elf_bcom_section;
	  i51_elf_bcom_section.alignment_power = 0;
	  i51_elf_bcom_section.symbol = &i51_elf_bcom_symbol;
	  i51_elf_bcom_section.symbol_ptr_ptr = &i51_elf_bcom_symbol_ptr;
	  i51_elf_bcom_symbol.name = ".bbss";
	  i51_elf_bcom_symbol.flags = BSF_SECTION_SYM;
	  i51_elf_bcom_symbol.section = &i51_elf_bcom_section;
	  i51_elf_bcom_symbol_ptr = &i51_elf_bcom_symbol;
	}
      asym->section = &i51_elf_bcom_section;
      asym->value = elfsym->internal_elf_sym.st_size;
      break;
    case SHN_I51_IDATA_C:
      if (i51_elf_icom_section.name == NULL)
	{
	  /* Initialize the idata common section.  */
	  i51_elf_icom_section.name = ".ibss";
	  i51_elf_icom_section.flags = SEC_IS_COMMON;
	  i51_elf_icom_section.output_section = &i51_elf_icom_section;
	  i51_elf_icom_section.alignment_power = 0;
	  i51_elf_icom_section.symbol = &i51_elf_icom_symbol;
	  i51_elf_icom_section.symbol_ptr_ptr = &i51_elf_icom_symbol_ptr;
	  i51_elf_icom_symbol.name = ".ibss";
	  i51_elf_icom_symbol.flags = BSF_SECTION_SYM;
	  i51_elf_icom_symbol.section = &i51_elf_icom_section;
	  i51_elf_icom_symbol_ptr = &i51_elf_icom_symbol;
	}
      asym->section = &i51_elf_icom_section;
      asym->value = elfsym->internal_elf_sym.st_size;
      break;
    case SHN_I51_XDATA_C:
      if (i51_elf_xcom_section.name == NULL)
	{
	  /* Initialize the xdata common section.  */
	  i51_elf_xcom_section.name = ".xbss";
	  i51_elf_xcom_section.flags = SEC_IS_COMMON;
	  i51_elf_xcom_section.output_section = &i51_elf_xcom_section;
	  i51_elf_xcom_section.alignment_power = 0;
	  i51_elf_xcom_section.symbol = &i51_elf_xcom_symbol;
	  i51_elf_xcom_section.symbol_ptr_ptr = &i51_elf_xcom_symbol_ptr;
	  i51_elf_xcom_symbol.name = ".xbss";
	  i51_elf_xcom_symbol.flags = BSF_SECTION_SYM;
	  i51_elf_xcom_symbol.section = &i51_elf_xcom_section;
	  i51_elf_xcom_symbol_ptr = &i51_elf_xcom_symbol;
	}
      asym->section = &i51_elf_xcom_section;
      asym->value = elfsym->internal_elf_sym.st_size;
      break;
    case SHN_I51_EDATA_C:
      if (i51_elf_ecom_section.name == NULL)
	{
	  /* Initialize the edata common section.  */
	  i51_elf_ecom_section.name = ".ebss";
	  i51_elf_ecom_section.flags = SEC_IS_COMMON;
	  i51_elf_ecom_section.output_section = &i51_elf_ecom_section;
	  i51_elf_ecom_section.alignment_power = 0;
	  i51_elf_ecom_section.symbol = &i51_elf_ecom_symbol;
	  i51_elf_ecom_section.symbol_ptr_ptr = &i51_elf_ecom_symbol_ptr;
	  i51_elf_ecom_symbol.name = ".ebss";
	  i51_elf_ecom_symbol.flags = BSF_SECTION_SYM;
	  i51_elf_ecom_symbol.section = &i51_elf_ecom_section;
	  i51_elf_ecom_symbol_ptr = &i51_elf_ecom_symbol;
	}
      asym->section = &i51_elf_ecom_section;
      asym->value = elfsym->internal_elf_sym.st_size;
      break;
    case SHN_I51_BITDATA_C:
      if (i51_elf_bitcom_section.name == NULL)
	{
	  /* Initialize the bdata common section.  */
	  i51_elf_bitcom_section.name = ".bitbss";
	  i51_elf_bitcom_section.flags = SEC_IS_COMMON;
	  i51_elf_bitcom_section.output_section = &i51_elf_bitcom_section;
	  i51_elf_bitcom_section.alignment_power = 0;
	  i51_elf_bitcom_section.symbol = &i51_elf_bitcom_symbol;
	  i51_elf_bitcom_section.symbol_ptr_ptr = &i51_elf_bitcom_symbol_ptr;
	  i51_elf_bitcom_symbol.name = ".bitbss";
	  i51_elf_bitcom_symbol.flags = BSF_SECTION_SYM;
	  i51_elf_bitcom_symbol.section = &i51_elf_bitcom_section;
	  i51_elf_bitcom_symbol_ptr = &i51_elf_bitcom_symbol;
	}
      asym->section = &i51_elf_bitcom_section;
      asym->value = elfsym->internal_elf_sym.st_size;
      break;
    }
}

static bfd_boolean
elf32_i51_add_symbol_hook (abfd, info, sym, namep, flagsp, secp, valp)
     bfd *abfd;
     struct bfd_link_info *info;
     Elf_Internal_Sym *sym;
     const char **namep;
     flagword *flagsp ATTRIBUTE_UNUSED;
     asection **secp;
     bfd_vma *valp;

{
  switch (sym->st_shndx)
    {
    case SHN_I51_REGBANK:
      *secp = bfd_make_section_old_way (abfd, ".regbank");
      (*secp)->flags |= SEC_IS_COMMON;
      *valp = sym->st_size;
      break;
    case SHN_I51_RDATA_C:
      *secp = bfd_make_section_old_way (abfd, ".rbss");
      (*secp)->flags |= SEC_IS_COMMON;
      *valp = sym->st_size;
      break;
    case SHN_I51_BDATA_C:
      *secp = bfd_make_section_old_way (abfd, ".bbss");
      (*secp)->flags |= (SEC_IS_COMMON);
      *valp = sym->st_size;
      break;
    case SHN_I51_IDATA_C:
      *secp = bfd_make_section_old_way (abfd, ".ibss");
      (*secp)->flags |= SEC_IS_COMMON;
      *valp = sym->st_size;
      break;
    case SHN_I51_XDATA_C:
      *secp = bfd_make_section_old_way (abfd, ".xbss");
      (*secp)->flags |= SEC_IS_COMMON;
      *valp = sym->st_size;
      break;
    case SHN_I51_EDATA_C:
      *secp = bfd_make_section_old_way (abfd, ".ebss");
      (*secp)->flags |= SEC_IS_COMMON;
      *valp = sym->st_size;
      break;
    case SHN_I51_BITDATA_C:
      *secp = bfd_make_section_old_way (abfd, ".bitbss");
      (*secp)->flags |= SEC_IS_COMMON;
      *valp = sym->st_size;
      break;
    }
  return TRUE;
}

#define ELF_ARCH		bfd_arch_i51
#define ELF_MACHINE_CODE	EM_I51
#define ELF_MAXPAGESIZE		1

#define TARGET_BIG_SYM       bfd_elf32_i51_vec
#define TARGET_BIG_NAME	     "elf32-i51"
#define bfd_elf32_bfd_reloc_type_lookup i51_elf32_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup i51_elf_reloc_name_lookup

#define elf_info_to_howto	             i51_info_to_howto_rela
#define elf_info_to_howto_rel	             0
#define elf_backend_relocate_section         elf32_i51_relocate_section
#define elf_backend_check_relocs             elf32_i51_check_relocs
#define elf_backend_can_gc_sections          1
#define elf_backend_can_refcount             1
//#define elf_backend_gc_mark_hook             elf32_i51_gc_mark_hook
//#define elf_backend_gc_sweep_hook            elf32_i51_gc_sweep_hook
#define elf_backend_rela                     1
#define elf_backend_final_write_processing   bfd_elf_i51_final_write_processing
#define elf_backend_object_p		     elf32_i51_object_p

//#define elf_backend_fake_sections		i51_elf_fake_sections
#define elf_backend_section_from_shdr		i51_elf_section_from_shdr
//#define elf_backend_symbol_processing		elf32_i51_symbol_processing
//#define elf_backend_section_from_bfd_section	elf32_i51_section_from_bfd_section
//#define elf_backend_add_symbol_hook		elf32_i51_add_symbol_hook
//#define elf_backend_match_sections_by_type NULL
#include "elf32-target.h"
