/* $Id: dAVLTree.c,v 1.4 2000/11/21 19:28:22 drt Exp $
 *  --drt@un.bewaff.net
 *
 * Source code for the AVLTree library (ddns version).
 *
 * Based on:
 * iAVLTree.c: Source code for the AVLTree library (long integer version).
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
 * The official web page OF THE ORIGINAL product is:
 * http://www.tezcat.com/~cosine/pub/AVLTree/
 *
 * This is based on is version 0.1.0 (alpha).
 *
 * THIS IS A MODIFIED VERSION FOR DDNS
 *
 * You might find more Information at http://rc23.cx/
 * 
 * $Log: dAVLTree.c,v $
 * Revision 1.4  2000/11/21 19:28:22  drt
 * Changed Email Address from drt@ailis.de to drt@un.bewaff.net
 *
 * Revision 1.3  2000/07/31 19:15:56  drt
 * ddns-file(5) format changed
 * a lot of restructuring
 *
 * Revision 1.2  2000/07/29 21:05:48  drt
 * renamed functions to dAVL*
 * now data is directly saved in the leaves to lessen
 * memory fragmentation and overhead.
 *
 */

#include <malloc.h>

#include "byte.h" 
#include "stralloc.h"

#include "dAVLTree.h"
#include "ddns.h"

static dAVLNode *dAVLCloseSearchNode (dAVLTree const *avltree, uint32 key);
static void dAVLRebalanceNode (dAVLTree *avltree, dAVLNode *avlnode);
static void dAVLFreeBranch (dAVLNode *avlnode);
static void dAVLFillVacancy (dAVLTree *avltree,
                             dAVLNode *origparent, dAVLNode **superparent,
                             dAVLNode *left, dAVLNode *right);

#define MAX(x, y)      ((x) > (y) ? (x) : (y))
#define MIN(x, y)      ((x) < (y) ? (x) : (y))
#define L_DEPTH(n)     ((n)->left ? (n)->left->depth : 0)
#define R_DEPTH(n)     ((n)->right ? (n)->right->depth : 0)
#define CALC_DEPTH(n)  (MAX(L_DEPTH(n), R_DEPTH(n)) + 1)


/*
 * dAVLAllocTree:
 * Allocate memory for a new AVL tree. 
 * On success, a pointer to the malloced iAVLTree is returned.  If there
 * was a malloc failure, then NULL is returned.
 */
dAVLTree *dAVLAllocTree ()
{
  dAVLTree *rc;

  rc = malloc(sizeof(dAVLTree));
  if (rc == NULL)
    return NULL;

  rc->top = NULL;
  rc->count = 0;
  return rc;
}


/*
 * dAVLFreeTree:
 * Free all memory used by this AVL tree.  If freeitem is not NULL, then
 * it is assumed to be a destructor for the items reference in the AVL
 * tree, and they are deleted as well.
 */
void dAVLFreeTree (dAVLTree *avltree)
{
  if (avltree->top)
    dAVLFreeBranch(avltree->top);
  free(avltree);
}


/*
 * dAVLInsert:
 * Create a new node and insert an item there.
 *
 * Returns  0 on success,
 *         -1 on malloc failure,
 *          3 if duplicate key.
 */
int dAVLInsert (dAVLTree *avltree, uint32 uid, char *ip4, char *ip6, char *loc)
{
  dAVLNode *newnode;
  dAVLNode *node;
  dAVLNode *balnode;
  dAVLNode *nextbalnode;

  newnode = malloc(sizeof(dAVLNode));
  if (newnode == NULL)
    return -1;
 
  newnode->key = uid;
  byte_copy(newnode->ip4, 4, ip4);
  byte_copy(newnode->ip6, 16, ip6);
  byte_copy(newnode->loc, 16, loc);
  newnode->depth = 1;
  newnode->left = NULL;
  newnode->right = NULL;
  newnode->parent = NULL;

  if (avltree->top != NULL) {
    node = dAVLCloseSearchNode(avltree, newnode->key);

    /* XXX: handle this more intelligent */ 
    if (node->key == newnode->key) {
      free(newnode);
      return 3;
    }

    newnode->parent = node;

    if (newnode->key < node->key) {
      node->left = newnode;
      node->depth = CALC_DEPTH(node);
    }

    else {
      node->right = newnode;
      node->depth = CALC_DEPTH(node);
    }

    for (balnode = node->parent; balnode; balnode = nextbalnode) {
      nextbalnode = balnode->parent;
      dAVLRebalanceNode(avltree, balnode);
    }
  }

  else {
    avltree->top = newnode;
  }

  avltree->count++;
  return 0;
}


/*
 * dAVLSearch:
 * Return a pointer to the item with the given key in the AVL tree.  If
 * no such item is in the tree, then NULL is returned.
 */
uint32 dAVLSearch (dAVLTree const *avltree, long key)
{
  dAVLNode *node;

  node = dAVLCloseSearchNode(avltree, key);

  if (node && node->key == key)
    return key;

  return 0;
}


/*
 * dAVLDelete:
 * Deletes the node with the given key.  Does not delete the item at
 * that key.  Returns 0 on success and -1 if a node with the given key
 * does not exist.
 */
int dAVLDelete (dAVLTree *avltree, uint32 key)
{
  dAVLNode *avlnode;
  dAVLNode *origparent; 
  dAVLNode **superparent;
 
  avlnode = dAVLCloseSearchNode(avltree, key); 
  if (avlnode == NULL || avlnode->key != key)
    return -1;
 
  origparent = avlnode->parent;
 
  if (origparent) {
    if (avlnode->key < avlnode->parent->key) 
      superparent = &(avlnode->parent->left);
    else 
      superparent = &(avlnode->parent->right);
  }
  else
    superparent = &(avltree->top);

  dAVLFillVacancy(avltree, origparent, superparent, 
                  avlnode->left, avlnode->right);
  free(avlnode);
  avltree->count--;
  return 0;
}


/*
 * dAVLFirst:
 * Initializes an iAVLCursor object and returns the item with the lowest
 * key in the iAVLTree.
 */
dAVLNode *dAVLFirst (dAVLCursor *avlcursor, dAVLTree const *avltree)
{
  const dAVLNode *avlnode;

  avlcursor->avltree = avltree;

  if (avltree->top == NULL) {
    avlcursor->curnode = NULL;
    return NULL;
  }

  for (avlnode = avltree->top;
       avlnode->left != NULL;
       avlnode = avlnode->left);
  avlcursor->curnode = avlnode;
  return avlnode;
}


/*
 * dAVLNext:
 * Called after an dAVLFirst() call, this returns the item with the least
 * key that is greater than the last item returned either by iAVLFirst()
 * or a previous invokation of this function.
 */
dAVLNode *dAVLNext (dAVLCursor *avlcursor)
{
  const dAVLNode *avlnode;

  avlnode = avlcursor->curnode;

  if (avlnode->right != NULL) {
    for (avlnode = avlnode->right;
         avlnode->left != NULL;
         avlnode = avlnode->left);
    avlcursor->curnode = avlnode;
    return avlnode;
  }

  while (avlnode->parent && avlnode->parent->left != avlnode) {
    avlnode = avlnode->parent;
  }

  if (avlnode->parent == NULL) {
    avlcursor->curnode = NULL;
    return NULL;
  }

  avlcursor->curnode = avlnode->parent;
  return avlnode->parent;
}

/*
 * dAVLCloseSearchNode:
 * Return a pointer to the node closest to the given key.
 * Returns NULL if the AVL tree is empty.
 */
dAVLNode *dAVLCloseSearchNode (dAVLTree const *avltree, uint32 key)
{
  dAVLNode *node;

  node = avltree->top;

  if (!node)
    return NULL;

  for (;;) {
    if (node->key == key)
      return node;

    if (node->key < key) {
      if (node->right)
        node = node->right;
      else
        return node;
    }

    else {
      if (node->left)
        node = node->left;
      else
        return node;
    }
  }
}


/*
 * dAVLRebalanceNode:
 * Rebalances the AVL tree if one side becomes too heavy.  This function
 * assumes that both subtrees are AVL trees with consistant data.  This
 * function has the additional side effect of recalculating the depth of
 * the tree at this node.  It should be noted that at the return of this
 * function, if a rebalance takes place, the top of this subtree is no
 * longer going to be the same node.
 */
void dAVLRebalanceNode (dAVLTree *avltree, dAVLNode *avlnode)
{
  long depthdiff;
  dAVLNode *child;
  dAVLNode *gchild;
  dAVLNode *origparent;
  dAVLNode **superparent;

  origparent = avlnode->parent;

  if (origparent) {
    if (avlnode->key < avlnode->parent->key)
      superparent = &(avlnode->parent->left);
    else
      superparent = &(avlnode->parent->right);
  }
  else
    superparent = &(avltree->top);

  depthdiff = R_DEPTH(avlnode) - L_DEPTH(avlnode);

  if (depthdiff <= -2) {
    child = avlnode->left;

    if (L_DEPTH(child) >= R_DEPTH(child)) {
      avlnode->left = child->right;
      if (avlnode->left != NULL)
        avlnode->left->parent = avlnode;
      avlnode->depth = CALC_DEPTH(avlnode);
      child->right = avlnode;
      if (child->right != NULL)
        child->right->parent = child;
      child->depth = CALC_DEPTH(child);
      *superparent = child;
      child->parent = origparent;
    }

    else {
      gchild = child->right;
      avlnode->left = gchild->right;
      if (avlnode->left != NULL)
        avlnode->left->parent = avlnode;
      avlnode->depth = CALC_DEPTH(avlnode);
      child->right = gchild->left;
      if (child->right != NULL)
        child->right->parent = child;
      child->depth = CALC_DEPTH(child);
      gchild->right = avlnode;
      if (gchild->right != NULL)
        gchild->right->parent = gchild;
      gchild->left = child;
      if (gchild->left != NULL)
        gchild->left->parent = gchild;
      gchild->depth = CALC_DEPTH(gchild);
      *superparent = gchild;
      gchild->parent = origparent;
    }
  }

  else if (depthdiff >= 2) {
    child = avlnode->right;

    if (R_DEPTH(child) >= L_DEPTH(child)) {
      avlnode->right = child->left;
      if (avlnode->right != NULL)
        avlnode->right->parent = avlnode;
      avlnode->depth = CALC_DEPTH(avlnode);
      child->left = avlnode;
      if (child->left != NULL)
        child->left->parent = child;
      child->depth = CALC_DEPTH(child);
      *superparent = child;
      child->parent = origparent;
    }

    else {
      gchild = child->left;
      avlnode->right = gchild->left;
      if (avlnode->right != NULL)
        avlnode->right->parent = avlnode;
      avlnode->depth = CALC_DEPTH(avlnode);
      child->left = gchild->right;
      if (child->left != NULL)
        child->left->parent = child;
      child->depth = CALC_DEPTH(child);
      gchild->left = avlnode;
      if (gchild->left != NULL)
        gchild->left->parent = gchild;
      gchild->right = child;
      if (gchild->right != NULL)
        gchild->right->parent = gchild;
      gchild->depth = CALC_DEPTH(gchild);
      *superparent = gchild;
      gchild->parent = origparent;
    }
  }

  else {
    avlnode->depth = CALC_DEPTH(avlnode);
  }
}


/*
 * dAVLFreeBranch:
 * Free memory used by this node and its item. 
 */
void dAVLFreeBranch (dAVLNode *avlnode)
{
  if (avlnode->left)
    dAVLFreeBranch(avlnode->left);
  if (avlnode->right)
    dAVLFreeBranch(avlnode->right);
  free(avlnode);
}


/*
 * dAVLFillVacancy:
 * Given a vacancy in the AVL tree by it's parent, children, and parent
 * component pointer, fill that vacancy.
 */
void dAVLFillVacancy (dAVLTree *avltree,
                      dAVLNode *origparent, dAVLNode **superparent,
                      dAVLNode *left, dAVLNode *right)
{
  dAVLNode *avlnode;
  dAVLNode *balnode;
  dAVLNode *nextbalnode;

  if (left == NULL) {
    if (right == NULL) {
      *superparent = NULL;
      return;
    }

    *superparent = right;
    right->parent = origparent;
    balnode = origparent;
  }

  else {
    for (avlnode = left; avlnode->right != NULL; avlnode = avlnode->right);

    if (avlnode == left) {
      balnode = avlnode;
    }
    else {
      balnode = avlnode->parent;
      balnode->right = avlnode->left;
      if (balnode->right != NULL)
        balnode->right->parent = balnode;
      avlnode->left = left;
      left->parent = avlnode;
    }

    avlnode->right = right;
    if (right != NULL)
      right->parent = avlnode;
    *superparent = avlnode;
    avlnode->parent = origparent;
  }

  for (; balnode; balnode = nextbalnode) {
    nextbalnode = balnode->parent;
    dAVLRebalanceNode(avltree, balnode);
  }
}

