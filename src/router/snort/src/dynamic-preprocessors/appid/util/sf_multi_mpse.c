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
#include "sf_multi_mpse.h"
#include "string.h"
#include "sf_dynamic_preprocessor.h"

struct _patternRootNode;
typedef struct _tPatternList {
    tMlpPattern     pattern;
    void         *userData;     /*client/service info */

    struct _tPatternList  *nextPattern;
    struct _patternRootNode  *nextLevelMatcher;

} tPatternList ;

/*Root node */
typedef struct _patternRootNode {
    void         *patternTree;
    tPatternList *patternList;
    tPatternList *lastPattern;
    unsigned int  level;       /*some searches may be specific to levels. Increments from 1 at top level, */

} tPatternRootNode ;

/*Used to track matched patterns. */
typedef struct {
    tPatternList *patternNode;
    size_t       index;
    unsigned int level;
} MatchedPattern;

static int compareAppUrlPatterns(const void *p1, const void *p2);
static int createTreesRecusively(void *root);
static void destroyTreesRecursively(void *root);
static void dumpTreesRecursively(void *root, int level);
static int addPatternRecursively(void *root, const tMlpPattern **inputPatternList, void *metaData, int level);
static int longest_pattern_match (void *id, void *unused_tree, int index, void *data, void *unused_neg);
static int url_pattern_match (void *id, void *unused_tree, int index, void *data, void *unused_neg);

void *mlpCreate(void)
{
    tPatternRootNode *root = calloc(1, sizeof(tPatternRootNode));
    if (root) root->level = 0;
    return root;
}

/*last pattern should be NULL */
int mlpAddPattern(void *root, const tMlpPattern **inputPatternList, void *metaData)
{
    return addPatternRecursively(root, inputPatternList, metaData, 0);
}

int mlpProcessPatterns(void *root)
{
    int rvalue;

   rvalue = createTreesRecusively(root);
   if (rvalue) destroyTreesRecursively(root);
   return rvalue;
}
void *mlpMatchPatternLongest(void *root, tMlpPattern **inputPatternList)
{
    return mlpMatchPatternCustom(root, inputPatternList, longest_pattern_match);
}

void *mlpMatchPatternUrl(void *root, tMlpPattern **inputPatternList)
{
    return mlpMatchPatternCustom(root, inputPatternList, url_pattern_match);
}

static inline int matchDomainPattern(MatchedPattern mp, const uint8_t *pattern)
{
    if (!pattern)
        return -1;

    return (mp.level == 0 && !(mp.index == 0 || pattern[mp.index-1] == '.'));
}

void *mlpMatchPatternCustom(void *root, tMlpPattern **inputPatternList, int (*callback)(void*, void*, int, void*, void*))
{
    MatchedPattern mp = {0};
    void *data = NULL;
    void *tmpData = NULL;
    tPatternList *patternNode;
    tPatternRootNode *rootNode = (tPatternRootNode *)root;
    tMlpPattern *pattern = *inputPatternList;

    if (!rootNode || !pattern || !pattern->pattern)
        return NULL;

    mp.level = rootNode->level;

    _dpd.searchAPI->search_instance_find_all(rootNode->patternTree,
            (char *)pattern->pattern,
            pattern->patternSize, 0,
            callback, (void*)&mp);

    patternNode = mp.patternNode;
    if(patternNode)
    {
        if (matchDomainPattern(mp, pattern->pattern) != 0)
            return NULL;

        data = patternNode->userData;
        tmpData = mlpMatchPatternCustom(patternNode->nextLevelMatcher, ++inputPatternList, callback);
        if (tmpData)
            data = tmpData;
    }

    return data;
}

void mlpDestroy(void *root)
{
    destroyTreesRecursively(root);
}

void mlpDump(void *root)
{
    dumpTreesRecursively(root, 0);
}

/*alphabetically ordering */
static int compareAppUrlPatterns(const void *p1, const void *p2)
{
    tMlpPattern *pat1 = (tMlpPattern *)p1;
    tMlpPattern *pat2 = (tMlpPattern *)p2;
    int rValue;
    size_t minSize;

    /*first compare patterns by the smaller pattern size, if same then size wins */
    minSize = (pat1->patternSize > pat2->patternSize)? pat2->patternSize: pat1->patternSize;

    rValue = memcmp(pat1->pattern, pat2->pattern, minSize);
    if (rValue) return rValue;

    return ((int)pat1->patternSize - (int)pat2->patternSize);
}

/*pattern trees are not freed on error because in case of error, caller should call detroyTreesRecursively. */
static int createTreesRecusively(void *root)
{
    tPatternRootNode *rootNode = (tPatternRootNode *)root;
    void *patternMatcher;
    tPatternList *patternNode;

    /* set up the MPSE for url patterns */
    if (!(patternMatcher = rootNode->patternTree = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF)))
        return -1;

    for (patternNode = rootNode->patternList;
            patternNode;
            patternNode = patternNode->nextPattern)
    {
        /*recursion into next lower level */
        if (patternNode->nextLevelMatcher)
        {
            if (createTreesRecusively(patternNode->nextLevelMatcher)) return -1;
        }

        _dpd.searchAPI->search_instance_add_ex(patternMatcher,
                    (void *)patternNode->pattern.pattern,
                    patternNode->pattern.patternSize,
                    patternNode,
                    STR_SEARCH_CASE_SENSITIVE);
    }

    _dpd.searchAPI->search_instance_prep(patternMatcher);

    return 0;
}

static void destroyTreesRecursively(void *root)
{
    tPatternRootNode *rootNode = (tPatternRootNode *)root;
    tPatternList *patternNode;

    while ((patternNode = rootNode->patternList))
    {
        /*recursion into next lower level */
        if (patternNode->nextLevelMatcher)
        {
            destroyTreesRecursively(patternNode->nextLevelMatcher);
        }
        rootNode->patternList = patternNode->nextPattern;
        free(patternNode);
    }

    _dpd.searchAPI->search_instance_free(rootNode->patternTree);
    free(rootNode);
}

static void dumpTreesRecursively(void *root, int level)
{
    tPatternRootNode *rootNode = (tPatternRootNode *)root;
    tPatternList *patternNode;
    char *offset;

    offset = malloc(4*level+2);
    if (!offset) return;
    memset(offset, ' ', 4*level+1);
    offset[4*level] = '\0';

    for (patternNode = rootNode->patternList;
            patternNode;
            patternNode = patternNode->nextPattern)
    {
        printf("%sPattern %s, size %u, userData %p\n", offset,
                (char *)patternNode->pattern.pattern,
                (u_int32_t)patternNode->pattern.patternSize,
                patternNode->userData);

        /*recursion into next lower level */
        if (patternNode->nextLevelMatcher)
        {
            dumpTreesRecursively(patternNode->nextLevelMatcher, (level+1));
        }
    }
    free(offset);
}


static int longest_pattern_match (void *id, void *unused_tree, int index, void *data, void *unused_neg)
{
    tPatternList *target = (tPatternList *)id;
    MatchedPattern *match = (MatchedPattern *)data;
    int newMatchWins = 0;

    /*printf("LongestMatcher: level %d, index: %d, matched %s\n", matches->level, index, target->pattern.pattern); */

    /*first match */
    if (!match->patternNode) newMatchWins = 1;
    /*subsequent longer match */
    else if (match->patternNode->pattern.patternSize < target->pattern.patternSize) newMatchWins = 1;

    if (newMatchWins)
    {
        /*printf("new pattern wins\n"); */
        match->patternNode = target;
        match->index = index;
    }

    return 0;
}

static int url_pattern_match (void *id, void *unused_tree, int index, void *data, void *unused_neg)
{
    tPatternList *target = (tPatternList *)id;
    MatchedPattern *match = (MatchedPattern *)data;
    int newMatchWins = 0;

    /*printf("UrlMatcher: level %d, index: %d, matched %s\n", match->level, index, target->pattern.pattern); */
    /*first match */
    if (!match->patternNode) newMatchWins = 1;

    /*subsequent longer match */
    else if (match->patternNode->pattern.patternSize < target->pattern.patternSize) newMatchWins = 1;
    else if (match->patternNode->pattern.patternSize == target->pattern.patternSize)
    {
        /*host part matching towards later part is better. This is not designed to prevent mis-identifying */
        /*url 'www.spoof_for_google.google.com.phishing.com' as google. */
        if ((match->level == 0) && (match->index < (unsigned int) index)) newMatchWins = 1;
        /*path part matching towards lower index is better */
        if ((match->level == 1) && (match->index > (unsigned int) index)) newMatchWins = 1;
    }

    if (newMatchWins)
    {
        /*printf("new pattern wins\n"); */
        match->patternNode = target;
        match->index = index;
    }

    return 0;
}

static int addPatternRecursively(void *root, const tMlpPattern **inputPatternList, void *metaData, int level)
{
    tPatternRootNode *rootNode = (tPatternRootNode *)root;
    tPatternList*   prevNode = NULL;
    tPatternList *patternList;
    tPatternList *newNode;
    const tMlpPattern *nextPattern;
    const tMlpPattern *patterns = *inputPatternList;
    int rvalue;

    if (!rootNode || !patterns || !patterns->pattern)
        return -1;

    for (patternList = rootNode->patternList;
            patternList;
            prevNode = patternList, patternList = patternList->nextPattern)
    {
        rvalue = compareAppUrlPatterns(patterns, patternList);
        if (rvalue < 0) continue;
        if (rvalue == 0)
        {
            nextPattern = *(inputPatternList+1);

            if (!nextPattern || !nextPattern->pattern)
            {
                /*overriding any previous userData. */
                patternList->userData = metaData;
                return 0;
            }
            return addPatternRecursively(patternList->nextLevelMatcher, inputPatternList+1, metaData, level+1);
        }
        break;
    }

    /*allocate and initialize a new node */
    newNode = (tPatternList *)calloc(1,sizeof(tPatternList));
    if (!newNode)
    {
        return -1;
    }
    newNode->pattern.pattern = patterns->pattern;
    newNode->pattern.patternSize = patterns->patternSize;
    newNode->nextLevelMatcher = (tPatternRootNode *)calloc(1, sizeof(tPatternRootNode));
    if (!newNode->nextLevelMatcher)
    {
        free (newNode);
        return -1;
    }
    newNode->nextLevelMatcher->level = rootNode->level+1;

    /*insert the new node */
    if (!prevNode)
    {
        /*insert as first node since either this is the only node, or this is lexically smallest. */
        newNode->nextPattern = rootNode->patternList;
        rootNode->patternList = newNode;
    }
    else
    {
        /*insert after previous node since either there is either a biggest node after prevNode or */
        /*newNode is lexically largest. */
        newNode->nextPattern = prevNode->nextPattern;
        prevNode->nextPattern = newNode;
    }

    /*move down the new node */
    nextPattern = *(inputPatternList+1);
    if (!nextPattern || !nextPattern->pattern)
    {
        newNode->userData = metaData;
    }
    else
    {
        addPatternRecursively(newNode->nextLevelMatcher, inputPatternList+1, metaData, level+1);
    }

    return 0;
}

/**returns pattern tree at the level where inputPatternList runs out.
 */
void *mlpGetPatternMatcherTree(void *root, tMlpPattern **inputPatternList)
{
    MatchedPattern mp = {0};
    tPatternList *patternNode;
    tPatternRootNode *rootNode = (tPatternRootNode *)root;
    tMlpPattern *pattern = *inputPatternList;

    if (!rootNode || !pattern || !pattern->pattern)
        return NULL;

    mp.level = rootNode->level;

    _dpd.searchAPI->search_instance_find_all(rootNode->patternTree,
            (char *)pattern->pattern,
            pattern->patternSize, 0,
            longest_pattern_match, (void *)&mp);

    patternNode = mp.patternNode;
    if(patternNode)
    {
        ++inputPatternList;
        if (*inputPatternList && (*inputPatternList)->pattern)
        {
            return mlpMatchPatternCustom(patternNode->nextLevelMatcher, inputPatternList, longest_pattern_match);
        }
        return patternNode->nextLevelMatcher;
    }

    return NULL;
}

