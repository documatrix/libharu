/*
 * << Haru Free PDF Library >> -- hpdf_structure_element.h
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

#ifndef _HPDF_STRUCTURE_ELEMENT_H
#define _HPDF_STRUCTURE_ELEMENT_H

#include "hpdf_objects.h"
#include "hpdf_doc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/*------ HPDF_Structure_Element ----------------------------------------------*/

typedef struct _HPDF_StructureElementAttr_Rec  *HPDF_StructureElementAttr;

typedef struct _HPDF_StructureElementAttr_Rec {
    HPDF_StructTreeRoot struct_tree_root;
} HPDF_StructureElementAttr_Rec;

HPDF_StructureElement
HPDF_StructureElement_New  (HPDF_Doc              doc,
                            HPDF_StructureType    type,
                            HPDF_StructureElement parent);

HPDF_STATUS
HPDF_StructureElement_AddChild  (HPDF_StructureElement parent,
                                 HPDF_StructureElement child);

HPDF_STATUS
HPDF_StructureElement_AddMarkedContentSequence  (HPDF_StructureElement structure_element,
                                                 HPDF_UINT             mcid,
                                                 HPDF_Page             page);

HPDF_STATUS
HPDF_StructureElement_SetObjectReference (HPDF_StructureElement structure_element,
                                         HPDF_Page             page,
                                         HPDF_Annotation       annot);

HPDF_STATUS
HPDF_StructureElement_SetAlternateText  (HPDF_StructureElement  structure_element,
                                         const char            *alt,
                                         HPDF_UINT16            unicode_len);

HPDF_STATUS
HPDF_StructureElement_SetActualText  (HPDF_StructureElement  structure_element,
                                      const char            *actual_text,
                                      HPDF_UINT16            unicode_len);

HPDF_STATUS
HPDF_StructureElement_SetBBox (HPDF_StructureElement structure_element,
                               HPDF_Rect             rect);

HPDF_STATUS
HPDF_StructureElement_SetPlacement (HPDF_StructureElement structure_element,
                                    HPDF_Placement        placement);

HPDF_STATUS
HPDF_TableHeaderCell_SetScope (HPDF_StructureElement     structure_element,
                               HPDF_TableHeaderCellScope scope);

/*---------------------------------------------------------------------------*/
/*----- HPDF_StructTreeRoot -------------------------------------------------*/

HPDF_StructTreeRoot
HPDF_StructTreeRoot_New  (HPDF_MMgr mmgr,
                          HPDF_Xref xref);

HPDF_UINT32
HPDF_StructTreeRoot_AddParentTreeEntry  (HPDF_StructTreeRoot  struct_tree_root,
                                         void                *obj);

/*---------------------------------------------------------------------------*/
/*----- HPDF_Artifact -------------------------------------------------------*/

HPDF_Artifact
HPDF_Artifact_New  (HPDF_MMgr         mmgr,
                    HPDF_ArtifactType type);

HPDF_STATUS
HPDF_Artifact_SetSubtype  (HPDF_Artifact        artifact,
                           HPDF_ArtifactSubtype subtype);

/*---------------------------------------------------------------------------*/
/*----- HPDF_MarkedContent -------------------------------------------------------*/

typedef struct _HPDF_MarkedContentAttr_Rec  *HPDF_MarkedContentAttr;

typedef struct _HPDF_MarkedContentAttr_Rec {
    const char *tag;
} HPDF_MarkedContentAttr_Rec;

HPDF_MarkedContent
HPDF_MarkedContent_New  (HPDF_MMgr   mmgr,
                         const char *tag);

HPDF_STATUS
HPDF_MarkedContent_SetAlternateText  (HPDF_MarkedContent  marked_content,
                                      const char         *alt,
                                      HPDF_UINT16         unicode_len);

HPDF_STATUS
HPDF_MarkedContent_SetActualText  (HPDF_MarkedContent  marked_content,
                                   const char         *actual_text,
                                   HPDF_UINT16         unicode_len);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_STRUCTURE_ELEMENT_H */

