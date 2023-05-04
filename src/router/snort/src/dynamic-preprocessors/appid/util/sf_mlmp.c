/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/



#include "stdio.h"
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include "string.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_mlmp.h"
#include "string.h"

#define _MLMP_DEBUG 0

typedef struct _tPatternNode {
    tMlmpPattern     pattern;
    void         *userData;     /*client/service info */

    /**part number. Should start from 1. Ordering of parts does not matter in the sense
     * part 1 may appear after part 2 in payload.*/
    uint32_t      partNum;

    /**Total number of parts.*/
    uint32_t      partTotal;

    /**Uniq non-zero identifier to tie parts of a multi-part patterns together. */
    uint32_t      patternId;

    struct _tPatternNode  *nextPattern;

} tPatternNode ;

typedef struct _tPatternPrimaryNode {
    tPatternNode  patternNode;

    struct _tPatternPrimaryNode *nextPrimaryNode;

    /*Tree node for next level. Present only in primary pattern node i.e.  */
    struct tMlmpTree  *nextLevelMatcher;

} tPatternPrimaryNode ;

/*Node for mlmp tree */
typedef struct tMlmpTree {
    void         *patternTree;
    tPatternPrimaryNode *patternList;
    uint32_t  level;

} tTreeNode ;

/*Used to track matched patterns. */
typedef struct _tMatchedPatternList {
    tPatternNode *patternNode;
    size_t       index;
    /*uint32_t level; */
    struct _tMatchedPatternList *next;
} tMatchedPatternList;

static int compareMlmpPatterns(const void *p1, const void *p2);
static int createTreesRecusively(struct tMlmpTree *root);
static void destroyTreesRecursively(struct tMlmpTree *root);
static void dumpTreesRecursively(struct tMlmpTree *root);
static int addPatternRecursively(struct tMlmpTree *root, const tMlmpPattern *inputPatternList, void *metaData, uint32_t level);
static tPatternNode* urlPatternSelector (const tMatchedPatternList *matchList, const uint8_t* payload);
static tPatternNode* genericPatternSelector (const tMatchedPatternList *matchList, const uint8_t* payload);
static void *mlmpMatchPatternCustom(struct tMlmpTree *root, tMlmpPattern *inputPatternList, tPatternNode* (*callback)(const tMatchedPatternList *, const uint8_t*));
static int patternMatcherCallback (void *id, void *unused_tree, int index, void *data, void *unused_neg);

static uint32_t gPatternId = 1;

struct tMlmpTree* mlmpCreate(void)
{
    tTreeNode *root = calloc(1, sizeof(tTreeNode));
    if (root) root->level = 0;
    return root;
}

/*last pattern should be NULL */
int mlmpAddPattern(struct tMlmpTree *root, const tMlmpPattern *inputPatternList, void *metaData)
{
    return addPatternRecursively(root, inputPatternList, metaData, 0);
}

int mlmpProcessPatterns(struct tMlmpTree *root)
{
    int rvalue;

   rvalue = createTreesRecusively(root);
   if (rvalue) destroyTreesRecursively(root);
   return rvalue;
}

void *mlmpMatchPatternUrl(struct tMlmpTree *root, tMlmpPattern *inputPatternList)
{
    return mlmpMatchPatternCustom(root, inputPatternList, urlPatternSelector);
}

void *mlmpMatchPatternGeneric(struct tMlmpTree *root, tMlmpPattern *inputPatternList)
{
    return mlmpMatchPatternCustom(root, inputPatternList, genericPatternSelector);
}

static inline int matchDomainPattern(const tMatchedPatternList *mp, const uint8_t *pattern)
{
    if (!pattern)
        return -1;

    return (mp->patternNode->pattern.level == 0 && !(mp->index == 0 || pattern[mp->index-1] == '.'));
}

static void *mlmpMatchPatternCustom(struct tMlmpTree *rootNode, tMlmpPattern *inputPatternList, tPatternNode* (*callback)(const tMatchedPatternList *, const uint8_t*))
{
    tMatchedPatternList *mp = NULL;
    tMatchedPatternList *tmpMp;
    void *data = NULL;
    void *tmpData = NULL;
    tPatternPrimaryNode *primaryNode;
    tMlmpPattern *pattern = inputPatternList;

    if (!rootNode || !pattern || !pattern->pattern)
        return NULL;

    _dpd.searchAPI->search_instance_find_all(rootNode->patternTree,
            (char *)pattern->pattern,
            pattern->patternSize, 0,
            patternMatcherCallback, (void*)&mp);

    primaryNode = (tPatternPrimaryNode *)callback(mp, pattern->pattern);

    while (mp)
    {
        tmpMp = mp;
        mp = mp->next;
        free(tmpMp);
    }

    if(primaryNode)
    {
        data = primaryNode->patternNode.userData;
        tmpData = mlmpMatchPatternCustom(primaryNode->nextLevelMatcher, ++inputPatternList, callback);
        if (tmpData)
            data = tmpData;
    }

    return data;
}

void mlmpDestroy(struct tMlmpTree *root)
{
    destroyTreesRecursively(root);
}

void mlmpDump(struct tMlmpTree *root)
{
    dumpTreesRecursively(root);
}

/**tMlmpPattern comparator: compares patterns based on pattern, patternSize. This will
 * result in alphabatical order. Notice that patternId is ignored here.
 */
static int compareMlmpPatterns(const void *p1, const void *p2)
{
    tMlmpPattern *pat1 = (tMlmpPattern *)p1;
    tMlmpPattern *pat2 = (tMlmpPattern *)p2;
    int rValue;
    size_t minSize;

    /*first compare patterns by the smaller pattern size, if same then size wins */
    minSize = (pat1->patternSize > pat2->patternSize)? pat2->patternSize: pat1->patternSize;

    rValue = memcmp(pat1->pattern, pat2->pattern, minSize);
    if (rValue) return rValue;

    return ((int)pat1->patternSize - (int)pat2->patternSize);
}

/*pattern trees are not freed on error because in case of error, caller should call detroyTreesRecursively. */
static int createTreesRecusively(struct tMlmpTree *rootNode)
{
    void *patternMatcher;
    tPatternPrimaryNode *primaryPatternNode;
    tPatternNode *ddPatternNode;

    /* set up the MPSE for url patterns */
    if (!(patternMatcher = rootNode->patternTree = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF)))
        return -1;

    for (primaryPatternNode = rootNode->patternList;
            primaryPatternNode;
            primaryPatternNode = primaryPatternNode->nextPrimaryNode)
    {
        /*recursion into next lower level */
        if (primaryPatternNode->nextLevelMatcher)
        {
            if (createTreesRecusively(primaryPatternNode->nextLevelMatcher)) return -1;
        }

        for (ddPatternNode = &primaryPatternNode->patternNode;
                ddPatternNode;
                ddPatternNode = ddPatternNode->nextPattern)
        {
            _dpd.searchAPI->search_instance_add_ex(patternMatcher,
                    (void *)ddPatternNode->pattern.pattern,
                    ddPatternNode->pattern.patternSize,
                    ddPatternNode,
                    STR_SEARCH_CASE_INSENSITIVE);
        }
    }

    _dpd.searchAPI->search_instance_prep(patternMatcher);

    return 0;
}

static void destroyTreesRecursively(struct tMlmpTree *rootNode)
{
    tPatternPrimaryNode *primaryPatternNode;
    uint32_t partNum;

    if (!rootNode)
        return;

    while ((primaryPatternNode = rootNode->patternList))
    {
        /*recursion into next lower level */
        destroyTreesRecursively(primaryPatternNode->nextLevelMatcher);
        rootNode->patternList = primaryPatternNode->nextPrimaryNode;

        for (partNum = 2;
                partNum <= primaryPatternNode->patternNode.partTotal;
                partNum++)
        {
            tPatternNode *patternNode = primaryPatternNode->patternNode.nextPattern + (partNum -2);
            free((void*)patternNode->pattern.pattern);
        }
        free(primaryPatternNode->patternNode.nextPattern);
        free((void*)primaryPatternNode->patternNode.pattern.pattern);
        free(primaryPatternNode);
    }

    _dpd.searchAPI->search_instance_free(rootNode->patternTree);
    free(rootNode);
}

static void dumpTreesRecursively(struct tMlmpTree *rootNode)
{
    tPatternPrimaryNode *primaryPatternNode;
    tPatternNode *ddPatternNode;
    char prefix[41];
    uint32_t prefixSize;

    prefixSize = 4*(rootNode->level)+2;
    if (prefixSize > 40)
        prefixSize = 40;

    memset(prefix, ' ', prefixSize);
    prefix[prefixSize] = '\0';

    for (primaryPatternNode = rootNode->patternList;
            primaryPatternNode;
            primaryPatternNode = primaryPatternNode->nextPrimaryNode)
    {

        printf("%s%u. Primary id %u. partTotal %u, Data %p\n", prefix,
                rootNode->level+1,
                primaryPatternNode->patternNode.patternId,
                primaryPatternNode->patternNode.partTotal,
                primaryPatternNode->patternNode.userData);

        for (ddPatternNode = &primaryPatternNode->patternNode;
                ddPatternNode;
                ddPatternNode = ddPatternNode->nextPattern)
        {
            printf("%s\t part %u/%u: Pattern %s, size %u\n", prefix,
                    ddPatternNode->partNum,
                    ddPatternNode->partTotal,
                    (char *)ddPatternNode->pattern.pattern,
                    (u_int32_t)ddPatternNode->pattern.patternSize);
        }

        if (primaryPatternNode->nextLevelMatcher)
        {
            dumpTreesRecursively(primaryPatternNode->nextLevelMatcher);
        }
    }
}

/*compares multipart patterns, and orders then according to <patternId, partNum>.  */
/*Comparing multi-parts alphanumerically does not make sense. */
static int compareMlmpPatternList(const tPatternNode *p1, const tPatternNode *p2)
{
    if (p1->patternId != p2->patternId)
        return (p1->patternId - p2->patternId);

    return (p1->partNum - p2->partNum);
}

static  tPatternNode* patternSelector (const tMatchedPatternList *patternMatchList, const uint8_t *payload, bool domain)
{
    tPatternNode* bestNode = NULL;
    tPatternNode* currentPrimaryNode = NULL;
    const tMatchedPatternList *tmpList;
    uint32_t partNum, patternId, patternSize, maxPatternSize;

    /*partTotal = 0; */
    partNum = 0;
    patternId = 0;
    patternSize = maxPatternSize = 0;

#if  _MLMP_DEBUG
    tPatternNode *ddPatternNode;
    printf("\tMatches found -------------------\n"); for (tmpList = patternMatchList;
            tmpList;
            tmpList = tmpList->next)
    {
        ddPatternNode = tmpList->patternNode;
        {
            printf("\t\tid %d, Pattern %s, size %u, partNum %u, partTotal %u, userData %p\n",
                    ddPatternNode->patternId,
                    ddPatternNode->pattern.pattern,
                    (u_int32_t)ddPatternNode->pattern.patternSize,
                    ddPatternNode->partNum,
                    ddPatternNode->partTotal,
                    ddPatternNode->userData);
        }
    }
#endif

    for (tmpList = patternMatchList;
            tmpList;
            tmpList = tmpList->next)
    {
        if (tmpList->patternNode->patternId != patternId)
        {
            /*first pattern */

            /*skip incomplete pattern */
            if (tmpList->patternNode->partNum != 1)
                continue;

            /*new pattern started */
            patternId = tmpList->patternNode->patternId;
            currentPrimaryNode = tmpList->patternNode;
            partNum = 0;
            patternSize = 0;
        }

        if (tmpList->patternNode->partNum == (partNum+1))
        {
            partNum++;
            patternSize += tmpList->patternNode->pattern.patternSize;
        }

        if (tmpList->patternNode->partTotal != partNum)
            continue;

        /*backward compatibility */
        if ((tmpList->patternNode->partTotal == 1)
            && domain && matchDomainPattern(tmpList, payload))
            continue;

        /*last pattern part is seen in sequence */
        if (patternSize >= maxPatternSize)
        {
            maxPatternSize = patternSize;
            bestNode = currentPrimaryNode;
        }
    }

#if _MLMP_DEBUG
    if (bestNode)
    {
        ddPatternNode = bestNode;
        {
            printf("\t\tSELECTED Id %d, pattern %s, size %u, partNum %u, partTotal %u, userData %p\n",
                    ddPatternNode->patternId,
                    ddPatternNode->pattern.pattern,
                    (u_int32_t)ddPatternNode->pattern.patternSize,
                    ddPatternNode->partNum,
                    ddPatternNode->partTotal,
                    ddPatternNode->userData);
        }
    }
    printf("\tMatches end -------------------\n");
#endif
    return bestNode;
}

static  tPatternNode* urlPatternSelector (const tMatchedPatternList *patternMatchList, const uint8_t *payload)
{
    return patternSelector (patternMatchList, payload, true);
}
static  tPatternNode* genericPatternSelector (const tMatchedPatternList *patternMatchList, const uint8_t *payload)
{
    return patternSelector (patternMatchList, payload, false);
}

static int patternMatcherCallback (void *id, void *unused_tree, int index, void *data, void *unused_neg)
{
    tPatternNode *target = (tPatternNode *)id;
    tMatchedPatternList **matchList = (tMatchedPatternList **)data;
    tMatchedPatternList *prevNode;
    tMatchedPatternList *tmpList;
    tMatchedPatternList *newNode;
    int cmp;

    /*sort matches by patternId, and then by partId or pattern// */

#if _MLMP_DEBUG
    printf("\tCallback id %d, Pattern %s, size %u, partNum %u, partTotal %u, userData %p\n",
            target->patternId,
            target->pattern.pattern,
            (u_int32_t)target->pattern.patternSize,
            target->partNum,
            target->partTotal,
            target->userData);
#endif

    for (prevNode = NULL, tmpList = *matchList;
            tmpList;
            prevNode = tmpList, tmpList = tmpList->next)
    {
            cmp = compareMlmpPatternList (target, tmpList->patternNode);
            if (cmp > 0 ) continue;
            if (cmp == 0) return 0;
            break;
    }


    newNode = calloc(1,sizeof(*newNode));
    if (!newNode)
    {
        /*terminate search */
        return 1;
    }
    newNode->index = index;
    newNode->patternNode = target;

    if (prevNode == NULL)
    {
        /*first node */
        newNode->next = *matchList;
        *matchList = newNode;
    }
    else
    {
        newNode->next = prevNode->next;
        prevNode->next = newNode;
    }

    return 0;
}

/*find a match and insertion point if no match is found. Insertion point NULL means */
static tPatternPrimaryNode * findMatchPattern(struct tMlmpTree* rootNode, const tMlmpPattern *inputPatternList, uint32_t partTotal, tPatternPrimaryNode** prevPrimaryPatternNode)
{
    tPatternPrimaryNode *primaryPatternNode;
    tPatternNode *ddPatternNode;
    uint32_t partNum;
    int retVal;

    *prevPrimaryPatternNode = NULL;

    for (primaryPatternNode = rootNode->patternList;
                primaryPatternNode;
                *prevPrimaryPatternNode = primaryPatternNode, primaryPatternNode = primaryPatternNode->nextPrimaryNode
                )
    {
        if (primaryPatternNode->patternNode.partTotal != partTotal)
        {
            continue;
        }

        partNum = 1;
        for (ddPatternNode = &primaryPatternNode->patternNode;
                ddPatternNode;
                ddPatternNode = ddPatternNode->nextPattern)
        {
            retVal = compareMlmpPatterns(inputPatternList+(partNum-1), &ddPatternNode->pattern);
            if (retVal == 0)
            {
                /*all nodes matched */
                if (partNum == ddPatternNode->partTotal)
                    return primaryPatternNode;
                else
                    continue;
            }
            else if (retVal < 0)
            {
                return NULL;
            }
            break;
        }
        /**prevPrimaryPatternNode = primaryPatternNode; */
    }
    return NULL;
}

/**
 * @Note
 * a. Patterns in each patternList must be unique. Multipart patterns should be unique i.e. no two multi-part patterns
 * should have same ordered sub-parts.
 * b. Patterns are add in alphabetical ordering of primary nodes.
 */
static int addPatternRecursively(struct tMlmpTree *rootNode, const tMlmpPattern *inputPatternList, void *metaData, uint32_t level)
{
    tPatternNode *newNode;
    tPatternPrimaryNode*   prevPrimaryPatternNode = NULL;
    tPatternPrimaryNode *primaryNode = NULL;
    const tMlmpPattern *nextPattern;
    const tMlmpPattern *patterns = inputPatternList;
    uint32_t partTotal = 0;
    uint32_t patternId = 0;
    uint32_t i;

    if (!rootNode || !inputPatternList)
        return -1;

    /*make it easier for user to add patterns by calculating partTotal and partNum */
    for ( i = 0, patterns = inputPatternList;
            patterns->pattern && (patterns->level == level);
            patterns = inputPatternList + (++i))
    {
        partTotal++;
    }

    /*see if pattern is present already. Multipart-messages are considered match only if all parts */
    /*match. */
    primaryNode = findMatchPattern(rootNode, inputPatternList, partTotal, &prevPrimaryPatternNode);

    /*pattern not found, insert it in order */
    if (!primaryNode)
    {
        tPatternPrimaryNode *tmpPrimaryNode;
        uint32_t partNum;

        tmpPrimaryNode = (tPatternPrimaryNode *)calloc(1, sizeof(tPatternPrimaryNode));
        if (!tmpPrimaryNode)
        {
            return -1;
        }

        if (partTotal > 1)
        {
            tmpPrimaryNode->patternNode.nextPattern = (tPatternNode *)calloc(partTotal-1, sizeof(tPatternNode));
            if (!tmpPrimaryNode->patternNode.nextPattern)
            {
                free(tmpPrimaryNode);
                return -1;
            }
        }

        patternId = gPatternId++;
        i = 0;
        patterns = inputPatternList+i;

        /*initialize primary Node */
        tmpPrimaryNode->patternNode.pattern.pattern = patterns->pattern;
        tmpPrimaryNode->patternNode.pattern.patternSize = patterns->patternSize;
        tmpPrimaryNode->patternNode.pattern.level = patterns->level;
        tmpPrimaryNode->patternNode.partNum = 1;
        tmpPrimaryNode->patternNode.partTotal = partTotal;
        tmpPrimaryNode->patternNode.patternId = patternId;

        if (prevPrimaryPatternNode)
        {
            tmpPrimaryNode->nextPrimaryNode = prevPrimaryPatternNode->nextPrimaryNode;
            prevPrimaryPatternNode->nextPrimaryNode = tmpPrimaryNode;
        }
        else
        {
            /*insert as first node since either this is the only node, or this is lexically smallest. */
            tmpPrimaryNode->nextPrimaryNode = rootNode->patternList;
            rootNode->patternList = tmpPrimaryNode;;
        }

        i++;
        patterns = inputPatternList + i;

        /*create list of remaining nodes  */
        for (partNum = 2;
                partNum <= partTotal;
                partNum++)
        {
            newNode = tmpPrimaryNode->patternNode.nextPattern + (partNum -2);
            newNode->pattern.pattern = patterns->pattern;
            newNode->pattern.patternSize = patterns->patternSize;
            newNode->pattern.level = patterns->level;
            newNode->partNum = partNum;
            newNode->partTotal = partTotal;
            newNode->patternId = patternId;
            if (partNum < partTotal)
                newNode->nextPattern = newNode+1;
            else
                newNode->nextPattern = NULL;

            i++;
            patterns = inputPatternList + i;
        }
        primaryNode = tmpPrimaryNode;
    }
    else
    {
        for (i = 0; i < primaryNode->patternNode.partTotal; i++)
            free((void*)(inputPatternList+i)->pattern);

    }

    if (primaryNode)
    {
        /*move down the new node */
        nextPattern = inputPatternList + partTotal;
        if (!nextPattern || !nextPattern->pattern)
        {
            primaryNode->patternNode.userData = metaData;
        }
        else
        {
            if (!primaryNode->nextLevelMatcher)
            {
                tTreeNode * tmpRootNode;

                tmpRootNode = (tTreeNode *)calloc(1, sizeof(tTreeNode));
                if (!tmpRootNode)
                {
                    /*log error */
                    return -1;
                }
                primaryNode->nextLevelMatcher = tmpRootNode;
                primaryNode->nextLevelMatcher->level = rootNode->level+1;
            }
            addPatternRecursively(primaryNode->nextLevelMatcher, inputPatternList+partTotal, metaData, level+1);
        }
    }

    return 0;
}

