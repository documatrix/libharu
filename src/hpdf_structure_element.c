/*
 * << Haru Free PDF Library >> -- hpdf_annotation.c
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

#include "hpdf_conf.h"
#include "hpdf_utils.h"
#include "hpdf_info.h"
#include "hpdf_structure_element.h"
#include "hpdf.h"

static const char * const HPDF_STRUCTURE_TYPE_NAMES[] = {
    "Document",
    "Part",
    "Art",
    "Sect",
    "Div",
    "BlockQuote",
    "Caption",
    "TOC ",
    "TOCI",
    "Index",
    "NonStruct",
    "Private",
    "H",
    "H1",
    "H2",
    "H3",
    "H4",
    "H5",
    "H6",
    "P",
    "L",
    "LI",
    "Lbl",
    "LBody",
    "Table",
    "TR",
    "TH",
    "TD",
    "THead",
    "TBody",
    "TFoot",
    "Span",
    "Quote",
    "Note",
    "Reference",
    "BibEntry",
    "Code",
    "Link",
    "Annot",
    "Ruby",
    "Warichu",
    "Figure",
    "Formula",
    "Form"
};

static const char * const HPDF_ARTIFACT_TYPE_NAMES[4] = {
    "Pagination",
    "Layout",
    "Page",
    "Background"
};

static const char * const HPDF_ARTIFACT_SUBTYPE_NAMES[3] = {
    "Header",
    "Footer",
    "Watermark"
};

static const char * const HPDF_TABLE_HEADER_CELL_SCOPE_NAMES[3] = {
    "Row",
    "Column",
    "Both"
};

static const char * const HPDF_PLACEMENT_NAMES[5] = {
    "Block",
    "Inline",
    "Before",
    "Start",
    "End"
};

static void
StructureElement_OnFree  (HPDF_Dict obj);

static void
MarkedContent_OnFree  (HPDF_Dict obj);

static HPDF_STATUS
HPDF_Dict_AddUnicodeEntry  (HPDF_Dict    dict,
                            const char  *entry_name,
                            const char  *text,
                            HPDF_UINT16  unicode_len);

static HPDF_Dict
HPDF_StructureElement_GetAttributes  (HPDF_StructureElement  structure_element,
                                      const char            *owner);

HPDF_StructTreeRoot
HPDF_StructTreeRoot_New  (HPDF_MMgr mmgr,
                          HPDF_Xref xref)
{
    HPDF_StructTreeRoot struct_tree_root;

    HPDF_PTRACE((" HPDF_StructTreeRoot_New\n"));

    struct_tree_root = HPDF_Dict_New (mmgr);
    if (!struct_tree_root)
        return NULL;

    if (HPDF_Xref_Add (xref, struct_tree_root) != HPDF_OK)
        return NULL;

    if (HPDF_Dict_AddName (struct_tree_root, "Type", "StructTreeRoot") != HPDF_OK)
        return NULL;

    return struct_tree_root;
}

HPDF_UINT32
HPDF_StructTreeRoot_AddParentTreeEntry  (HPDF_StructTreeRoot  struct_tree_root,
                                         void                *obj)
{
    HPDF_STATUS ret = HPDF_OK;

    HPDF_PTRACE((" HPDF_StructTreeRoot_AddParentTreeArray\n"));

    HPDF_Array nums; // TODO max amount of children 8192 in pdf/a-1, could fix with kids + nums + limits
    HPDF_Dict parent_tree = (HPDF_Dict)HPDF_Dict_GetItem (struct_tree_root, "ParentTree", HPDF_OCLASS_DICT);
    if (!parent_tree) {
        // create ParentTree
        parent_tree = HPDF_Dict_New (struct_tree_root->mmgr);
        if (!parent_tree)
            return 0;

        ret = HPDF_Dict_Add (struct_tree_root, "ParentTree", parent_tree);
        if (ret != HPDF_OK)
            return 0;

        nums = HPDF_Array_New (parent_tree->mmgr);
        if (!nums)
            return 0;

        ret = HPDF_Dict_Add (parent_tree, "Nums", nums);
        if (ret != HPDF_OK)
            return 0;

        ret = HPDF_Dict_AddNumber (struct_tree_root, "ParentTreeNextKey", 1);
        if (ret != HPDF_OK)
            return 0;
    } else {
        nums = (HPDF_Array)HPDF_Dict_GetItem (parent_tree, "Nums", HPDF_OCLASS_ARRAY);
        if (!nums)
            return 0;
    }

    HPDF_Number next_key = (HPDF_Number)HPDF_Dict_GetItem (struct_tree_root, "ParentTreeNextKey", HPDF_OCLASS_NUMBER);
    if (!next_key)
        return 0;

    ret = HPDF_Array_AddNumber (nums, next_key->value);
    if (ret != HPDF_OK)
        return 0;

    ret = HPDF_Array_Add (nums, obj);
    if (ret != HPDF_OK)
        return 0;

    return next_key->value++;
}

HPDF_StructureElement
HPDF_StructureElement_New  (HPDF_Doc              pdf,
                            HPDF_StructureType    type,
                            HPDF_StructureElement parent)
{
    HPDF_StructureElement se;
    HPDF_StructureElementAttr attr;
    HPDF_STATUS ret = HPDF_OK;

    HPDF_PTRACE((" HPDF_StructureElement_New\n"));

    se = HPDF_Dict_New (pdf->mmgr);
    if (!se)
        return NULL;

    se->free_fn = StructureElement_OnFree;

    attr = HPDF_GetMem (pdf->mmgr, sizeof(HPDF_StructureElementAttr_Rec));
    if (!attr) {
        HPDF_Dict_Free (se);
        return NULL;
    }

    se->attr = attr;
    HPDF_MemSet (attr, 0, sizeof(HPDF_StructureElementAttr_Rec));

    if (HPDF_Xref_Add (pdf->xref, se) != HPDF_OK)
        return NULL;

    ret = HPDF_Dict_AddName (se, "S", HPDF_STRUCTURE_TYPE_NAMES[(HPDF_INT)type]);
    if (ret != HPDF_OK)
        return NULL;

    if (!parent) {
        // parent is structure tree root
        if (pdf->struct_tree_root) {
            parent = pdf->struct_tree_root;
        } else {
            pdf->struct_tree_root = HPDF_StructTreeRoot_New (pdf->mmgr, pdf->xref);
            if (pdf->struct_tree_root) {
                 HPDF_STATUS ret = HPDF_Dict_Add (pdf->catalog, "StructTreeRoot", pdf->struct_tree_root);
                if (ret != HPDF_OK) {
                    HPDF_CheckError (&pdf->error);
                    pdf->struct_tree_root = NULL;
                    return NULL;
                }

                parent = pdf->struct_tree_root;
            }
        }
    }
    attr->struct_tree_root = pdf->struct_tree_root;

    ret = HPDF_StructureElement_AddChild(parent, se);
    if (ret != HPDF_OK)
        return NULL;

    return se;
}

static void
StructureElement_OnFree  (HPDF_Dict obj)
{
    HPDF_StructureElementAttr attr = (HPDF_StructureElementAttr)obj->attr;

    HPDF_PTRACE((" HPDF_StructureElement_OnFree\n"));

    if (attr)
        HPDF_FreeMem (obj->mmgr, attr);
}

HPDF_STATUS
HPDF_StructureElement_AddChild  (HPDF_StructureElement parent,
                                 HPDF_StructureElement child)
{
    HPDF_Array children;
    HPDF_STATUS ret;

    HPDF_PTRACE((" HPDF_StructureElement_AddChild\n"));

    if (HPDF_Dict_GetItem (child, "P", HPDF_OCLASS_DICT))
        return HPDF_SetError (parent->error, HPDF_STRUCTURE_ELEMENT_CANNOT_SET_PARENT, 0);

    if ((ret = HPDF_Dict_Add (child, "P", parent)) != HPDF_OK)
        return ret;

    children = (HPDF_Array)HPDF_Dict_GetItem (parent, "K", HPDF_OCLASS_ARRAY);
    if (!children) {
        children = HPDF_Array_New (parent->mmgr);
        if (!children)
            return HPDF_Error_GetCode (parent->error);

        if ((ret = HPDF_Dict_Add (parent, "K", children)) != HPDF_OK)
            return ret;
    }

    return HPDF_Array_Add (children, child);
}

HPDF_STATUS
HPDF_StructureElement_AddMarkedContentSequence  (HPDF_StructureElement structure_element,
                                                 HPDF_UINT             mcid,
                                                 HPDF_Page             page)
{
    HPDF_Array children;
    HPDF_Dict pg;
    HPDF_STATUS ret;

    HPDF_PTRACE((" HPDF_StructureElement_AddChild\n"));

    pg = (HPDF_Dict)HPDF_Dict_GetItem (structure_element, "Pg", HPDF_OCLASS_DICT);
    if (!pg) {
        pg = page;
        if ((ret = HPDF_Dict_Add (structure_element, "Pg", page)) != HPDF_OK)
            return ret;
    }

    children = (HPDF_Array)HPDF_Dict_GetItem (structure_element, "K", HPDF_OCLASS_ARRAY);
    if (!children) {
        children = HPDF_Array_New (structure_element->mmgr);
        if (!children)
            return HPDF_Error_GetCode (structure_element->error);

        if ((ret = HPDF_Dict_Add (structure_element, "K", children)) != HPDF_OK)
            return ret;
    }

    if (pg == page) {
        // marked content sequence is contained in the same page as /Pg
        ret = HPDF_Array_AddNumber (children, mcid);
    } else {
        // marked content sequence resides on another page, need to add a marked-content reference
        HPDF_Dict mcr = HPDF_Dict_New (structure_element->mmgr);
        if (!mcr)
            return HPDF_Error_GetCode (structure_element->error);

        if ((ret = HPDF_Dict_AddName (mcr, "Type", "MCR")) != HPDF_OK)
            return ret;

        if ((ret = HPDF_Dict_Add (mcr, "Pg", page)) != HPDF_OK)
            return ret;

        if ((ret = HPDF_Dict_AddNumber (mcr, "MCID", mcid)) != HPDF_OK)
            return ret;

        ret = HPDF_Array_Add (children, mcr);
    }

    return ret;
}

HPDF_STATUS
HPDF_StructureElement_SetAlternateText  (HPDF_StructureElement  structure_element,
                                         const char            *alt,
                                         HPDF_UINT16            unicode_len)
{
    return HPDF_Dict_AddUnicodeEntry (structure_element, "Alt", alt, unicode_len);
}

HPDF_STATUS
HPDF_StructureElement_SetActualText  (HPDF_StructureElement  structure_element,
                                      const char            *actual_text,
                                      HPDF_UINT16            unicode_len)
{
    return HPDF_Dict_AddUnicodeEntry (structure_element, "ActualText", actual_text, unicode_len);
}

HPDF_STATUS
HPDF_Dict_AddUnicodeEntry  (HPDF_Dict    dict,
                            const char  *entry_name,
                            const char  *text,
                            HPDF_UINT16  unicode_len)
{
    HPDF_String s = HPDF_String_New_Unicode (dict->mmgr, text, unicode_len);
    if (!s)
        return HPDF_Error_GetCode (dict->error);

    return HPDF_Dict_Add (dict, entry_name, s);
}

HPDF_Dict
HPDF_StructureElement_GetAttributes  (HPDF_StructureElement  structure_element,
                                      const char            *owner)
{
    // TODO support multiple attribute objects with different owners
    HPDF_Dict attr = (HPDF_Dict)HPDF_Dict_GetItem (structure_element, "A", HPDF_OCLASS_DICT);
    if (!attr) {
        attr = HPDF_Dict_New (structure_element->mmgr);
        if (!attr)
            return NULL;

        if (HPDF_Dict_Add (structure_element, "A", attr) != HPDF_OK)
            return NULL;

        if (HPDF_Dict_AddName (attr, "O", owner) != HPDF_OK)
            return NULL;
    }

    return attr;
}

HPDF_STATUS
HPDF_TableHeaderCell_SetScope (HPDF_StructureElement     structure_element,
                               HPDF_TableHeaderCellScope scope)
{
    HPDF_Dict attr;

    HPDF_PTRACE ((" HPDF_TableHeaderCell_SetScope\n"));

    attr = HPDF_StructureElement_GetAttributes (structure_element, "Table");
    if (!attr)
        return HPDF_Error_GetCode (structure_element->error);

    return HPDF_Dict_AddName (attr, "Scope", HPDF_TABLE_HEADER_CELL_SCOPE_NAMES[(HPDF_INT)scope]);
}

HPDF_STATUS
HPDF_StructureElement_SetBBox (HPDF_StructureElement structure_element,
                               HPDF_Rect             rect)
{
    HPDF_STATUS ret;
    HPDF_Dict attr;
    HPDF_Array array;
    HPDF_REAL tmp;

    HPDF_PTRACE ((" HPDF_StructureElement_SetBBox\n"));

    attr = HPDF_StructureElement_GetAttributes (structure_element, "Layout");
    if (!attr)
        return HPDF_Error_GetCode (structure_element->error);

    array = HPDF_Array_New (attr->mmgr);
    if (!array)
        return HPDF_Error_GetCode (attr->error);

    if ((ret = HPDF_Dict_Add (attr, "BBox", array)) != HPDF_OK)
        return ret;

    if (rect.top < rect.bottom) {
        tmp = rect.top;
        rect.top = rect.bottom;
        rect.bottom = tmp;
    }

    ret += HPDF_Array_AddReal (array, rect.left);
    ret += HPDF_Array_AddReal (array, rect.bottom);
    ret += HPDF_Array_AddReal (array, rect.right);
    ret += HPDF_Array_AddReal (array, rect.top);

    return ret;
}

HPDF_STATUS
HPDF_StructureElement_SetPlacement (HPDF_StructureElement structure_element,
                                    HPDF_Placement        placement)
{
    HPDF_Dict attr;

    HPDF_PTRACE ((" HPDF_StructureElement_SetPlacement\n"));

    attr = HPDF_StructureElement_GetAttributes (structure_element, "Layout");
    if (!attr)
        return HPDF_Error_GetCode (structure_element->error);

    return HPDF_Dict_AddName (attr, "Placement", HPDF_PLACEMENT_NAMES[(HPDF_INT)placement]);
}

HPDF_Artifact
HPDF_Artifact_New  (HPDF_MMgr         mmgr,
                    HPDF_ArtifactType type)
{
    HPDF_Artifact artifact;
    HPDF_STATUS ret = HPDF_OK;

    HPDF_PTRACE((" HPDF_Artifact_New\n"));

    artifact = HPDF_Dict_New (mmgr);
    if (!artifact)
        return NULL;

    ret = HPDF_Dict_AddName (artifact, "Type", HPDF_ARTIFACT_TYPE_NAMES[(HPDF_INT)type]);
    if (ret != HPDF_OK)
        return NULL;

    return artifact;
}

HPDF_STATUS
HPDF_Artifact_SetSubtype  (HPDF_Artifact        artifact,
                           HPDF_ArtifactSubtype subtype)
{
    return HPDF_Dict_AddName (artifact, "Subtype", HPDF_ARTIFACT_SUBTYPE_NAMES[(HPDF_INT)subtype]);
}

HPDF_MarkedContent
HPDF_MarkedContent_New  (HPDF_MMgr   mmgr,
                         const char *tag)
{
    HPDF_MarkedContent marked_content;
    HPDF_MarkedContentAttr attr;

    HPDF_PTRACE((" HPDF_MarkedContent_New\n"));

    marked_content = HPDF_Dict_New (mmgr);
    if (!marked_content)
        return NULL;

    marked_content->free_fn = MarkedContent_OnFree;

    attr = HPDF_GetMem (mmgr, sizeof(HPDF_MarkedContentAttr_Rec));
    if (!attr) {
        HPDF_Dict_Free (marked_content);
        return NULL;
    }

    marked_content->attr = attr;
    HPDF_MemSet (attr, 0, sizeof(HPDF_MarkedContentAttr_Rec));

    attr->tag = tag;

    return marked_content;
}

static void
MarkedContent_OnFree  (HPDF_Dict obj)
{
    HPDF_MarkedContentAttr attr = (HPDF_MarkedContentAttr)obj->attr;

    HPDF_PTRACE((" HPDF_MarkedContent_OnFree\n"));

    if (attr)
        HPDF_FreeMem (obj->mmgr, attr);
}

HPDF_STATUS
HPDF_MarkedContent_SetAlternateText  (HPDF_MarkedContent  marked_content,
                                      const char         *alt,
                                      HPDF_UINT16         unicode_len)
{
    return HPDF_Dict_AddUnicodeEntry (marked_content, "Alt", alt, unicode_len);
}

HPDF_STATUS
HPDF_MarkedContent_SetActualText  (HPDF_MarkedContent  marked_content,
                                      const char      *actual_text,
                                      HPDF_UINT16      unicode_len)
{
    return HPDF_Dict_AddUnicodeEntry (marked_content, "ActualText", actual_text, unicode_len);
}
