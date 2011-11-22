/* $Id$
 * --------------------------------------------------------------------------
 *
 *              //===//   //=====   //===//   //=====  //   //      //
 *             //    //  //        //    //  //       //   //=/  /=//
 *            //===//   //=====   //===//   //====   //   //  //  //
 *           //   \\         //  //             //  //   //  //  //
 *          //     \\  =====//  //        =====//  //   //      //  Version V
 *
 * ------------- An Open Source RSerPool Simulation for OMNeT++ -------------
 *
 * Copyright (C) 2003-2010 by Thomas Dreibholz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact: dreibh@iem.uni-due.de
 */

#include <stdlib.h>
#include <stdio.h>

#include "binarytree.h"
#include "debug.h"


#ifdef __cplusplus
extern "C" {
#endif


static struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeInternalFindPrev)(
                                                const struct BT_DEFINITION(BinaryTree)*     bt,
                                                const struct BT_DEFINITION(BinaryTreeNode)* cmpNode);
static struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeInternalFindNext)(
                                                const struct BT_DEFINITION(BinaryTree)*     bt,
                                                const struct BT_DEFINITION(BinaryTreeNode)* cmpNode);


/* ###### Initialize ##################################################### */
void BT_FUNCTION(BinaryTreeNodeNew)(
        struct BT_DEFINITION(BinaryTreeNode)* node)
{
#ifdef USE_LEAFLINKED
   doubleLinkedRingListNodeNew(&node->ListNode);
#endif
   node->Parent       = NULL;
   node->LeftSubtree  = NULL;
   node->RightSubtree = NULL;
   node->Value        = 0;
   node->ValueSum     = 0;
}


/* ###### Invalidate ##################################################### */
void BT_FUNCTION(BinaryTreeNodeDelete)(
        struct BT_DEFINITION(BinaryTreeNode)* node)
{
   node->Parent       = NULL;
   node->LeftSubtree  = NULL;
   node->RightSubtree = NULL;
   node->Value        = 0;
   node->ValueSum     = 0;
#ifdef USE_LEAFLINKED
   doubleLinkedRingListNodeDelete(&node->ListNode);
#endif
}


/* ###### Is node linked? ################################################ */
int BT_FUNCTION(BinaryTreeNodeIsLinked)(
       const struct BT_DEFINITION(BinaryTreeNode)* node)
{
   return(node->LeftSubtree != NULL);
}


/* ##### Initialize ###################################################### */
void BT_FUNCTION(BinaryTreeNew)(
        struct BT_DEFINITION(BinaryTree)* bt,
        void                              (*printFunction)(const void* node, FILE* fd),
        int                               (*comparisonFunction)(const void* node1, const void* node2))
{
#ifdef USE_LEAFLINKED
   doubleLinkedRingListNew(&bt->List);
#endif
   bt->PrintFunction         = printFunction;
   bt->ComparisonFunction    = comparisonFunction;
   bt->NullNode.Parent       = &bt->NullNode;
   bt->NullNode.LeftSubtree  = &bt->NullNode;
   bt->NullNode.RightSubtree = &bt->NullNode;
   bt->NullNode.Value        = 0;
   bt->NullNode.ValueSum     = 0;
   bt->Elements              = 0;
}


/* ##### Invalidate ###################################################### */
void BT_FUNCTION(BinaryTreeDelete)(
        struct BT_DEFINITION(BinaryTree)* bt)
{
   bt->Elements              = 0;
   bt->NullNode.Parent       = NULL;
   bt->NullNode.LeftSubtree  = NULL;
   bt->NullNode.RightSubtree = NULL;
#ifdef USE_LEAFLINKED
   doubleLinkedRingListDelete(&bt->List);
#endif
}


/* ##### Update value sum ################################################ */
inline static void BT_FUNCTION(BinaryTreeUpdateValueSum)(
                 struct BT_DEFINITION(BinaryTreeNode)* node)
{
   node->ValueSum = node->LeftSubtree->ValueSum + node->Value + node->RightSubtree->ValueSum;
}


/* ##### Update value sum for node and all parents up to tree root ####### */
inline static void BT_FUNCTION(BinaryTreeUpdateValueSumsUpToRoot)(
                      struct BT_DEFINITION(BinaryTree)*     bt,
                      struct BT_DEFINITION(BinaryTreeNode)* node)
{
   while(node != &bt->NullNode) {
       BT_FUNCTION(BinaryTreeUpdateValueSum)(node);
       node = node->Parent;
   }
}


/* ###### Internal method for printing a node ############################# */
static void BT_FUNCTION(BinaryTreePrintNode)(
               const struct BT_DEFINITION(BinaryTree)*     bt,
               const struct BT_DEFINITION(BinaryTreeNode)* node,
               FILE*                                       fd)
{
   bt->PrintFunction(node, fd);
#ifdef DEBUG
   fprintf(fd, " ptr=%p v=%llu vsum=%llu",
           node, node->Value, node->ValueSum);
   if(node->LeftSubtree != &bt->NullNode) {
      fprintf(fd, " l=%p[", node->LeftSubtree);
      bt->PrintFunction(node->LeftSubtree, fd);
      fprintf(fd, "]");
   }
   else {
      fprintf(fd, " l=()");
   }
   if(node->RightSubtree != &bt->NullNode) {
      fprintf(fd, " r=%p[", node->RightSubtree);
      bt->PrintFunction(node->RightSubtree, fd);
      fprintf(fd, "]");
   }
   else {
      fprintf(fd, " r=()");
   }
   if(node->Parent != &bt->NullNode) {
      fprintf(fd, " p=%p[", node->Parent);
      bt->PrintFunction(node->Parent, fd);
      fprintf(fd, "]   ");
   }
   else {
      fprintf(fd, " p=())   ");
   }
   fputs("\n", fd);
#endif
}


/* ##### Internal printing function ###################################### */
static void BT_FUNCTION(BinaryTreeInternalPrint)(
               const struct BT_DEFINITION(BinaryTree)*     bt,
               const struct BT_DEFINITION(BinaryTreeNode)* node,
               FILE*                                       fd)
{
   if(node != &bt->NullNode) {
      BT_FUNCTION(BinaryTreeInternalPrint)(bt, node->LeftSubtree, fd);
      BT_FUNCTION(BinaryTreePrintNode)(bt, node, fd);
      BT_FUNCTION(BinaryTreeInternalPrint)(bt, node->RightSubtree, fd);
   }
}


/* ###### Print tree ##################################################### */
void BT_FUNCTION(BinaryTreePrint)(
        const struct BT_DEFINITION(BinaryTree)* bt,
        FILE*                                   fd)
{
#ifdef DEBUG
   fprintf(fd, "\n\nroot=%p[", bt->NullNode.LeftSubtree);
   if(bt->NullNode.LeftSubtree != &bt->NullNode) {
      bt->PrintFunction(bt->NullNode.LeftSubtree, fd);
   }
   fprintf(fd, "] null=%p   \n", &bt->NullNode);
#endif
   BT_FUNCTION(BinaryTreeInternalPrint)(bt, bt->NullNode.LeftSubtree, fd);
   fputs("\n", fd);
}


/* ###### Is tree empty? ################################################# */
int BT_FUNCTION(BinaryTreeIsEmpty)(
       const struct BT_DEFINITION(BinaryTree)* bt)
{
   return(bt->NullNode.LeftSubtree == &bt->NullNode);
}


/* ###### Get first node ################################################## */
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetFirst)(
                                         const struct BT_DEFINITION(BinaryTree)* bt)
{
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingListNode* node = bt->List.Node.Next;
   if(node != bt->List.Head) {
      return((struct BT_DEFINITION(BinaryTreeNode)*)node);
   }
   return(NULL);
#else
   const struct BT_DEFINITION(BinaryTreeNode)* node = bt->NullNode.LeftSubtree;
   if(node == &bt->NullNode) {
      node = bt->NullNode.RightSubtree;
   }
   while(node->LeftSubtree != &bt->NullNode) {
      node = node->LeftSubtree;
   }
   if(node != &bt->NullNode) {
      return((struct BT_DEFINITION(BinaryTreeNode)*)node);
   }
   return(NULL);
#endif
}


/* ###### Get last node ################################################### */
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetLast)(
                                         const struct BT_DEFINITION(BinaryTree)* bt)
{
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingListNode* node = bt->List.Node.Prev;
   if(node != bt->List.Head) {
      return((struct BT_DEFINITION(BinaryTreeNode)*)node);
   }
   return(NULL);
#else
   const struct BT_DEFINITION(BinaryTreeNode)* node = bt->NullNode.RightSubtree;
   if(node == &bt->NullNode) {
      node = bt->NullNode.LeftSubtree;
   }
   while(node->RightSubtree != &bt->NullNode) {
      node = node->RightSubtree;
   }
   if(node != &bt->NullNode) {
      return((struct BT_DEFINITION(BinaryTreeNode)*)node);
   }
   return(NULL);
#endif
}


/* ###### Get previous node ############################################### */
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetPrev)(
                                         const struct BT_DEFINITION(BinaryTree)*     bt,
                                         const struct BT_DEFINITION(BinaryTreeNode)* node)
{
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingListNode* prev = node->ListNode.Prev;
   if(prev != bt->List.Head) {
      return((struct BT_DEFINITION(BinaryTreeNode)*)prev);
   }
   return(NULL);
#else
   struct BT_DEFINITION(BinaryTreeNode)* result;
   result = BT_FUNCTION(BinaryTreeInternalFindPrev)(bt, node);
   if(result != &bt->NullNode) {
      return(result);
   }
   return(NULL);
#endif
}


/* ###### Get next node ################################################## */
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetNext)(
                                         const struct BT_DEFINITION(BinaryTree)*     bt,
                                         const struct BT_DEFINITION(BinaryTreeNode)* node)
{
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingListNode* next = node->ListNode.Next;
   if(next != bt->List.Head) {
      return((struct BT_DEFINITION(BinaryTreeNode)*)next);
   }
   return(NULL);
#else
   struct BT_DEFINITION(BinaryTreeNode)* result;
   result = BT_FUNCTION(BinaryTreeInternalFindNext)(bt, node);
   if(result != &bt->NullNode) {
      return(result);
   }
   return(NULL);
#endif
}


/* ###### Find nearest previous node ##################################### */
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetNearestPrev)(
                                         const struct BT_DEFINITION(BinaryTree)*     bt,
                                         const struct BT_DEFINITION(BinaryTreeNode)* cmpNode)
{
   struct BT_DEFINITION(BinaryTreeNode)*const* nodePtr;
   struct BT_DEFINITION(BinaryTreeNode)*const* parentPtr;
   const struct BT_DEFINITION(BinaryTreeNode)* node;
   const struct BT_DEFINITION(BinaryTreeNode)* parent;
   int                                         cmpResult = 0;

#ifdef DEBUG
   printf("nearest prev: ");
   bt->PrintFunction(cmpNode, stdout);
   printf("\n");
   BT_FUNCTION(BinaryTreePrint)(bt, stdout);
#endif

   parentPtr = NULL;
   nodePtr   = &bt->NullNode.LeftSubtree;
   while(*nodePtr != &bt->NullNode) {
      cmpResult = bt->ComparisonFunction(cmpNode, *nodePtr);
      if(cmpResult < 0) {
         parentPtr = nodePtr;
         nodePtr   = &(*nodePtr)->LeftSubtree;
      }
      else if(cmpResult > 0) {
         parentPtr = nodePtr;
         nodePtr   = &(*nodePtr)->RightSubtree;
      }
      if(cmpResult == 0) {
         return(BT_FUNCTION(BinaryTreeGetPrev)(bt, *nodePtr));
      }
   }

   if(parentPtr == NULL) {
      if(cmpResult > 0) {
         return(bt->NullNode.LeftSubtree);
      }
      return(NULL);
   }
   else {
      /* The new node would be the right child of its parent.
         => The parent is the nearest previous node! */
      if(nodePtr == &(*parentPtr)->RightSubtree) {
         return(*parentPtr);
      }
      else {
         parent = *parentPtr;

         /* If there is a left subtree, the nearest previous node is the
            rightmost child of the left subtree. */
         if(parent->LeftSubtree != &bt->NullNode) {
            node = parent->LeftSubtree;
            while(node->RightSubtree != &bt->NullNode) {
               node = node->RightSubtree;
            }
            if(node != &bt->NullNode) {
               return((struct BT_DEFINITION(BinaryTreeNode)*)node);
            }
         }

         /* If there is no left subtree, the nearest previous node is an
            ancestor node which has the node on its right side. */
         else {
            node   = parent;
            parent = node->Parent;
            while((parent != &bt->NullNode) && (node == parent->LeftSubtree)) {
               node   = parent;
               parent = parent->Parent;
            }
            if(parent != &bt->NullNode) {
               return((struct BT_DEFINITION(BinaryTreeNode)*)parent);
            }
         }
      }
   }
   return(NULL);
}


/* ###### Find nearest next node ######################################### */
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetNearestNext)(
                                         const struct BT_DEFINITION(BinaryTree)*     bt,
                                         const struct BT_DEFINITION(BinaryTreeNode)* cmpNode)
{
   struct BT_DEFINITION(BinaryTreeNode)*const* nodePtr;
   struct BT_DEFINITION(BinaryTreeNode)*const* parentPtr;
   const struct BT_DEFINITION(BinaryTreeNode)* node;
   const struct BT_DEFINITION(BinaryTreeNode)* parent;
   int                                         cmpResult = 0;

#ifdef DEBUG
   printf("nearest next: ");
   bt->PrintFunction(cmpNode, stdout);
   printf("\n");
   BT_FUNCTION(BinaryTreePrint)(bt, stdout);
#endif

   parentPtr = NULL;
   nodePtr   = &bt->NullNode.LeftSubtree;
   while(*nodePtr != &bt->NullNode) {
      cmpResult = bt->ComparisonFunction(cmpNode, *nodePtr);
      if(cmpResult < 0) {
         parentPtr = nodePtr;
         nodePtr   = &(*nodePtr)->LeftSubtree;
      }
      else if(cmpResult > 0) {
         parentPtr = nodePtr;
         nodePtr   = &(*nodePtr)->RightSubtree;
      }
      if(cmpResult == 0) {
         return(BT_FUNCTION(BinaryTreeGetNext)(bt, *nodePtr));
      }
   }

   if(parentPtr == NULL) {
      if(cmpResult < 0) {
         return(bt->NullNode.LeftSubtree);
      }
      return(NULL);
   }
   else {
      /* The new node would be the left child of its parent.
         => The parent is the nearest next node! */
      if(nodePtr == &(*parentPtr)->LeftSubtree) {
         return(*parentPtr);
      }
      else {
         parent = *parentPtr;

         /* If there is a right subtree, the nearest next node is the
            leftmost child of the right subtree. */
         if(parent->RightSubtree != &bt->NullNode) {
            node = parent->RightSubtree;
            while(node->LeftSubtree != &bt->NullNode) {
               node = node->LeftSubtree;
            }
            if(node != &bt->NullNode) {
               return((struct BT_DEFINITION(BinaryTreeNode)*)node);
            }
         }

         /* If there is no right subtree, the nearest next node is an
            ancestor node which has the node on its left side. */
         else {
            node   = parent;
            parent = node->Parent;
            while((parent != &bt->NullNode) && (node == parent->RightSubtree)) {
               node   = parent;
               parent = parent->Parent;
            }
            if(parent != &bt->NullNode) {
               return((struct BT_DEFINITION(BinaryTreeNode)*)parent);
            }
         }
      }
   }
   return(NULL);
}


/* ###### Get number of elements ########################################## */
size_t BT_FUNCTION(BinaryTreeGetElements)(
          const struct BT_DEFINITION(BinaryTree)* bt)
{
   return(bt->Elements);
}


/* ###### Get prev node by walking through the tree (does *not* use list!) */
static struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeInternalFindPrev)(
                                                const struct BT_DEFINITION(BinaryTree)*     bt,
                                                const struct BT_DEFINITION(BinaryTreeNode)* cmpNode)
{
   const struct BT_DEFINITION(BinaryTreeNode)* node = cmpNode->LeftSubtree;
   const struct BT_DEFINITION(BinaryTreeNode)* parent;

   if(node != &bt->NullNode) {
      while(node->RightSubtree != &bt->NullNode) {
         node = node->RightSubtree;
      }
      return((struct BT_DEFINITION(BinaryTreeNode)*)node);
   }
   else {
      node   = cmpNode;
      parent = cmpNode->Parent;
      while((parent != &bt->NullNode) && (node == parent->LeftSubtree)) {
         node   = parent;
         parent = parent->Parent;
      }
      return((struct BT_DEFINITION(BinaryTreeNode)*)parent);
   }
}


/* ###### Get next node by walking through the tree (does *not* use list!) */
static struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeInternalFindNext)(
                                                const struct BT_DEFINITION(BinaryTree)*     bt,
                                                const struct BT_DEFINITION(BinaryTreeNode)* cmpNode)
{
   const struct BT_DEFINITION(BinaryTreeNode)* node = cmpNode->RightSubtree;
   const struct BT_DEFINITION(BinaryTreeNode)* parent;

   if(node != &bt->NullNode) {
      while(node->LeftSubtree != &bt->NullNode) {
         node = node->LeftSubtree;
      }
      return((struct BT_DEFINITION(BinaryTreeNode)*)node);
   }
   else {
      node   = cmpNode;
      parent = cmpNode->Parent;
      while((parent != &bt->NullNode) && (node == parent->RightSubtree)) {
         node   = parent;
         parent = parent->Parent;
      }
      return((struct BT_DEFINITION(BinaryTreeNode)*)parent);
   }
}


/* ###### Find node ####################################################### */
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeFind)(
                                         const struct BT_DEFINITION(BinaryTree)*     bt,
                                         const struct BT_DEFINITION(BinaryTreeNode)* cmpNode)
{
#ifdef DEBUG
   printf("find: ");
   bt->PrintFunction(cmpNode, stdout);
   printf("\n");
#endif

   struct BT_DEFINITION(BinaryTreeNode)* node = bt->NullNode.LeftSubtree;
   while(node != &bt->NullNode) {
      const int cmpResult = bt->ComparisonFunction(cmpNode, node);
      if(cmpResult == 0) {
         return(node);
      }
      else if(cmpResult < 0) {
         node = node->LeftSubtree;
      }
      else {
         node = node->RightSubtree;
      }
   }
   return(NULL);
}


/* ###### Get value sum from root node ################################### */
BinaryTreeNodeValueType BT_FUNCTION(BinaryTreeGetValueSum)(
                             const struct BT_DEFINITION(BinaryTree)* bt)
{
   return(bt->NullNode.LeftSubtree->ValueSum);
}


/* ###### Insert ######################################################### */
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeInsert)(
                                         struct BT_DEFINITION(BinaryTree)*     bt,
                                         struct BT_DEFINITION(BinaryTreeNode)* node)
{
   int                                   cmpResult = -1;
   struct BT_DEFINITION(BinaryTreeNode)* where     = bt->NullNode.LeftSubtree;
   struct BT_DEFINITION(BinaryTreeNode)* parent    = &bt->NullNode;
   struct BT_DEFINITION(BinaryTreeNode)* result;
#ifdef USE_LEAFLINKED
   struct BT_DEFINITION(BinaryTreeNode)* prev;
#endif
#ifdef DEBUG
   printf("insert: ");
   bt->PrintFunction(node, stdout);
   printf("\n");
#endif


   /* ====== Find location of new node =================================== */
   while(where != &bt->NullNode) {
      parent = where;
      cmpResult = bt->ComparisonFunction(node, where);
      if(cmpResult < 0) {
         where = where->LeftSubtree;
      }
      else if(cmpResult > 0) {
         where = where->RightSubtree;
      }
      else {
         /* Node with same key is already available -> return. */
         result = where;
         goto finished;
      }
   }
   CHECK(where == &bt->NullNode);

   if(cmpResult < 0) {
      parent->LeftSubtree = node;
   }
   else {
      parent->RightSubtree = node;
   }


   /* ====== Link node =================================================== */
   node->Parent       = parent;
   node->LeftSubtree  = &bt->NullNode;
   node->RightSubtree = &bt->NullNode;
   node->ValueSum     = node->Value;
#ifdef USE_LEAFLINKED
   prev = BT_FUNCTION(BinaryTreeInternalFindPrev)(bt, node);
   if(prev != &bt->NullNode) {
      doubleLinkedRingListAddAfter(&prev->ListNode, &node->ListNode);
   }
   else {
      doubleLinkedRingListAddHead(&bt->List, &node->ListNode);
   }
#endif
   bt->Elements++;
   result = node;

   /* ====== Update parent's value sum =================================== */
   BT_FUNCTION(BinaryTreeUpdateValueSumsUpToRoot)(bt, node->Parent);

finished:
#ifdef DEBUG
   BT_FUNCTION(BinaryTreePrint)(bt, stdout);
#endif
#ifdef VERIFY
   BT_FUNCTION(BinaryTreeVerify)(bt);
#endif
   return(result);
}


/* ###### Remove ######################################################### */
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeRemove)(
                                         struct BT_DEFINITION(BinaryTree)*     bt,
                                         struct BT_DEFINITION(BinaryTreeNode)* node)
{
   struct BT_DEFINITION(BinaryTreeNode)* child;
   struct BT_DEFINITION(BinaryTreeNode)* delParent;
   struct BT_DEFINITION(BinaryTreeNode)* next;
   struct BT_DEFINITION(BinaryTreeNode)* nextParent;
#ifdef DEBUG
   printf("remove: ");
   bt->PrintFunction(node, stdout);
   printf("\n");
#endif

   CHECK(BT_FUNCTION(BinaryTreeNodeIsLinked)(node));

   /* ====== Unlink node ================================================= */
   if((node->LeftSubtree != &bt->NullNode) && (node->RightSubtree != &bt->NullNode)) {
      next       = BT_FUNCTION(BinaryTreeGetNext)(bt, node);
      nextParent = next->Parent;

      CHECK(next != &bt->NullNode);
      CHECK(next->Parent != &bt->NullNode);
      CHECK(next->LeftSubtree == &bt->NullNode);

      child         = next->RightSubtree;
      child->Parent = nextParent;
      if(nextParent->LeftSubtree == next) {
         nextParent->LeftSubtree = child;
      } else {
         CHECK(nextParent->RightSubtree == next);
         nextParent->RightSubtree = child;
      }


      delParent                  = node->Parent;
      next->Parent               = delParent;
      next->LeftSubtree          = node->LeftSubtree;
      next->RightSubtree         = node->RightSubtree;
      next->LeftSubtree->Parent  = next;
      next->RightSubtree->Parent = next;

      if(delParent->LeftSubtree == node) {
         delParent->LeftSubtree = next;
      } else {
         CHECK(delParent->RightSubtree == node);
         delParent->RightSubtree = next;
      }

      /* ====== Update parent's value sum ================================ */
      BT_FUNCTION(BinaryTreeUpdateValueSumsUpToRoot)(bt, next);
      BT_FUNCTION(BinaryTreeUpdateValueSumsUpToRoot)(bt, nextParent);
   } else {
      CHECK(node != &bt->NullNode);
      CHECK((node->LeftSubtree == &bt->NullNode) || (node->RightSubtree == &bt->NullNode));

      child         = (node->LeftSubtree != &bt->NullNode) ? node->LeftSubtree : node->RightSubtree;
      child->Parent = delParent = node->Parent;

      if(node == delParent->LeftSubtree) {
         delParent->LeftSubtree = child;
      } else {
         CHECK(node == delParent->RightSubtree);
         delParent->RightSubtree = child;
      }

      /* ====== Update parent's value sum ================================ */
      BT_FUNCTION(BinaryTreeUpdateValueSumsUpToRoot)(bt, delParent);
   }


   /* ====== Unlink node from list and invalidate pointers =============== */
   node->Parent       = NULL;
   node->RightSubtree = NULL;
   node->LeftSubtree  = NULL;
#ifdef USE_LEAFLINKED
   doubleLinkedRingListRemNode(&node->ListNode);
   node->ListNode.Prev = NULL;
   node->ListNode.Next = NULL;
#endif
   CHECK(bt->Elements > 0);
   bt->Elements--;

#ifdef DEBUG
    BT_FUNCTION(BinaryTreePrint)(bt, stdout);
#endif
#ifdef VERIFY
    BT_FUNCTION(BinaryTreeVerify)(bt);
#endif
   return(node);
}


/* ##### Get node by value ############################################### */
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetNodeByValue)(
                                         struct BT_DEFINITION(BinaryTree)* bt,
                                         BinaryTreeNodeValueType           value)
{
   struct BT_DEFINITION(BinaryTreeNode)* node = bt->NullNode.LeftSubtree;
   for(;;) {
      if(value < node->LeftSubtree->ValueSum) {
         if(node->LeftSubtree != &bt->NullNode) {
            node = node->LeftSubtree;
         }
         else {
            break;
         }
      }
      else if(value < node->LeftSubtree->ValueSum + node->Value) {
         break;
      }
      else {
         if(node->RightSubtree != &bt->NullNode) {
            value -= node->LeftSubtree->ValueSum + node->Value;
            node = node->RightSubtree;
         }
         else {
            break;
         }
      }
   }

   if(node !=  &bt->NullNode) {
      return(node);
   }
   return(NULL);
}


/* ##### Internal verification function ################################## */
static void BT_FUNCTION(BinaryTreeInternalVerify)(
               struct BT_DEFINITION(BinaryTree)*      bt,
               struct BT_DEFINITION(BinaryTreeNode)*  parent,
               struct BT_DEFINITION(BinaryTreeNode)*  node,
               struct BT_DEFINITION(BinaryTreeNode)** lastBinaryTreeNode,
#ifdef USE_LEAFLINKED
               struct DoubleLinkedRingListNode**      lastListNode,
#endif
               size_t*                                counter)
{
#ifdef USE_LEAFLINKED
   struct BT_DEFINITION(BinaryTreeNode)* prev;
   struct BT_DEFINITION(BinaryTreeNode)* next;
#endif

   if(node != &bt->NullNode) {
      /* ====== Print node =============================================== */
#ifdef DEBUG
      printf("verifying ");
      BT_FUNCTION(BinaryTreePrintNode)(bt, node, stdout);
      puts("");
#endif

      /* ====== Correct parent? ========================================== */
      CHECK(node->Parent == parent);

      /* ====== Correct tree and heap properties? ======================== */
      if(node->LeftSubtree != &bt->NullNode) {
         CHECK(bt->ComparisonFunction(node, node->LeftSubtree) > 0);
      }
      if(node->RightSubtree != &bt->NullNode) {
         CHECK(bt->ComparisonFunction(node, node->RightSubtree) < 0);
      }

      /* ====== Is value sum okay? ======================================= */
      CHECK(node->ValueSum == node->LeftSubtree->ValueSum +
                              node->Value +
                              node->RightSubtree->ValueSum);

      /* ====== Is left subtree okay? ==================================== */
      BT_FUNCTION(BinaryTreeInternalVerify)(
         bt, node, node->LeftSubtree, lastBinaryTreeNode,
#ifdef USE_LEAFLINKED
         lastListNode,
#endif
         counter);

#ifdef USE_LEAFLINKED
      /* ====== Is ring list okay? ======================================= */
      CHECK((*lastListNode)->Next != bt->List.Head);
      *lastListNode = (*lastListNode)->Next;
      CHECK(*lastListNode == &node->ListNode);
#endif

#ifdef USE_LEAFLINKED
      /* ====== Is linking working correctly? ============================ */
      prev = BT_FUNCTION(BinaryTreeInternalFindPrev)(bt, node);
      if(prev != &bt->NullNode) {
         CHECK((*lastListNode)->Prev == &prev->ListNode);
      }
      else {
         CHECK((*lastListNode)->Prev == bt->List.Head);
      }

      next = BT_FUNCTION(BinaryTreeInternalFindNext)(bt, node);
      if(next != &bt->NullNode) {
         CHECK((*lastListNode)->Next == &next->ListNode);
      }
      else {
         CHECK((*lastListNode)->Next == bt->List.Head);
      }
#endif

      /* ====== Count elements =========================================== */
      (*counter)++;

      /* ====== Is right subtree okay? =================================== */
      BT_FUNCTION(BinaryTreeInternalVerify)(
         bt, node, node->RightSubtree, lastBinaryTreeNode,
#ifdef USE_LEAFLINKED
         lastListNode,
#endif
         counter);
   }
}


/* ##### Verify structures ############################################### */
void BT_FUNCTION(BinaryTreeVerify)(
        struct BT_DEFINITION(BinaryTree)* bt)
{
   size_t                                counter            = 0;
   struct BT_DEFINITION(BinaryTreeNode)* lastBinaryTreeNode = NULL;
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingListNode*      lastListNode       = &bt->List.Node;
#endif

   CHECK(bt->NullNode.Value == 0);
   CHECK(bt->NullNode.ValueSum == 0);

   BT_FUNCTION(BinaryTreeInternalVerify)(bt, &bt->NullNode, bt->NullNode.LeftSubtree, &lastBinaryTreeNode,
#ifdef USE_LEAFLINKED
      &lastListNode,
#endif
      &counter);
   CHECK(counter == bt->Elements);
}


#ifdef __cplusplus
}
#endif
