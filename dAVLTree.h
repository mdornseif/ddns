/* $Id: dAVLTree.h,v 1.3 2000/11/21 19:28:22 drt Exp $
 *  --drt@un.bewaff.net
 *
 * iAVLTree.h: Header file for AVLTrees with ddns data.
 *
 * Based on:
 * iAVLTree.h: Header file for AVLTrees with long integer keys.
 * Copyright (C) 1998  Michael H. Buselli
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * The official web page for THE ORIGINAL product is:
 * http://www.tezcat.com/~cosine/pub/AVLTree/
 *
 * This is based on version 0.1.0 (alpha).
 *
 * THIS IS A MODIFIED VERSION FOR DDNS
 *
 * You might find more Information at http://rc23.cx/
 *
 * $Log: dAVLTree.h,v $
 * Revision 1.3  2000/11/21 19:28:22  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.2  2000/07/29 21:05:48  drt
 * renamed functions to dAVL*
 * now data is directly saved in the leaves to lessen
 * memory fragmentation and overhead.
 *
 */

#ifndef _DAVLTREE_H_
#define _DAVLTREE_H_

#include "uint32.h"

typedef struct _dAVLNode {

  uint32 key;  /* = uid */
  char ip4[4];
  char ip6[16];
  char loc[16];
  long depth;
  struct _dAVLNode *parent;
  struct _dAVLNode *left;
  struct _dAVLNode *right;
} dAVLNode;


typedef struct {
  dAVLNode *top;
  long count;
} dAVLTree;


typedef struct {
  const dAVLTree *avltree;
  const dAVLNode *curnode;
} dAVLCursor;


extern dAVLTree *dAVLAllocTree ();
int dAVLInsert (dAVLTree *avltree, uint32 uid, char *ip4, char *ip6, char *loc);
extern dAVLNode *dAVLFirst (dAVLCursor *avlcursor, dAVLTree const *avltree);
extern dAVLNode *dAVLNext (dAVLCursor *avlcursor);
extern int dAVLDelete (dAVLTree *avltree, uint32 key);

/*
extern void dAVLFreeTree (dAVLTree *avltree, void (freeitem)(void *item));
extern void *dAVLSearch (dAVLTree const *avltree, long key);
*/

#endif
