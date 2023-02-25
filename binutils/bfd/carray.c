/* BFD back-end for C array objects.
   Copyright 2015 Free Software Foundation, Inc.
   Written by Robert Agoston <robert.agoston@hightec-rt.com>
   Infrastructure and other bits originally copied from binary.c.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef WITHOUT_FEATURE_HDP_934

#include "sysdep.h"
#include "bfd.h"
#include "safe-ctype.h"
#include "libbfd.h"
#include <stdint.h>

#define carray_close_and_cleanup     _bfd_generic_close_and_cleanup
#define carray_bfd_free_cached_info  _bfd_generic_bfd_free_cached_info
#define carray_new_section_hook      _bfd_generic_new_section_hook
#define carray_get_section_contents_in_window     _bfd_generic_get_section_contents_in_window
#define carray_set_arch_mach         _bfd_generic_set_arch_mach
#define carray_bfd_copy_private_bfd_data _bfd_generic_bfd_copy_private_bfd_data
#define carray_bfd_merge_private_bfd_data _bfd_generic_bfd_merge_private_bfd_data
#define carray_bfd_set_private_flags _bfd_generic_bfd_set_private_flags
#define carray_bfd_copy_private_symbol_data _bfd_generic_bfd_copy_private_symbol_data
#define carray_bfd_copy_private_header_data _bfd_generic_bfd_copy_private_header_data
#define carray_bfd_print_private_bfd_data _bfd_generic_bfd_print_private_bfd_data

extern bfd_boolean carray_set_odef (bfd *, bfd *, bfd_boolean, bfd_boolean);
extern bfd_boolean carray_set_variable (bfd *, const char *, const char *, const char *);
extern bfd_boolean carray_add_comments (bfd *, bfd_boolean, bfd_boolean, bfd_boolean);

const char *carray_default_output_label = "carray_mem";
const char *carray_default_output_type = "uint32_t";
const char *carray_default_output_size = "4";

typedef struct carray_label
  {
    struct carray_label *next;
    const char *name;
    symvalue value;
  }
carray_label_list;

typedef struct carray_section
  {
    struct carray_section *next;
    const char *name;
    char *array_data;
    bfd_size_type size;
    carray_label_list *labels;
    asection *sec;
  }
carray_section_list;

typedef struct carray_data_struct
  {
    bfd_size_type full_size;
    carray_section_list *sections;
    asymbol **symbol_table;
    long number_of_symbols;
    bfd *odef_bfd;
    const char *output_label;
    const char *output_type;
    const char *output_size;
    int ioutput_size;
    bfd_boolean odef_add_include;
    bfd_boolean odef_add_extern;
    bfd_boolean add_comments;
    bfd_boolean odef_add_stdint;
    bfd_boolean odef_add_closegaps;
  }
tdata_type;


bfd_boolean
carray_set_variable (bfd *abfd,
		     const char *type,
		     const char *size,
		     const char *label)
{
  tdata_type *tdata;

  if (abfd == NULL)
    return FALSE;

  tdata = (tdata_type *)abfd->tdata.any;

  if (type != NULL)
    tdata->output_type = type;

  if (size != NULL)
    {
    tdata->output_size = size;
    int res;
    res=sscanf(tdata->output_size,"%d",&tdata->ioutput_size);
    if (res!=1) { (*_bfd_error_handler) (_("invalid argument osize 1,2 or 4")); return FALSE; }
    if ((tdata->ioutput_size!=1) && (tdata->ioutput_size!=2) && (tdata->ioutput_size!=4)) { (*_bfd_error_handler) (_("invalid argument osize 1,2 or 4")); return FALSE; }
    }
  if (label != NULL)
    tdata->output_label = label;

  return TRUE;
}


bfd_boolean
carray_set_odef (bfd *abfd,
		 bfd *odef_bfd,
		 bfd_boolean is_inc,
		 bfd_boolean is_ext)
{
  tdata_type *tdata;

  if (abfd == NULL || odef_bfd == NULL)
    return FALSE;

  if (abfd->tdata.any == NULL)
    return FALSE;

  tdata = (tdata_type *)abfd->tdata.any;
  tdata->odef_bfd = odef_bfd;
  tdata->odef_add_include = is_inc;
  tdata->odef_add_extern = is_ext;
  return TRUE;
}


bfd_boolean
carray_add_comments (bfd *abfd,
		     bfd_boolean add_comments,
	             bfd_boolean is_stdint,
	             bfd_boolean is_closegaps)
{
  tdata_type *tdata;

  if (abfd == NULL)
    return FALSE;

  if (abfd->tdata.any == NULL)
    return FALSE;

  tdata = (tdata_type *)abfd->tdata.any;
  tdata->add_comments = add_comments;
  tdata->odef_add_stdint = is_stdint;
  tdata->odef_add_closegaps = is_closegaps;
  return TRUE;
}


/* Create a C array object.  Invoked via bfd_set_format.  */

static bfd_boolean
carray_mkobject (bfd *abfd)
{
  tdata_type *tdata;
  int res;
  tdata = (tdata_type*) bfd_alloc (abfd, sizeof (tdata_type));
  if (tdata == NULL)
    return FALSE;

  abfd->tdata.any = tdata;
  tdata->full_size = 0;
  tdata->sections = NULL;
  tdata->odef_bfd = NULL;
  tdata->output_label = carray_default_output_label;
  tdata->output_type = carray_default_output_type;
  tdata->output_size = carray_default_output_size;
  res=sscanf(tdata->output_size,"%d",&tdata->ioutput_size);
  if (res!=1) { (*_bfd_error_handler) (_("invalid argument osize 1,2 or 4")); return FALSE; }
  if ((tdata->ioutput_size!=1) && (tdata->ioutput_size!=2) && (tdata->ioutput_size!=4)) { (*_bfd_error_handler) (_("invalid argument osize 1,2 or 4")); return FALSE; }
  tdata->odef_add_include = FALSE;
  tdata->odef_add_extern = FALSE;
  tdata->odef_add_stdint = FALSE;
  tdata->odef_add_closegaps = FALSE;
  tdata->symbol_table = NULL;
  tdata->number_of_symbols = 0;
  tdata->add_comments = FALSE;
  return TRUE;
}


/* Get contents of the only section.  */

static bfd_boolean
carray_get_section_contents (bfd *abfd,
			     asection *section ATTRIBUTE_UNUSED,
			     void * location,
			     file_ptr offset,
			     bfd_size_type count)
{
  if (bfd_seek (abfd, offset, SEEK_SET) != 0
      || bfd_bread (location, count, abfd) != count)
    return FALSE;
  return TRUE;
}


static void
add_label (bfd *abfd, carray_section_list *csection, const char *name, symvalue value)
{
  carray_label_list *label = (carray_label_list *) bfd_alloc (abfd, sizeof (carray_label_list));
  label->next = NULL;
  label->name = name;
  label->value = value;

  if (csection->labels == NULL)
    {
      csection->labels = label;
    }
  else
    {
      if (value <= csection->labels->value)
	{
	  /* We have a new head. */
	  label->next = csection->labels;
	  csection->labels = label;
	}
      else
	{
	  carray_label_list *list = csection->labels;

	  while ((list->value < value) && (list->next != NULL) && (list->next->value < value))
	    {
	      list = list->next;
	    }

	  carray_label_list *next = list->next;
	  list->next = label;
	  label->next = next;
	}
    }
}


static carray_section_list *
carray_find_csection_by_index (int index,
			       tdata_type *tdata)
{
  carray_section_list *csection;

  if (tdata == NULL || tdata->sections == NULL)
    return NULL;

  csection = tdata->sections;

  while (csection != NULL)
    {
      if (csection->sec != NULL &&
	  csection->sec->index == index)
	{
	  return csection;
	}

      csection = csection->next;
    }

  return NULL;
}

/* check if size of output element and size + start is fitting together) */
static bfd_boolean
carray_verify_requirements (carray_section_list *csection, int size)
{
int fail;
  fail=0;
  if ((csection->size % size)!=0) fail+=1;
  if ((csection->sec->vma % size)!=0) fail+=1;
  if ((csection->sec->lma % size)!=0) fail+=1;
  if (fail!=0)
    {
    (*_bfd_error_handler) (_("section does not meet the requirements (size,len,alignment) sec_name=%s sec_size=0x%x sec_vma=0x%lx sec_lma=0x%lx vs. --osize %d"),
                             csection->name,csection->size,csection->sec->vma,csection->sec->lma,size);
    return FALSE;
    }
  return TRUE;
}

/* Write section contents of a carray file.  */

static bfd_boolean
carray_set_section_contents (bfd *abfd,
			     asection *sec,
			     const void * data,
			     file_ptr offset ATTRIBUTE_UNUSED,
			     bfd_size_type size)
{
  tdata_type *tdata = (tdata_type*) abfd->tdata.any;
  carray_section_list *csection;

  if ((sec->size <= 0) || (size <= 0))
    return TRUE;
  if ((sec->flags & (SEC_LOAD | SEC_ALLOC)) == 0)
    return TRUE;
  if ((sec->flags & SEC_NEVER_LOAD) != 0)
    return TRUE;
  csection = carray_find_csection_by_index (sec->index, tdata);
  if (csection != NULL)
    {
      bfd_size_type old_size = csection->size;
      bfd_size_type new_size = csection->size + size;
      char *old_array_data = (char *) csection->array_data;
      char *new_array_data = bfd_malloc (new_size);

      if (new_array_data == NULL)
	{
	  return FALSE;
	}
      else
	{
	  memcpy (new_array_data, old_array_data, old_size);
	  memcpy ((new_array_data + old_size), data, size);
	  free (old_array_data);

	  csection->array_data = (char *)new_array_data;
	  csection->size = new_size;
	  tdata->full_size += size;
          if (carray_verify_requirements(csection,tdata->ioutput_size)==FALSE) return FALSE;
	  return TRUE;
	}
    }
  else
    {
      csection = (carray_section_list*) bfd_alloc (abfd, sizeof (carray_section_list));
      if (csection == NULL)
	return FALSE;

      csection->next = NULL;
      csection->name = bfd_get_section_name (abfd, sec);
      csection->size = size;
      csection->sec = sec;
      csection->array_data = (char *) bfd_malloc (size);
      csection->labels = NULL;

      if (csection->array_data == NULL)
	return FALSE;

      memcpy (csection->array_data, data, size);
      tdata->full_size += size;

      if (tdata->sections == NULL)
	tdata->sections = csection;
      else
	{
	  carray_section_list *list = tdata->sections;
	  while (list->next != NULL)
	    list = list->next;
	  list->next = csection;
	}
      if (carray_verify_requirements(csection,tdata->ioutput_size)==FALSE) return FALSE;
    }

  return TRUE;
}


static bfd_boolean
slurp_symtab (bfd* ibfd, bfd* obfd)
{
  long storage_needed;
  tdata_type *tdata;

  if (ibfd == NULL || obfd == NULL)
    return FALSE;

  tdata = (tdata_type *)obfd->tdata.any;

  if (tdata->symbol_table != NULL)
    return TRUE;

  storage_needed = bfd_get_symtab_upper_bound (ibfd);
  if (storage_needed < 0)
    return FALSE;

  if (storage_needed == 0)
    return TRUE;

  tdata->symbol_table = (asymbol **) bfd_alloc (ibfd, storage_needed);

  tdata->number_of_symbols = bfd_canonicalize_symtab (ibfd, tdata->symbol_table);

  if (tdata->number_of_symbols < 0)
    return FALSE;

  return TRUE;
}


static bfd_boolean
carray_bfd_copy_private_section_data (bfd *ibfd,
				      asection *isec,
				      bfd *obfd,
				      asection *osec)
{
  if (isec->size <= 0)
    return TRUE;
  if ((isec->flags & (SEC_HAS_CONTENTS)) == 0)
    return TRUE;
  if ((isec->flags & (SEC_LOAD | SEC_ALLOC)) == 0)
    return TRUE;
  if ((isec->flags & SEC_NEVER_LOAD) != 0)
    return TRUE;

  osec->output_section = isec;

  if (! slurp_symtab (ibfd, obfd))
    return FALSE;

  return TRUE;
}


static char*
macroname (const char* str)
{
  int len = strlen (str);
  char* buf = bfd_malloc (len + 1);
  int i = 0;

  while (*str)
    {
      buf[i] = (ISALPHA(*str) || ISDIGIT(*str)) ? *str : '_';
      str++;
      i++;
    }
  buf[i] = '\0';

  return buf;
}


static bfd_boolean
carray_write_object_contents (bfd *abfd)
{
  tdata_type *tdata = (tdata_type*) abfd->tdata.any;
  static char buf[1024];
  bfd_size_type len;
  bfd_size_type label_index = 0;
  bfd_size_type written_octets = 0;
  bfd_size_type full_size;
  bfd_size_type array_size;
  carray_section_list *csection;
  carray_label_list *label;
  char* macro_label;
  int i;

#define CARRAY_WRITE_BUF(abfd) {\
  len = (bfd_size_type)strlen (buf);\
  if (bfd_bwrite (buf, len, abfd) != len)\
    {\
      return FALSE;\
    }\
}

  if (tdata == NULL)
    {
      return FALSE;
    }

  csection = tdata->sections;

  if (csection == NULL)
    {
      return FALSE;
    }

  macro_label = macroname (tdata->output_label);

  if (macro_label == NULL)
    {
      return FALSE;
    }

  /* check if the collected sections are containing gaps */
  csection = tdata->sections;
  while (csection != NULL)
    {
      carray_section_list *csection_next;
      csection_next=csection->next;
      if (csection_next != NULL)
        {
          if ((csection->sec->vma + csection->size) != csection_next->sec->vma)
            {
              if (!tdata->odef_add_closegaps)
                {
                (*_bfd_error_handler) (_("sections containing gaps between sec_name=%s sec_size=0x%x sec_vma=0x%lx sec_lma=0x%lx and sec_name=%s sec_size=0x%x sec_vma=0x%lx sec_lma=0x%lx" ),
                                       csection->name,csection->size,csection->sec->vma,csection->sec->lma,csection_next->name,csection_next->size,csection_next->sec->vma,csection_next->sec->lma);
                return FALSE;
                }
              //fill the gap
              int new_size=csection_next->sec->vma-csection->sec->vma;
              char *new_array_data = bfd_malloc (new_size);
              memset (new_array_data,0,new_size);
              memcpy (new_array_data,csection->array_data,csection->size);
              tdata->full_size += new_size - csection->size;
              csection->size = new_size;
              free (csection->array_data);
              csection->array_data=new_array_data;
            }
        }
        csection = csection->next;
    }
  csection = tdata->sections;


  full_size = tdata->full_size;
  array_size = (bfd_size_type)(full_size);

  if (! abfd->output_has_begun)
    abfd->output_has_begun = TRUE;

  if (tdata->odef_bfd != NULL)
    {
      char* macro_filename = macroname (tdata->odef_bfd->filename);

      sprintf (buf, "#ifndef %s_\n#define %s_\n\n",
	       macro_filename,  macro_filename);
      CARRAY_WRITE_BUF(tdata->odef_bfd);
      if (tdata->odef_add_stdint)
        {
          sprintf (buf, "#include <stdint.h>\n\n");
          CARRAY_WRITE_BUF(tdata->odef_bfd);
        }

      if (tdata->odef_add_extern)
	{
	  sprintf (buf, "extern %s %s[%lu];\n\n",
		   tdata->output_type, tdata->output_label,
		   array_size);
	  CARRAY_WRITE_BUF(tdata->odef_bfd);
	}

      sprintf (buf, "#define OFFSET_%s    (%ld) /*(0x%lx) */\n", macro_label, 0UL, 0UL);
      CARRAY_WRITE_BUF(tdata->odef_bfd);

      sprintf (buf, "#define SIZE_%s      (%ld) /*(0x%lx) */\n\n",
	       macro_label, full_size, full_size);
      CARRAY_WRITE_BUF(tdata->odef_bfd);

      free (macro_filename);
    }

  if (tdata->odef_add_stdint && (tdata->odef_bfd != NULL))
    {
      sprintf (buf, "#include <stdint.h>\n");
      CARRAY_WRITE_BUF(abfd);
    }

  if (tdata->odef_add_include && (tdata->odef_bfd != NULL))
    {
      sprintf (buf, "#include \"%s\"\n", tdata->odef_bfd->filename);
      CARRAY_WRITE_BUF(abfd);
    }

  sprintf (buf,"\n/* The IHX representation */\n");
  CARRAY_WRITE_BUF(abfd);
  sprintf (buf,
	  "\n%s %s[%lu] = { \n",
	  tdata->output_type,
	  tdata->output_label,
	  array_size);
  CARRAY_WRITE_BUF(abfd);

  if (tdata->odef_bfd != NULL)
    {
      /* write all symbols with there memory index to odef file */
      for (i = 0; i < tdata->number_of_symbols; i++)
        {
          asymbol* sym = tdata->symbol_table[i];
          if ((sym->section->flags & (SEC_LOAD | SEC_ALLOC))
              && ((sym->flags & BSF_SECTION_SYM) == 0))
            {
              char* label_name = macroname (sym->name);
              add_label (abfd, csection, sym->name, sym->value + sym->section->vma);
              sprintf (buf, "#define LABEL_%s_%s      (%ld) /*(0x%lx) */\n",
                       macro_label, label_name, (sym->value + sym->section->vma),(sym->value + sym->section->vma));
              free (label_name);
             CARRAY_WRITE_BUF(tdata->odef_bfd);
            }
        }
    }
  label = csection->labels;
  while (csection != NULL)
    {
      char *array_data = csection->array_data;
      bfd_size_type section_size = csection->size;
      bfd_size_type index;
      if (tdata->add_comments && section_size > 0)
	{
          sprintf (buf, "/* csection: %s output_size=%d section_size=%ld full_size=%ld array_size=%ld vma=%lx lma=%lx*/\n",csection->name,tdata->ioutput_size,csection->size,full_size,array_size,csection->sec->vma,csection->sec->lma);
	  CARRAY_WRITE_BUF(abfd);
	}

      for (index = 0; (index * tdata->ioutput_size) < section_size; index++)
	{
	  while ((label != NULL) && ((label->value / tdata->ioutput_size) == label_index))
	    {
	      if (tdata->add_comments)
		{
		  sprintf (buf, "/* %s [%lu]: */\n", label->name, label_index);
		  CARRAY_WRITE_BUF(abfd);
		}

	      label = label->next;
	    }

	  unsigned int element;
	  if (tdata->ioutput_size==4)
	    {
	      uint32_t *p32;
	      p32=(uint32_t *) &array_data[index];
	      element=*p32;
	    }
          if (tdata->ioutput_size==2)
            {
              uint16_t *p16;
              p16=(uint16_t *) &array_data[index];
              element=*p16;
            }
          if (tdata->ioutput_size==1)
            {
              uint8_t *p8;
              p8=(uint8_t *) &array_data[index];
              element=*p8;
            }

	  written_octets += tdata->ioutput_size;
	  const char *comma = (written_octets < full_size) ? "," : "";

	  if (tdata->ioutput_size==4) sprintf (buf, "    0x%08X%s\n", element, comma);
          if (tdata->ioutput_size==2) sprintf (buf, "    0x%04X%s\n", element, comma);
          if (tdata->ioutput_size==1) sprintf (buf, "    0x%02X%s\n", element, comma);
	  CARRAY_WRITE_BUF(abfd);
	  label_index++;
	}
      /* gap filler */
      csection = csection->next;
    }

  sprintf (buf, "};\n");
  CARRAY_WRITE_BUF(abfd);

  if (tdata->odef_bfd != NULL)
    {
      sprintf (buf, "\n#endif\n");
      CARRAY_WRITE_BUF(tdata->odef_bfd);
      bfd_close (tdata->odef_bfd);
    }

  free (macro_label);

  return TRUE;
}


const bfd_target carray_vec =
{
  "carray",			/* name */
  bfd_target_unknown_flavour,	/* flavour */
  BFD_ENDIAN_UNKNOWN,		/* byteorder */
  BFD_ENDIAN_UNKNOWN,		/* header_byteorder */
  BFD_NO_FLAGS,		/* object_flags */
  (SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE | SEC_DATA
   | SEC_ROM | SEC_HAS_CONTENTS), /* section_flags */
  0,				/* symbol_leading_char */
  ' ',				/* ar_pad_char */
  16,				/* ar_max_namelen */
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,	/* data */
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,	/* hdrs */
  {				/* bfd_check_format */
    _bfd_dummy_target,
    _bfd_dummy_target,
    _bfd_dummy_target,
    _bfd_dummy_target,
  },
  {				/* bfd_set_format */
    bfd_false,
    carray_mkobject,
    bfd_false,
    bfd_false,
  },
  {				/* bfd_write_contents */
    bfd_false,
    carray_write_object_contents,
    bfd_false,
    bfd_false,
  },

  BFD_JUMP_TABLE_GENERIC (carray),
  BFD_JUMP_TABLE_COPY (carray),
  BFD_JUMP_TABLE_CORE (_bfd_nocore),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_noarchive),
  BFD_JUMP_TABLE_SYMBOLS (_bfd_nosymbols),
  BFD_JUMP_TABLE_RELOCS (_bfd_norelocs),
  BFD_JUMP_TABLE_WRITE (carray),
  BFD_JUMP_TABLE_LINK (_bfd_nolink),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  NULL,

  NULL
};

#endif /* without HDP-934 */
