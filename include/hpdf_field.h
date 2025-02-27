/*
 * << Haru Free PDF Library >> -- hpdf_field.h
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

#ifndef _HPDF_FIELD_H
#define _HPDF_FIELD_H

#include "hpdf_objects.h"
#include "hpdf_doc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/*------ HPDF_RadioButtonField ----------------------------------------------*/

HPDF_RadioButtonField
HPDF_RadioButtonField_New  (HPDF_Doc    pdf,
                            const char *name,
                            HPDF_UINT   flag);

HPDF_STATUS
HPDF_RadioButtonField_AddChild  (HPDF_RadioButtonField fld,
                                 HPDF_Dict             child);

HPDF_INT
HPDF_RadioButtonField_AddOpt  (HPDF_RadioButtonField  fld,
                               const char            *value,
                               HPDF_Encoder           encoder);

HPDF_STATUS
HPDF_RadioButtonField_SetSelected  (HPDF_RadioButtonField  fld,
                                    const char            *value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_FIELD_H */
