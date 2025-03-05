/*
 * << Haru Free PDF Library >> -- hpdf_field.c
 *
 * URL: http://libharu.org
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 * Copyright (c) 2007-2009 Antony Dovgal <tony@daylessday.org>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */

#include "hpdf_utils.h"
#include "hpdf_field.h"
#include "hpdf.h"

HPDF_RadioButtonField
HPDF_RadioButtonField_New  (HPDF_Doc    pdf,
                            const char *name,
                            HPDF_UINT   flag)
{
    HPDF_RadioButtonField fld;

    HPDF_PTRACE((" HPDF_RadioButtonField_New\n"));

    fld = HPDF_Dict_New (pdf->mmgr);
    if (!fld)
        return NULL;

    if (HPDF_Xref_Add (pdf->xref, fld) != HPDF_OK)
        return NULL;

    if (HPDF_Dict_AddName (fld, "FT", "Btn") != HPDF_OK)
        return NULL;

    HPDF_String fieldName = HPDF_String_New (pdf->mmgr, name, NULL);
    if (!fieldName)
        return NULL;
    if (HPDF_Dict_Add (fld, "T", fieldName) != HPDF_OK)
        return NULL;

    // ensure HPDF_FIELD_RADIO flag is set
    flag |= HPDF_FIELD_RADIO;
    // ensure that HPDF_PUSHBUTTON flag is not set
    flag &= ~HPDF_FIELD_PUSHBUTTON;
    if (HPDF_Dict_AddNumber (fld, "Ff", flag) != HPDF_OK)
        return NULL;

    if (HPDF_Catalog_AddInteractiveField (pdf->catalog, fld) != HPDF_OK)
        return NULL;

    return fld;
}

HPDF_STATUS
HPDF_RadioButtonField_AddChild  (HPDF_RadioButtonField fld,
                                 HPDF_Dict             child)
{
    HPDF_Array children;
    HPDF_STATUS ret;

    HPDF_PTRACE((" HPDF_RadioButtonField_AddChild\n"));

    if ((ret = HPDF_Dict_Add (child, "Parent", fld)) != HPDF_OK)
        return ret;

    children = (HPDF_Array)HPDF_Dict_GetItem (fld, "Kids", HPDF_OCLASS_ARRAY);
    if (!children) {
        children = HPDF_Array_New (fld->mmgr);
        if (!children)
            return HPDF_Error_GetCode (fld->error);

        if ((ret = HPDF_Dict_Add (fld, "Kids", children)) != HPDF_OK)
            return ret;
    }

    return HPDF_Array_Add (children, child);
}

HPDF_INT
HPDF_RadioButtonField_AddOpt  (HPDF_RadioButtonField  fld,
                               const char            *value,
                               HPDF_Encoder           encoder)
{
    HPDF_Array opt;

    HPDF_PTRACE((" HPDF_RadioButtonField_AddOpt\n"));

    opt = (HPDF_Array)HPDF_Dict_GetItem (fld, "Opt", HPDF_OCLASS_ARRAY);
    if (!opt) {
        opt = HPDF_Array_New (fld->mmgr);
        if (!opt)
            return -1;

        if (HPDF_Dict_Add (fld, "Opt", opt) != HPDF_OK)
            return -1;
    }

    HPDF_Number flag = (HPDF_Number)HPDF_Dict_GetItem (fld, "Ff", HPDF_OCLASS_NUMBER);
    if (!flag) {
        HPDF_SetError (fld->error, HPDF_DICT_ITEM_NOT_FOUND, 0);
        return -1;
    }

    HPDF_String s = HPDF_String_New (fld->mmgr, value, encoder);
    if (!s)
        return -1;

    if (HPDF_Array_Add (opt, s) != HPDF_OK)
        return -1;

    // if radios in unison is set, check if the value is already in the array
    // if it is, return the index of the value in the array
    if (flag->value & HPDF_FIELD_RADIOSINUNISON) {
        HPDF_UINT i;
        for (i = 0; i < opt->list->count; i++) {
            HPDF_String s2 = HPDF_Array_GetItem(opt, i, HPDF_OCLASS_STRING);
            if (!s2)
                return -1;

            if (HPDF_StrCmp(value, (const char *)s2->value) == 0)
                return i;
        }
    }

    return HPDF_Array_Items (opt) - 1;
}

HPDF_STATUS
HPDF_RadioButtonField_SetSelected  (HPDF_RadioButtonField  fld,
                                    const char            *value)
{
    HPDF_PTRACE((" HPDF_RadioButtonField_SetSelected\n"));

    return HPDF_Dict_AddName (fld, "V", value);
}
