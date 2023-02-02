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

#ifndef LINEARLIST_H
#define LINEARLIST_H

#include <stdlib.h>
#include "doublelinkedringlist.h"
#include "debug.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned long long LinearListNodeValueType;


struct LinearListNode
{
   struct DoubleLinkedRingListNode Node;
   LinearListNodeValueType         Value;
};

struct LinearList
{
   struct DoubleLinkedRingList List;
   LinearListNodeValueType     ValueSum;
   size_t                      Elements;
   void                        (*PrintFunction)(const void* node, FILE* fd);
   int                         (*ComparisonFunction)(const void* node1, const void* node2);
};


void linearListNodeNew(struct LinearListNode* node);
void linearListNodeDelete(struct LinearListNode* node);
int linearListNodeIsLinked(const struct LinearListNode* node);


void linearListNew(struct LinearList* ll,
                   void               (*printFunction)(const void* node, FILE* fd),
                   int                (*comparisonFunction)(const void* node1, const void* node2));
void linearListDelete(struct LinearList* ll);
void linearListVerify(struct LinearList* ll);
void linearListPrint(const struct LinearList* ll, FILE* fd);
int linearListIsEmpty(const struct LinearList* ll);
struct LinearListNode* linearListGetFirst(const struct LinearList* ll);
struct LinearListNode* linearListGetLast(const struct LinearList* ll);
struct LinearListNode* linearListGetPrev(const struct LinearList*     ll,
                                         const struct LinearListNode* node);
struct LinearListNode* linearListGetNext(const struct LinearList*     ll,
                                         const struct LinearListNode* node);
struct LinearListNode* linearListGetNearestPrev(
                          const struct LinearList*     ll,
                          const struct LinearListNode* cmpNode);
struct LinearListNode* linearListGetNearestNext(
                          const struct LinearList*     ll,
                          const struct LinearListNode* cmpNode);
size_t linearListGetElements(const struct LinearList* ll);
struct LinearListNode* linearListInsert(struct LinearList*     ll,
                                        struct LinearListNode* newNode);
struct LinearListNode* linearListFind(const struct LinearList*     ll,
                                      const struct LinearListNode* cmpNode);
struct LinearListNode* linearListRemove(struct LinearList*     ll,
                                        struct LinearListNode* node);
LinearListNodeValueType linearListGetValueSum(const struct LinearList* ll);
struct LinearListNode* linearListGetNodeByValue(const struct LinearList* ll,
                                                LinearListNodeValueType  value);


#ifdef __cplusplus
}
#endif

#endif
