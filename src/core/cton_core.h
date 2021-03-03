/*******************************************************************************
 * "THE BEER-WARE LICENSE" (Revision 43):
 * <yeonji@ieee.org> create this project. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a sweety curry rice in return (cuz
 * I cannot drink alcoholic beverages). Yeonji Lee
 *
 ******************************************************************************/

#ifndef _CTON_CORELIB_HEADER_
#define _CTON_CORELIB_HEADER_ 1

#include <inttypes.h>
#include <stddef.h>

#define _CTON_CORELIB_ 1

#include <cton.h>
#include <auto_config.h>

#define CTON_STRUCT_MAGIC 0x4E4F5443

/*
 * cton_objtype(obj)
 *
 * DESCRIPTION
 *   Same as cton_object_gettype(), but only available for internal useage.
 *
 * PARAMETER
 *   obj: The object which you want to get it's type
 *
 * RETURN
 *   The type of given object.
 */
#define cton_objtype(obj) ((obj)->type)

#endif /* _CTON_CORELIB_HEADER_ */
