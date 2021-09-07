/*  Copyright (c) 2021 David Anderson
    This test code is hereby placed in the public domain
    for anyone to use in any way.  */
#include "config.h"
#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for free(). */
#endif /* HAVE_STDLIB_H */
#include <stdio.h> /* For debugging. */
#ifdef HAVE_STDINT_H
#include <stdint.h> /* For uintptr_t */
#endif /* HAVE_STDINT_H */
#if defined(_WIN32) && defined(HAVE_STDAFX_H)
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */
#ifdef HAVE_STRING_H
#include <string.h>  /* strcpy() strlen() */
#endif
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif
#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "dwarf_base_types.h"
#include "dwarf_opaque.h"
#include "dwarf_error.h"

/*  This demonstates processing DWARF
    from in_memory data.  For simplicity
    we are using a static dataset.

    The motivation is from JIT compiling, where
    at runtime of some application, it generates
    code on the file and DWARF information for it too.

    This gives an example of enabling all of libdwarf's
    functions without actually having the DWARF information
    in a file.  (If you have a file in some odd format
    you can use this approach to have libdwarf access
    the format for DWARF data and work normally without
    ever exposing the format to libdwarf.)

    None of the structures defined here in this source
    (or any source using this feature)
    are ever known to libdwarf. they are totally
    private to your code.
*/

/* Some valid DWARF2 data */
Dwarf_Small abbrevbytes[] = {
0x01, 0x11, 0x01, 0x25, 0x0e, 0x13, 0x0b, 0x03, 0x08, 0x1b, 
0x0e, 0x11, 0x01, 0x12, 0x01, 0x10, 0x06, 0x00, 0x00, 0x02, 
0x2e, 0x01, 0x3f, 0x0c, 0x03, 0x08, 0x3a, 0x0b, 0x3b, 0x0b, 
0x39, 0x0b, 0x27, 0x0c, 0x11, 0x01, 0x12, 0x01, 0x40, 0x06, 
0x97, 0x42, 0x0c, 0x01, 0x13, 0x00, 0x00, 0x03, 0x34, 0x00, 
0x03, 0x08, 0x3a, 0x0b, 0x3b, 0x0b, 0x39, 0x0b, 0x49, 0x13, 
0x02, 0x0a, 0x00, 0x00, 0x04, 0x24, 0x00, 0x0b, 0x0b, 0x3e, 
0x0b, 0x03, 0x08, 0x00, 0x00, 0x00, };
int abbrevbyteslen = sizeof(abbrevbytes); 
Dwarf_Small infobytes[] = {
0x60, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x74, 0x2e, 0x63, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x66, 0x00, 0x01, 
0x02, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x01, 0x5c, 0x00, 0x00, 0x00, 0x03, 0x69, 
0x00, 0x01, 0x03, 0x08, 0x5c, 0x00, 0x00, 0x00, 0x02, 0x91, 
0x6c, 0x00, 0x04, 0x04, 0x05, 0x69, 0x6e, 0x74, 0x00, 0x00, };
int infobyteslen = sizeof(infobytes);
Dwarf_Small strbytes[] = {
0x47, 0x4e, 0x55, 0x20, 0x43, 0x31, 0x37, 0x20, 0x39, 0x2e, 
0x33, 0x2e, 0x30, 0x20, 0x2d, 0x6d, 0x74, 0x75, 0x6e, 0x65, 
0x3d, 0x67, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x20, 0x2d, 
0x6d, 0x61, 0x72, 0x63, 0x68, 0x3d, 0x78, 0x38, 0x36, 0x2d, 
0x36, 0x34, 0x20, 0x2d, 0x67, 0x64, 0x77, 0x61, 0x72, 0x66, 
0x2d, 0x32, 0x20, 0x2d, 0x4f, 0x30, 0x20, 0x2d, 0x66, 0x61, 
0x73, 0x79, 0x6e, 0x63, 0x68, 0x72, 0x6f, 0x6e, 0x6f, 0x75, 
0x73, 0x2d, 0x75, 0x6e, 0x77, 0x69, 0x6e, 0x64, 0x2d, 0x74, 
0x61, 0x62, 0x6c, 0x65, 0x73, 0x20, 0x2d, 0x66, 0x73, 0x74, 
0x61, 0x63, 0x6b, 0x2d, 0x70, 0x72, 0x6f, 0x74, 0x65, 0x63, 
0x74, 0x6f, 0x72, 0x2d, 0x73, 0x74, 0x72, 0x6f, 0x6e, 0x67, 
0x20, 0x2d, 0x66, 0x73, 0x74, 0x61, 0x63, 0x6b, 0x2d, 0x63, 
0x6c, 0x61, 0x73, 0x68, 0x2d, 0x70, 0x72, 0x6f, 0x74, 0x65, 
0x63, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x2d, 0x66, 0x63, 0x66, 
0x2d, 0x70, 0x72, 0x6f, 0x74, 0x65, 0x63, 0x74, 0x69, 0x6f, 
0x6e, 0x00, 0x2f, 0x76, 0x61, 0x72, 0x2f, 0x74, 0x6d, 0x70, 
0x2f, 0x74, 0x69, 0x6e, 0x79, 0x64, 0x77, 0x61, 0x72, 0x66, 
0x00, };
int strbyteslen = sizeof(strbytes);


/* An internals_t , data used elsewhere but
   not directly visible elsewhere. One needs to have one
   of these but  maybe the content here too little
   or useless, this is just an example of sorts.  */
#define SECCOUNT 4
struct sectiondata_s {
   unsigned int   sd_addr;
   unsigned int   sd_objoffsetlen;
   unsigned int   sd_objpointersize;
   Dwarf_Unsigned sd_sectionsize;
   const char   * sd_secname;
   Dwarf_Small  * sd_content;
};

/*  The secname must not be 0 , pass "" if
    there is no name. */
static struct sectiondata_s sectiondata[SECCOUNT] = {
{0,0,0,0,"",0},
{0,32,32,sizeof(abbrevbytes),".debug_abbrev",abbrevbytes},
{0,32,32,sizeof(infobytes),".debug_info",infobytes},
{0,32,32,sizeof(strbytes),".debug_str",strbytes}
};

typedef struct special_filedata_s {
    int            f_is_64bit;
    Dwarf_Endianness f_object_endian;
    unsigned       f_pointersize;
    unsigned       f_offsetsize;
    Dwarf_Unsigned f_filesize;
    Dwarf_Unsigned f_sectioncount;
    struct sectiondata_s * f_sectarray;
} special_filedata_internals_t;

static special_filedata_internals_t base_internals =
    { FALSE,DW_ENDIAN_LITTLE,32,32,0,SECCOUNT, sectiondata };




static
int gsinfo(void* obj,
    Dwarf_Half section_index,
    Dwarf_Obj_Access_Section_a* return_section,
    int* error UNUSEDARG)
{
    special_filedata_internals_t *internals =
        (special_filedata_internals_t *)(obj);
    struct sectiondata_s *finfo = 0;

    if (section_index >= SECCOUNT) {
        return DW_DLV_NO_ENTRY;
    }
    finfo = internals->f_sectarray + section_index;
    return_section->as_name   = finfo->sd_secname;
    return_section->as_type   = 0;
    return_section->as_flags  = 0;
    return_section->as_addr   = finfo->sd_addr;
    return_section->as_offset = 0;
    return_section->as_size   = finfo->sd_sectionsize;
    return_section->as_link   = 0;
    return_section->as_info   = 0;
    return_section->as_addralign = 0;
    return_section->as_entrysize = 1; 
    return DW_DLV_OK;
}
static
Dwarf_Endianness gborder(void * obj)
{
    special_filedata_internals_t *internals =
        (special_filedata_internals_t *)(obj);
    return internals->f_object_endian;
}
static
Dwarf_Small glensize(void * obj)
{
    /* offset size */
    special_filedata_internals_t *internals =
        (special_filedata_internals_t *)(obj);
    return internals->f_offsetsize/8;
}
static
Dwarf_Small gptrsize(void * obj)
{
    special_filedata_internals_t *internals =
        (special_filedata_internals_t *)(obj);
    return internals->f_pointersize/8;
}
static
Dwarf_Unsigned gfilesize(void * obj)
{
    special_filedata_internals_t *internals =
        (special_filedata_internals_t *)(obj);
    return internals->f_filesize;
}
static
Dwarf_Unsigned gseccount(void* obj)
{
    special_filedata_internals_t *internals =
        (special_filedata_internals_t *)(obj);
    return internals->f_sectioncount;
}
static
int gloadsec(void * obj,
    Dwarf_Half secindex,
    Dwarf_Small**rdata,
    int *error UNUSEDARG)
{
    special_filedata_internals_t *internals =
        (special_filedata_internals_t *)(obj);
    struct sectiondata_s *secp = 0;
    if (secindex >= internals->f_sectioncount) {
         return DW_DLV_NO_ENTRY;
    }
    secp = secindex +internals->f_sectarray; 
    *rdata = secp->sd_content;
    return DW_DLV_OK;
}

const Dwarf_Obj_Access_Methods_a methods = {
    gsinfo,
    gborder,
    glensize,
    gptrsize,
    gfilesize,
    gseccount,
    gloadsec,
    0 /* no relocating anything */
    };
struct Dwarf_Obj_Access_Interface_a_s interface = 
{ &base_internals,&methods };


static const Dwarf_Sig8 zerosignature;
static int
isformstring(Dwarf_Half form)
{
    /*  Not handling every form string, just the
        ones used in simple cases. */
    switch(form) {
    case DW_FORM_string:        return TRUE;
    case DW_FORM_GNU_strp_alt:  return TRUE;
    case DW_FORM_GNU_str_index: return TRUE;
    case DW_FORM_strx:          return TRUE;
    case DW_FORM_strx1:         return TRUE;
    case DW_FORM_strx2:         return TRUE;
    case DW_FORM_strx3:         return TRUE;
    case DW_FORM_strx4:         return TRUE;
    case DW_FORM_strp:          return TRUE;
    };
    return FALSE;
}

static int
print_attr(Dwarf_Attribute atr,
    Dwarf_Signed anumber, Dwarf_Error *error)
{
    int res = 0;
    char *str = 0;
    const char *attrname = 0;
    const char *formname = 0;
    Dwarf_Half form = 0;
    Dwarf_Half attrnum = 0;
    res = dwarf_whatform(atr,&form,error);
    if (res != DW_DLV_OK) {
        printf("dwarf_whatform failed! res %d\n",res);
        return res;
    }
    res = dwarf_whatattr(atr,&attrnum,error);
    if (res != DW_DLV_OK) {
        printf("dwarf_whatattr failed! res %d\n",res);
        return res;
    }
    res = dwarf_get_AT_name(attrnum,&attrname);
    if (res == DW_DLV_NO_ENTRY) {
       printf("Bogus attrnum 0x%x\n",attrnum);
       attrname = "<internal error ?>";
    }
    res = dwarf_get_FORM_name(form,&formname);
    if (res == DW_DLV_NO_ENTRY) {
       printf("Bogus form 0x%x\n",attrnum);
       attrname = "<internal error ?>";
    }
    if (!isformstring(form)) {
        
        printf("  [%2d] Attr: %-15s  Form: %-15s\n",
            (int)anumber,attrname,formname);
        return DW_DLV_OK;
    }
    res = dwarf_formstring(atr,&str,error);
    if (res != DW_DLV_OK) {
        printf("dwarf_formstring failed! res %d\n",res);
        return res;
    }
    printf("  [%2d] Attr: %-15s  Form: %-15s %s\n",
        (int)anumber,attrname,formname,str);
    return DW_DLV_OK;
}

static int
print_one_die(Dwarf_Die in_die,int level,
    Dwarf_Error *error)
{
    Dwarf_Attribute *attrbuf = 0;
    Dwarf_Signed  attrcount = 0;
    Dwarf_Half tag = 0;
    const char * tagname = 0;
    int res = 0;
    Dwarf_Signed i = 0;

    res = dwarf_tag(in_die,&tag,error);
    if (res != DW_DLV_OK) {
        printf("dwarf_tag failed! res %d\n",res);
        return res;
    }
    res = dwarf_get_TAG_name(tag,&tagname);
    if (res != DW_DLV_OK) {
         tagname = "<bogus tag>";
    }
    printf("%3d:  Die: %s\n",level,tagname);
    res = dwarf_attrlist(in_die,&attrbuf,&attrcount,error);
    if (res != DW_DLV_OK) {
        printf("dwarf_attrlist failed! res %d\n",res);
        return res;
    }
    for (i = 0; i <attrcount;++i) {
        res  =print_attr(attrbuf[i],i,error);
        if (res != DW_DLV_OK) {
            printf("dwarf_attr print failed! res %d\n",res);
            return res;
        }
    }
    return DW_DLV_OK;
}

static int
print_object_info(Dwarf_Debug dbg,Dwarf_Error *error)
{
    Dwarf_Bool is_info = TRUE; /* our data is not DWARF4
        .debug_types. */
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Half     version_stamp = 0;
    Dwarf_Off      abbrev_offset = 0;
    Dwarf_Half     address_size  = 0;
    Dwarf_Half     length_size   = 0;
    Dwarf_Half     extension_size = 0;
    Dwarf_Sig8     type_signature;
    Dwarf_Unsigned typeoffset     = 0;
    Dwarf_Unsigned next_cu_header_offset = 0;
    Dwarf_Half     header_cu_type = 0;
    int res = 0;
    Dwarf_Die cu_die = 0;
    int level = 0;

    type_signature = zerosignature;
    res = dwarf_next_cu_header_d(dbg,
        is_info,
        &cu_header_length,
        &version_stamp,
        &abbrev_offset,
        &address_size,
        &length_size,
        &extension_size,
        &type_signature,
        &typeoffset,
        &next_cu_header_offset,
        &header_cu_type,
        error);
    if (res != DW_DLV_OK) {
        printf("Next cu header  result %d. "
            "Something is wrong FAIL, line %d\n",res,__LINE__);
        exit(1);
    }
    printf("CU header length..........0x%" DW_PR_DUx "\n",
        cu_header_length);
    printf("Version stamp.............%d\n",version_stamp);
    printf("Address size .............%d\n",address_size);
    printf("Offset size...............%d\n",length_size);
    printf("Next cu header offset.....0x%" DW_PR_DUx "\n",
        next_cu_header_offset);

    res = dwarf_siblingof_b(dbg, NULL,is_info, &cu_die, error);
    if (res != DW_DLV_OK) {
        /* There is no CU die, which should be impossible. */
        if (res == DW_DLV_ERROR) {
            printf("ERROR: dwarf_siblingof_b failed, no CU die\n");
            return res;
        } else {
            printf("ERROR: dwarf_siblingof_b got NO_ENTRY, "
                "no CU die\n");
            return res;
        }
        exit(1);
    }
    res = print_one_die(cu_die,level,error);
    if (res != DW_DLV_OK) {
        printf("print_one_die failed! %d\n",res);
        exit(1);
    }


    return DW_DLV_OK;
}

/*  testing interfaces useful for embedding
    libdwarf inside another program or library. */
int main()
{
    int res = 0;
    Dwarf_Debug dbg = 0;
    Dwarf_Error error = 0;
    int fail = FALSE;

    /*  Fill in iface before this call. 
        We are using a static area, see above. */
    res = dwarf_object_init_b(&interface,
        0,0,DW_GROUPNUMBER_ANY,&dbg,
        &error);
    if (res != DW_DLV_OK) {
        if (res == DW_DLV_NO_ENTRY) {
            printf("FAIL Cannot dwarf_object_init_b() NO ENTRY. \n");
        } else {
            printf("FAIL Cannot dwarf_object_init_b(). \n");
            printf("msg: %s\n",dwarf_errmsg(error));
        }
        exit(1);
    }
    res = print_object_info(dbg,&error);
    if (res != DW_DLV_OK) {
        printf("FAIL printing, res %d line %d\n",res,__LINE__);
        exit(1);
    }
    
    dwarf_object_finish(dbg,&error);

    if (fail) {
        printf("FAIL objectaccess.c\n");
        exit(1);
    }
    return 0;
}
