/* --------------------------------------------------------------------------
 *
 *              //===//   //=====   //===//   //=====  //   //      //
 *             //    //  //        //    //  //       //   //=/  /=//
 *            //===//   //=====   //===//   //====   //   //  //  //
 *           //   \\         //  //             //  //   //  //  //
 *          //     \\  =====//  //        =====//  //   //      //  Version V
 *
 * ------------- An Open Source RSerPool Simulation for OMNeT++ -------------
 *
 * Copyright (C) 2003-2026 by Thomas Dreibholz
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
 * Contact: thomas.dreibholz@gmail.com
 */

#include <stdlib.h>
#include <stdio.h>

#include "treap.h"
#include "randomizer.h"
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif


static struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapInternalFindPrev)(
                                           const struct TP_DEFINITION(Treap)*     treap,
                                           const struct TP_DEFINITION(TreapNode)* cmpNode);
static struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapInternalFindNext)(
                                           const struct TP_DEFINITION(Treap)*     treap,
                                           const struct TP_DEFINITION(TreapNode)* cmpNode);


/* ###### Initialize ##################################################### */
void TP_FUNCTION(TreapNodeNew)(
        struct TP_DEFINITION(TreapNode)* node)
{
#ifdef USE_LEAFLINKED
   doubleLinkedRingListNodeNew(&node->ListNode);
#endif
   node->Parent       = NULL;
   node->LeftSubtree  = NULL;
   node->RightSubtree = NULL;
   node->Priority     = 0;
   node->Value        = 0;
   node->ValueSum     = 0;
}


/* ###### Invalidate ##################################################### */
void TP_FUNCTION(TreapNodeDelete)(
        struct TP_DEFINITION(TreapNode)* node)
{
   node->Parent       = NULL;
   node->LeftSubtree  = NULL;
   node->RightSubtree = NULL;
   node->Priority     = 0;
   node->Value        = 0;
   node->ValueSum     = 0;
#ifdef USE_LEAFLINKED
   doubleLinkedRingListNodeDelete(&node->ListNode);
#endif
}


/* ###### Is node linked? ################################################ */
int TP_FUNCTION(TreapNodeIsLinked)(
       const struct TP_DEFINITION(TreapNode)* node)
{
   /* A treap with a single node has no parent, left and right subtree
      -> We simply set priority > 0 when the node is linked! */
   return(node->Priority != 0);
}


/* ##### Initialize ###################################################### */
void TP_FUNCTION(TreapNew)(
        struct TP_DEFINITION(Treap)* treap,
        void                         (*printFunction)(const void* node, FILE* fd),
        int                          (*comparisonFunction)(const void* node1, const void* node2))
{
#ifdef USE_LEAFLINKED
   doubleLinkedRingListNew(&treap->List);
#endif
   treap->PrintFunction      = printFunction;
   treap->ComparisonFunction = comparisonFunction;
   treap->Root               = NULL;
   treap->Elements           = 0;
}


/* ##### Invalidate ###################################################### */
void TP_FUNCTION(TreapDelete)(
        struct TP_DEFINITION(Treap)* treap)
{
   treap->Root     = NULL;
   treap->Elements = 0;
#ifdef USE_LEAFLINKED
   doubleLinkedRingListDelete(&treap->List);
#endif
}


/* ###### Internal method for printing a node ############################# */
static void TP_FUNCTION(TreapPrintNode)(
               const struct TP_DEFINITION(Treap)*     treap,
               const struct TP_DEFINITION(TreapNode)* node,
               FILE*                                  fd)
{
   treap->PrintFunction(node, fd);
#ifdef DEBUG
   fprintf(fd, " ptr=%p pri=%u v=%llu vsum=%llu",
           node, node->Priority,
           node->Value, node->ValueSum);
   if(node->LeftSubtree != NULL) {
      fprintf(fd, " l=%p[", node->LeftSubtree);
      treap->PrintFunction(node->LeftSubtree, fd);
      fprintf(fd, "]");
   }
   else {
      fprintf(fd, " l=()");
   }
   if(node->RightSubtree != NULL) {
      fprintf(fd, " r=%p[", node->RightSubtree);
      treap->PrintFunction(node->RightSubtree, fd);
      fprintf(fd, "]");
   }
   else {
      fprintf(fd, " r=()");
   }
   if(node->Parent != NULL) {
      fprintf(fd, " p=%p[", node->Parent);
      treap->PrintFunction(node->Parent, fd);
      fprintf(fd, "]   ");
   }
   else {
      fprintf(fd, " p=())   ");
   }
   fputs("\n", fd);
#endif
}


/* ##### Internal printing function ###################################### */
static void TP_FUNCTION(TreapInternalPrint)(
               const struct TP_DEFINITION(Treap)*     treap,
               const struct TP_DEFINITION(TreapNode)* node,
               FILE*                                  fd)
{
   if(node != NULL) {
      TP_FUNCTION(TreapInternalPrint)(treap, node->LeftSubtree, fd);
      TP_FUNCTION(TreapPrintNode)(treap, node, fd);
      TP_FUNCTION(TreapInternalPrint)(treap, node->RightSubtree, fd);
   }
}


/* ###### Print treap ##################################################### */
void TP_FUNCTION(TreapPrint)(
        const struct TP_DEFINITION(Treap)* treap,
        FILE*                              fd)
{
#ifdef DEBUG
   fprintf(fd, "\n\nroot=%p[", treap->Root);
   if(treap->Root != NULL) {
      treap->PrintFunction(treap->Root, fd);
   }
   fputs("]\n", fd);
#endif
   TP_FUNCTION(TreapInternalPrint)(treap, treap->Root, fd);
   fputs("\n", fd);
}


/* ###### Is treap empty? ################################################ */
int TP_FUNCTION(TreapIsEmpty)(
       const struct TP_DEFINITION(Treap)* treap)
{
   return(treap->Root == NULL);
}


/* ###### Get first node ################################################## */
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetFirst)(
                                    const struct TP_DEFINITION(Treap)* treap)
{
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingListNode* node = treap->List.Node.Next;
   if(node != treap->List.Head) {
      return((struct TP_DEFINITION(TreapNode)*)node);
   }
   return(NULL);
#else
   const struct TP_DEFINITION(TreapNode)* node = treap->Root;
   if(node) {
      while(node->LeftSubtree != NULL) {
         node = node->LeftSubtree;
      }
   }
   return((struct TP_DEFINITION(TreapNode)*)node);
#endif
}


/* ###### Get last node ################################################### */
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetLast)(
                                    const struct TP_DEFINITION(Treap)* treap)
{
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingListNode* node = treap->List.Node.Prev;
   if(node != treap->List.Head) {
      return((struct TP_DEFINITION(TreapNode)*)node);
   }
   return(NULL);
#else
   const struct TP_DEFINITION(TreapNode)* node = treap->Root;
   if(node) {
      while(node->RightSubtree != NULL) {
         node = node->RightSubtree;
      }
   }
   return((struct TP_DEFINITION(TreapNode)*)node);
#endif
}


/* ###### Get previous node ############################################### */
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetPrev)(
                                    const struct TP_DEFINITION(Treap)*     treap,
                                    const struct TP_DEFINITION(TreapNode)* node)
{
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingListNode* prev = node->ListNode.Prev;
   if(prev != treap->List.Head) {
      return((struct TP_DEFINITION(TreapNode)*)prev);
   }
   return(NULL);
#else
   return(TP_FUNCTION(TreapInternalFindPrev)(treap, node));
#endif
}


/* ###### Get next node ################################################## */
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetNext)(
                                    const struct TP_DEFINITION(Treap)*     treap,
                                    const struct TP_DEFINITION(TreapNode)* node)
{
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingListNode* next = node->ListNode.Next;
   if(next != treap->List.Head) {
      return((struct TP_DEFINITION(TreapNode)*)next);
   }
   return(NULL);
#else
   return(TP_FUNCTION(TreapInternalFindNext)(treap, node));
#endif
}


/* ###### Find nearest previous node ##################################### */
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetNearestPrev)(
                                    const struct TP_DEFINITION(Treap)*     treap,
                                    const struct TP_DEFINITION(TreapNode)* cmpNode)
{
   struct TP_DEFINITION(TreapNode)*const* nodePtr;
   struct TP_DEFINITION(TreapNode)*const* parentPtr;
   const struct TP_DEFINITION(TreapNode)* node;
   const struct TP_DEFINITION(TreapNode)* parent;
   int                                    cmpResult = 0;

#ifdef DEBUG
   printf("nearest prev: ");
   treap->PrintFunction(cmpNode, stdout);
   printf("\n");
   TP_FUNCTION(TreapPrint)(treap, stdout);
#endif

   parentPtr = NULL;
   nodePtr   = &treap->Root;
   while(*nodePtr != NULL) {
      cmpResult = treap->ComparisonFunction(cmpNode, *nodePtr);
      if(cmpResult < 0) {
         parentPtr = nodePtr;
         nodePtr   = &(*nodePtr)->LeftSubtree;
      }
      else if(cmpResult > 0) {
         parentPtr = nodePtr;
         nodePtr   = &(*nodePtr)->RightSubtree;
      }
      if(cmpResult == 0) {
         return(TP_FUNCTION(TreapGetPrev)(treap, *nodePtr));
      }
   }

   if(parentPtr == NULL) {
      if(cmpResult > 0) {
         return(treap->Root);
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
         if(parent->LeftSubtree != NULL) {
            node = parent->LeftSubtree;
            while(node->RightSubtree != NULL) {
               node = node->RightSubtree;
            }
            return((struct TP_DEFINITION(TreapNode)*)node);
         }

         /* If there is no left subtree, the nearest previous node is an
            ancestor node which has the node on its right side. */
         else {
            node   = parent;
            parent = node->Parent;
            while((parent != NULL) && (node == parent->LeftSubtree)) {
               node   = parent;
               parent = parent->Parent;
            }
            return((struct TP_DEFINITION(TreapNode)*)parent);
         }
      }
   }
}


/* ###### Find nearest next node ######################################### */
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetNearestNext)(
                                    const struct TP_DEFINITION(Treap)*     treap,
                                    const struct TP_DEFINITION(TreapNode)* cmpNode)
{
   struct TP_DEFINITION(TreapNode)*const* nodePtr;
   struct TP_DEFINITION(TreapNode)*const* parentPtr;
   const struct TP_DEFINITION(TreapNode)* node;
   const struct TP_DEFINITION(TreapNode)* parent;
   int                                    cmpResult = 0;

#ifdef DEBUG
   printf("nearest next: ");
   treap->PrintFunction(cmpNode, stdout);
   printf("\n");
   TP_FUNCTION(TreapPrint)(treap, stdout);
#endif

   parentPtr = NULL;
   nodePtr   = &treap->Root;
   while(*nodePtr != NULL) {
      cmpResult = treap->ComparisonFunction(cmpNode, *nodePtr);
      if(cmpResult < 0) {
         parentPtr = nodePtr;
         nodePtr   = &(*nodePtr)->LeftSubtree;
      }
      else if(cmpResult > 0) {
         parentPtr = nodePtr;
         nodePtr   = &(*nodePtr)->RightSubtree;
      }
      if(cmpResult == 0) {
         return(TP_FUNCTION(TreapGetNext)(treap, *nodePtr));
      }
   }

   if(parentPtr == NULL) {
      if(cmpResult < 0) {
         return(treap->Root);
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
         if(parent->RightSubtree != NULL) {
            node = parent->RightSubtree;
            while(node->LeftSubtree != NULL) {
               node = node->LeftSubtree;
            }
            return((struct TP_DEFINITION(TreapNode)*)node);
         }

         /* If there is no right subtree, the nearest next node is an
            ancestor node which has the node on its left side. */
         else {
            node   = parent;
            parent = node->Parent;
            while((parent != NULL) && (node == parent->RightSubtree)) {
               node   = parent;
               parent = parent->Parent;
            }
            return((struct TP_DEFINITION(TreapNode)*)parent);
         }
      }
   }
}


/* ###### Get number of elements ########################################## */
size_t TP_FUNCTION(TreapGetElements)(
          const struct TP_DEFINITION(Treap)* treap)
{
   return(treap->Elements);
}


/* ###### Get prev node by walking through the tree (does *not* use list!) */
static struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapInternalFindPrev)(
                                           const struct TP_DEFINITION(Treap)*     treap,
                                           const struct TP_DEFINITION(TreapNode)* cmpNode)
{
   const struct TP_DEFINITION(TreapNode)* node = cmpNode->LeftSubtree;
   const struct TP_DEFINITION(TreapNode)* parent;

   if(node != NULL) {
      while(node->RightSubtree != NULL) {
         node = node->RightSubtree;
      }
      return((struct TP_DEFINITION(TreapNode)*)node);
   }
   else {
      node   = cmpNode;
      parent = cmpNode->Parent;
      while((parent != NULL) && (node == parent->LeftSubtree)) {
         node   = parent;
         parent = parent->Parent;
      }
      return((struct TP_DEFINITION(TreapNode)*)parent);
   }
}


/* ###### Get next node by walking through the tree (does *not* use list!) */
static struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapInternalFindNext)(
                                           const struct TP_DEFINITION(Treap)*     treap,
                                           const struct TP_DEFINITION(TreapNode)* cmpNode)
{
   const struct TP_DEFINITION(TreapNode)* node = cmpNode->RightSubtree;
   const struct TP_DEFINITION(TreapNode)* parent;

   if(node != NULL) {
      while(node->LeftSubtree != NULL) {
         node = node->LeftSubtree;
      }
      return((struct TP_DEFINITION(TreapNode)*)node);
   }
   else {
      node   = cmpNode;
      parent = cmpNode->Parent;
      while((parent != NULL) && (node == parent->RightSubtree)) {
         node   = parent;
         parent = parent->Parent;
      }
      return((struct TP_DEFINITION(TreapNode)*)parent);
   }
}


/* ###### Find node ####################################################### */
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapFind)(
                                    const struct TP_DEFINITION(Treap)*     treap,
                                    const struct TP_DEFINITION(TreapNode)* cmpNode)
{
#ifdef DEBUG
   printf("find: ");
   treap->PrintFunction(cmpNode, stdout);
   printf("\n");
#endif

   struct TP_DEFINITION(TreapNode)* node = treap->Root;
   while(node != NULL) {
      const int cmpResult = treap->ComparisonFunction(cmpNode, node);
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
TreapNodeValueType TP_FUNCTION(TreapGetValueSum)(
                      const struct TP_DEFINITION(Treap)* treap)
{
   if(treap->Root) {
      return(treap->Root->ValueSum);
   }
   return(0);
}


/* ##### Update value sum ################################################ */
inline static void TP_FUNCTION(TreapUpdateValueSum)(
                      struct TP_DEFINITION(Treap)*     treap,
                      struct TP_DEFINITION(TreapNode)* node)
{
   node->ValueSum = ((node->LeftSubtree != NULL) ? node->LeftSubtree->ValueSum : 0) +
                       node->Value +
                       ((node->RightSubtree != NULL) ? node->RightSubtree->ValueSum : 0);
}


/* ##### Rotation with left subtree ###################################### */
static void TP_FUNCTION(TreapRotateLeft)(
               struct TP_DEFINITION(Treap)*     treap,
               struct TP_DEFINITION(TreapNode)* node)
{
   struct TP_DEFINITION(TreapNode)* rightSubtree;
   struct TP_DEFINITION(TreapNode)* parent;

   CHECK(node != NULL);
   CHECK(node->RightSubtree != NULL);

   rightSubtree = node->RightSubtree;
   node->RightSubtree = rightSubtree->LeftSubtree;
   if(rightSubtree->LeftSubtree) {
      rightSubtree->LeftSubtree->Parent = node;
   }
   parent               = node->Parent;
   rightSubtree->Parent = parent;

   if(parent) {
      if(parent->LeftSubtree == node) {
         parent->LeftSubtree = rightSubtree;
      }
      else {
         parent->RightSubtree = rightSubtree;
      }
   }
   else {
      treap->Root = rightSubtree;
   }

   rightSubtree->LeftSubtree = node;
   node->Parent              = rightSubtree;

   TP_FUNCTION(TreapUpdateValueSum)(treap, node);
   TP_FUNCTION(TreapUpdateValueSum)(treap, node->Parent);
}


/* ##### Rotation with ripht subtree ##################################### */
static void TP_FUNCTION(TreapRotateRight)(
               struct TP_DEFINITION(Treap)*     treap,
               struct TP_DEFINITION(TreapNode)* node)
{
   struct TP_DEFINITION(TreapNode)* leftSubtree;
   struct TP_DEFINITION(TreapNode)* parent;

   CHECK(node != NULL);
   CHECK(node->LeftSubtree != NULL);

   leftSubtree = node->LeftSubtree;
   node->LeftSubtree = leftSubtree->RightSubtree;
   if(leftSubtree->RightSubtree) {
      leftSubtree->RightSubtree->Parent = node;
   }
   parent              = node->Parent;
   leftSubtree->Parent = parent;

   if(parent) {
      if(parent->LeftSubtree == node) {
         parent->LeftSubtree = leftSubtree;
      }
      else {
         parent->RightSubtree = leftSubtree;
      }
   }
   else {
      treap->Root = leftSubtree;
   }

   leftSubtree->RightSubtree = node;
   node->Parent              = leftSubtree;

   TP_FUNCTION(TreapUpdateValueSum)(treap, node);
   TP_FUNCTION(TreapUpdateValueSum)(treap, node->Parent);
}


/* ###### Insert ######################################################### */
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapInsert)(
                                    struct TP_DEFINITION(Treap)*     treap,
                                    struct TP_DEFINITION(TreapNode)* node)
{
   struct TP_DEFINITION(TreapNode)* next;
   struct TP_DEFINITION(TreapNode)* parent;
#ifdef USE_LEAFLINKED
   struct TP_DEFINITION(TreapNode)* prev;
#endif
   int                              cmpResult = -1;

#ifdef DEBUG
   printf("insert: ");
   treap->PrintFunction(node, stdout);
   printf("\n");
#endif


   parent = NULL;
   next   = treap->Root;
   while(next != NULL) {
      cmpResult = treap->ComparisonFunction(node, next);
      if(cmpResult < 0) {
         parent = next;
         next   = next->LeftSubtree;
      }
      else if(cmpResult > 0) {
         parent = next;
         next   = next->RightSubtree;
      }
      else {
         return(next);
      }
   }

   /* ====== Ensure treap properties ===================================== */
   node->Parent       = parent;
   node->LeftSubtree  = NULL;
   node->RightSubtree = NULL;
   node->ValueSum     = node->Value;
   node->Priority     = 1 + (random32() % 0xffffffff);
   /* Note: Priority may never be 0 -> necessary for TP_FUNCTION(IsLinked)! */

   if(parent) {   /* New node has a parent (i.e. it is not root). */
      if(cmpResult < 0) {   /* Link node from parent. */
         parent->LeftSubtree = node;
      }
      else {
         parent->RightSubtree = node;
      }

      do {
        TP_FUNCTION(TreapUpdateValueSum)(treap, parent);
        if(parent->Priority <= node->Priority) {
            break;
         }
         if(parent->LeftSubtree == node) {
            TP_FUNCTION(TreapRotateRight)(treap, parent);
         }
         else {
            TP_FUNCTION(TreapRotateLeft)(treap, parent);
         }
         parent = node->Parent;
      } while(parent);
      while(parent != NULL) {
         /* No more rotations, but updating value sum is still necessary. */
         TP_FUNCTION(TreapUpdateValueSum)(treap, parent);
         parent = parent->Parent;
      }
   }
   else {   /* New node becomes root. */
      CHECK(treap->Elements == 0);
      treap->Root = node;
   }

   /* ====== Link node to list =========================================== */
#ifdef USE_LEAFLINKED
   prev = TP_FUNCTION(TreapInternalFindPrev)(treap, node);
   if(prev != NULL) {
      doubleLinkedRingListAddAfter(&prev->ListNode, &node->ListNode);
   }
   else {
      doubleLinkedRingListAddHead(&treap->List, &node->ListNode);
   }
#endif
   treap->Elements++;


#ifdef DEBUG
   TP_FUNCTION(TreapPrint)(treap, stdout);
#endif
#ifdef VERIFY
   TP_FUNCTION(TreapVerify)(treap);
#endif
   return(node);
}


/* ###### Remove ######################################################### */
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapRemove)(
                                    struct TP_DEFINITION(Treap)*     treap,
                                    struct TP_DEFINITION(TreapNode)* node)
{
   struct TP_DEFINITION(TreapNode)* parent;
   struct TP_DEFINITION(TreapNode)* out;

#ifdef DEBUG
   printf("remove: ");
   treap->PrintFunction(node, stdout);
   printf("\n");
#endif

   /* ====== Remove node from treap ====================================== */
   while((node->LeftSubtree != NULL) && (node->RightSubtree != NULL)) {
      if(node->LeftSubtree->Priority < node->RightSubtree->Priority) {
         TP_FUNCTION(TreapRotateRight)(treap, node);
      }
      else {
         TP_FUNCTION(TreapRotateLeft)(treap, node);
      }
   }
   parent = node->Parent;
   out = (node->LeftSubtree != NULL) ? node->LeftSubtree : node->RightSubtree;
   if(out) {
      out->Parent = parent;
   }
   if(parent) {   /* Update link in parent. */
      if(parent->LeftSubtree == node) {
         parent->LeftSubtree = out;
      }
      else {
         parent->RightSubtree = out;
      }
   }
   else {   /* Removed node was root. */
      treap->Root = out;
   }

   while(parent != NULL) {   /* Update value sum. */
      TP_FUNCTION(TreapUpdateValueSum)(treap, parent);
      parent = parent->Parent;
   }


   /* ====== Unlink node from list and invalidate pointers =============== */
   node->Priority     = 0;   /* Invalidate priority -> for TP_FUNCTION(IsLinked)! */
   node->Parent       = NULL;
   node->RightSubtree = NULL;
   node->LeftSubtree  = NULL;
#ifdef USE_LEAFLINKED
   doubleLinkedRingListRemNode(&node->ListNode);
   node->ListNode.Prev = NULL;
   node->ListNode.Next = NULL;
#endif
   CHECK(treap->Elements > 0);
   treap->Elements--;

#ifdef DEBUG
printf("=> output-rem: t=%p   %p\n",treap,treap->Root);
    TP_FUNCTION(TreapPrint)(treap, stdout);
#endif
#ifdef VERIFY
    TP_FUNCTION(TreapVerify)(treap);
#endif
   return(node);
}


/* ##### Get node by value ############################################### */
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetNodeByValue)(
                                    const struct TP_DEFINITION(Treap)* treap,
                                    TreapNodeValueType                 value)
{
   struct TP_DEFINITION(TreapNode)* node = treap->Root;
   for(;;) {
      if(value < ((node->LeftSubtree != NULL) ? node->LeftSubtree->ValueSum : 0)) {
         if(node->LeftSubtree != NULL) {
            node = node->LeftSubtree;
         }
         else {
            break;
         }
      }
      else if((value < ((node->LeftSubtree != NULL) ? node->LeftSubtree->ValueSum : 0) + node->Value)) {
         break;
      }
      else {
         if(node->RightSubtree != NULL) {
            value -= ((node->LeftSubtree != NULL) ? node->LeftSubtree->ValueSum : 0) + node->Value;
            node = node->RightSubtree;
         }
         else {
            break;
         }
      }
   }

   return(node);
}


/* ##### Internal verification function ################################## */
static void TP_FUNCTION(TreapInternalVerify)(
               struct TP_DEFINITION(Treap)*      treap,
               struct TP_DEFINITION(TreapNode)*  parent,
               struct TP_DEFINITION(TreapNode)*  node,
               struct TP_DEFINITION(TreapNode)** lastTreapNode,
#ifdef USE_LEAFLINKED
               struct DoubleLinkedRingListNode** lastListNode,
#endif
               size_t*                           counter)
{
#ifdef USE_LEAFLINKED
   struct TP_DEFINITION(TreapNode)* prev;
   struct TP_DEFINITION(TreapNode)* next;
#endif

   if(node != NULL) {
      /* ====== Print node =============================================== */
#ifdef DEBUG
      printf("verifying ");
      TP_FUNCTION(TreapPrintNode)(treap, node, stdout);
      puts("");
#endif

      /* ====== Correct parent? ========================================== */
      CHECK(node->Parent == parent);

      /* ====== Correct tree and heap properties? ======================== */
      if(node->LeftSubtree != NULL) {
         CHECK(treap->ComparisonFunction(node, node->LeftSubtree) > 0);
         CHECK(node->Priority <= node->LeftSubtree->Priority);
      }
      if(node->RightSubtree != NULL) {
         CHECK(treap->ComparisonFunction(node, node->RightSubtree) < 0);
         CHECK(node->Priority <= node->RightSubtree->Priority);
      }

      /* ====== Is value sum okay? ======================================= */
      CHECK(node->ValueSum == ((node->LeftSubtree != NULL) ? node->LeftSubtree->ValueSum : 0) +
                                 node->Value +
                                 ((node->RightSubtree != NULL) ? node->RightSubtree->ValueSum : 0));

      /* ====== Is left subtree okay? ==================================== */
      TP_FUNCTION(TreapInternalVerify)(treap, node, node->LeftSubtree, lastTreapNode,
#ifdef USE_LEAFLINKED
                                       lastListNode,
#endif
                                       counter);

#ifdef USE_LEAFLINKED
      /* ====== Is ring list okay? ======================================= */
      CHECK((*lastListNode)->Next != treap->List.Head);
      *lastListNode = (*lastListNode)->Next;
      CHECK(*lastListNode == &node->ListNode);

      /* ====== Is linking working correctly? ============================ */
      prev = TP_FUNCTION(TreapInternalFindPrev)(treap, node);
      if(prev != NULL) {
         CHECK((*lastListNode)->Prev == &prev->ListNode);
      }
      else {
         CHECK((*lastListNode)->Prev == treap->List.Head);
      }

      next = TP_FUNCTION(TreapInternalFindNext)(treap, node);
      if(next != NULL) {
         CHECK((*lastListNode)->Next == &next->ListNode);
      }
      else {
         CHECK((*lastListNode)->Next == treap->List.Head);
      }
#endif

      /* ====== Count elements =========================================== */
      (*counter)++;

      /* ====== Is right subtree okay? =================================== */
      TP_FUNCTION(TreapInternalVerify)(treap, node, node->RightSubtree, lastTreapNode,
#ifdef USE_LEAFLINKED
                                       lastListNode,
#endif
                                       counter);
   }
}


/* ##### Verify structures ############################################### */
void TP_FUNCTION(TreapVerify)(
        struct TP_DEFINITION(Treap)* treap)
{
   size_t                           counter       = 0;
   struct TP_DEFINITION(TreapNode)* lastTreapNode = NULL;
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingListNode* lastListNode  = &treap->List.Node;
#endif

   TP_FUNCTION(TreapInternalVerify)(treap, NULL, treap->Root, &lastTreapNode,
#ifdef USE_LEAFLINKED
                                    &lastListNode,
#endif
                                    &counter);
   CHECK(counter == treap->Elements);
}


#ifdef __cplusplus
}
#endif
