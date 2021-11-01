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
 * Copyright (C) 2003-2022 by Thomas Dreibholz
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

#include <stdio.h>
#include <stdlib.h>


#undef TP_DEFINITION
#undef TP_FUNCTION

#ifdef USE_LEAFLINKED
#include "doublelinkedringlist.h"
#define TP_DEFINITION(x) LeafLinked##x
#define TP_FUNCTION(x)   leafLinked##x
#else
#define TP_DEFINITION(x) Simple##x
#define TP_FUNCTION(x)   simple##x
#endif


#if (!defined(LEAFLINKED_TREAP_H) && defined(USE_LEAFLINKED)) || (!defined(REGULAR_TREAP_H) && !defined(USE_LEAFLINKED))
#ifdef USE_LEAFLINKED
#define LEAFLINKED_TREAP_H
#else
#define REGULAR_TREAP_H
#endif

#ifdef __cplusplus
extern "C" {
#endif


#ifndef TREAP_H_CONSTANTS
#define TREAP_H_CONSTANTS
typedef unsigned int       TreapNodePriorityType;
typedef unsigned long long TreapNodeValueType;
#endif


struct TP_DEFINITION(TreapNode)
{
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingListNode  ListNode;
#endif
   struct TP_DEFINITION(TreapNode)* Parent;
   struct TP_DEFINITION(TreapNode)* LeftSubtree;
   struct TP_DEFINITION(TreapNode)* RightSubtree;
   TreapNodePriorityType            Priority;
   TreapNodeValueType               Value;
   TreapNodeValueType               ValueSum;  /* ValueSum := LeftSubtree->Value + Value + RightSubtree->Value */
};

struct TP_DEFINITION(Treap)
{
   struct TP_DEFINITION(TreapNode)* Root;
#ifdef USE_LEAFLINKED
   struct DoubleLinkedRingList      List;
#endif
   size_t                           Elements;
   void                             (*PrintFunction)(const void* node, FILE* fd);
   int                              (*ComparisonFunction)(const void* node1, const void* node2);
};


void TP_FUNCTION(TreapNodeNew)(struct TP_DEFINITION(TreapNode)* node);
void TP_FUNCTION(TreapNodeDelete)(struct TP_DEFINITION(TreapNode)* node);
int TP_FUNCTION(TreapNodeIsLinked)(const struct TP_DEFINITION(TreapNode)* node);


void TP_FUNCTION(TreapNew)(struct TP_DEFINITION(Treap)* treap,
                           void                         (*printFunction)(const void* node, FILE* fd),
                           int                          (*comparisonFunction)(const void* node1, const void* node2));
void TP_FUNCTION(TreapDelete)(struct TP_DEFINITION(Treap)* treap);
void TP_FUNCTION(TreapVerify)(struct TP_DEFINITION(Treap)* treap);
void TP_FUNCTION(TreapPrint)(const struct TP_DEFINITION(Treap)* treap,
                             FILE*                              fd);
int TP_FUNCTION(TreapIsEmpty)(const struct TP_DEFINITION(Treap)* treap);
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetFirst)(
                                    const struct TP_DEFINITION(Treap)* treap);
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetLast)(
                                    const struct TP_DEFINITION(Treap)* treap);
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetPrev)(
                                    const struct TP_DEFINITION(Treap)*     treap,
                                    const struct TP_DEFINITION(TreapNode)* node);
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetNext)(
                                    const struct TP_DEFINITION(Treap)*     treap,
                                    const struct TP_DEFINITION(TreapNode)* node);
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetNearestPrev)(
                                    const struct TP_DEFINITION(Treap)*     treap,
                                    const struct TP_DEFINITION(TreapNode)* cmpNode);
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetNearestNext)(
                                    const struct TP_DEFINITION(Treap)*     treap,
                                    const struct TP_DEFINITION(TreapNode)* cmpNode);
size_t TP_FUNCTION(TreapGetElements)(const struct TP_DEFINITION(Treap)* treap);
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapInsert)(
                                    struct TP_DEFINITION(Treap)*     treap,
                                    struct TP_DEFINITION(TreapNode)* node);
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapRemove)(
                                    struct TP_DEFINITION(Treap)*     treap,
                                    struct TP_DEFINITION(TreapNode)* node);
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapFind)(
                                    const struct TP_DEFINITION(Treap)*     treap,
                                    const struct TP_DEFINITION(TreapNode)* cmpNode);
TreapNodeValueType TP_FUNCTION(TreapGetValueSum)(
                      const struct TP_DEFINITION(Treap)* treap);
struct TP_DEFINITION(TreapNode)* TP_FUNCTION(TreapGetNodeByValue)(
                                    const struct TP_DEFINITION(Treap)* treap,
                                    TreapNodeValueType                 value);


#ifdef __cplusplus
}
#endif

#endif
