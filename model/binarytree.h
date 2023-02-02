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
 * Copyright (C) 2003-2023 by Thomas Dreibholz
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

#include <stdio.h>
#include <stdlib.h>


#undef BT_DEFINITION
#undef BT_FUNCTION

#ifdef USE_LEAFLINKED
#include "doublelinkedringlist.h"
#define BT_DEFINITION(x) LeafLinked##x
#define BT_FUNCTION(x)   leafLinked##x
#else
#define BT_DEFINITION(x) Simple##x
#define BT_FUNCTION(x)   simple##x
#endif


#if (!defined(LEAFLINKED_BINARYTREE_H) && defined(USE_LEAFLINKED)) || (!defined(REGULAR_BINARYTREE_H) && !defined(USE_LEAFLINKED))
#ifdef USE_LEAFLINKED
#define LEAFLINKED_BINARYTREE_H
#else
#define REGULAR_BINARYTREE_H
#endif

#ifdef __cplusplus
extern "C" {
#endif


#ifndef BINARYTREE_H_CONSTANTS
#define BINARYTREE_H_CONSTANTS
typedef unsigned long long BinaryTreeNodeValueType;
#endif


struct BT_DEFINITION(BinaryTreeNode)
{
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingListNode       ListNode;
#endif
   struct BT_DEFINITION(BinaryTreeNode)* Parent;
   struct BT_DEFINITION(BinaryTreeNode)* LeftSubtree;
   struct BT_DEFINITION(BinaryTreeNode)* RightSubtree;
   BinaryTreeNodeValueType               Value;
   BinaryTreeNodeValueType               ValueSum;  /* ValueSum := LeftSubtree->Value + Value + RightSubtree->Value */
};

struct BT_DEFINITION(BinaryTree)
{
   struct BT_DEFINITION(BinaryTreeNode) NullNode;
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingList          List;
#endif
   size_t                               Elements;
   void                                 (*PrintFunction)(const void* node, FILE* fd);
   int                                  (*ComparisonFunction)(const void* node1, const void* node2);
};


void BT_FUNCTION(BinaryTreeNodeNew)(struct BT_DEFINITION(BinaryTreeNode)* node);
void BT_FUNCTION(BinaryTreeNodeDelete)(struct BT_DEFINITION(BinaryTreeNode)* node);
int BT_FUNCTION(BinaryTreeNodeIsLinked)(const struct BT_DEFINITION(BinaryTreeNode)* node);


void BT_FUNCTION(BinaryTreeNew)(struct BT_DEFINITION(BinaryTree)* bt,
                                void                              (*printFunction)(const void* node, FILE* fd),
                                int                               (*comparisonFunction)(const void* node1, const void* node2));
void BT_FUNCTION(BinaryTreeDelete)(struct BT_DEFINITION(BinaryTree)* bt);
void BT_FUNCTION(BinaryTreeVerify)(struct BT_DEFINITION(BinaryTree)* bt);
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeInternalGetNearestPrev)(
                                         struct BT_DEFINITION(BinaryTree)*      bt,
                                         struct BT_DEFINITION(BinaryTreeNode)** root,
                                         struct BT_DEFINITION(BinaryTreeNode)*  parent,
                                         struct BT_DEFINITION(BinaryTreeNode)*  node);
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeInternalGetNearestNext)(
                                         struct BT_DEFINITION(BinaryTree)*      bt,
                                         struct BT_DEFINITION(BinaryTreeNode)** root,
                                         struct BT_DEFINITION(BinaryTreeNode)*  parent,
                                         struct BT_DEFINITION(BinaryTreeNode)*  node);
void BT_FUNCTION(BinaryTreePrint)(const struct BT_DEFINITION(BinaryTree)* bt,
                                  FILE*                                   fd);
int BT_FUNCTION(BinaryTreeIsEmpty)(const struct BT_DEFINITION(BinaryTree)* bt);
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetFirst)(
                                         const struct BT_DEFINITION(BinaryTree)* bt);
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetLast)(
                                         const struct BT_DEFINITION(BinaryTree)* bt);
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetPrev)(
                                         const struct BT_DEFINITION(BinaryTree)*     bt,
                                         const struct BT_DEFINITION(BinaryTreeNode)* node);
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetNext)(
                                         const struct BT_DEFINITION(BinaryTree)*     bt,
                                         const struct BT_DEFINITION(BinaryTreeNode)* node);
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetNearestPrev)(
                                         const struct BT_DEFINITION(BinaryTree)*     bt,
                                         const struct BT_DEFINITION(BinaryTreeNode)* cmpNode);
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetNearestNext)(
                                         const struct BT_DEFINITION(BinaryTree)*     bt,
                                         const struct BT_DEFINITION(BinaryTreeNode)* cmpNode);
size_t BT_FUNCTION(BinaryTreeGetElements)(const struct BT_DEFINITION(BinaryTree)* bt);
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeInsert)(
                                         struct BT_DEFINITION(BinaryTree)*     bt,
                                         struct BT_DEFINITION(BinaryTreeNode)* node);
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeRemove)(
                                         struct BT_DEFINITION(BinaryTree)*     bt,
                                         struct BT_DEFINITION(BinaryTreeNode)* node);
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeFind)(
                                         const struct BT_DEFINITION(BinaryTree)*     bt,
                                         const struct BT_DEFINITION(BinaryTreeNode)* cmpNode);
BinaryTreeNodeValueType BT_FUNCTION(BinaryTreeGetValueSum)(
                                         const struct BT_DEFINITION(BinaryTree)* bt);
struct BT_DEFINITION(BinaryTreeNode)* BT_FUNCTION(BinaryTreeGetNodeByValue)(
                                         struct BT_DEFINITION(BinaryTree)* bt,
                                         BinaryTreeNodeValueType           value);


#ifdef __cplusplus
}
#endif

#endif
