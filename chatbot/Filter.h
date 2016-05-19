//
//  LQDetector.h
//  chatbot
//
//  Created on 5/18/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef LQDetector_h
#define LQDetector_h

#include <stdio.h>
#include <regex.h>

#include "ChatRoom.h"
#include "Post.h"

typedef enum {
    FILTER_REGEX
}FilterType;

typedef struct {
    char *filter;
    regex_t regex;
    FilterType type;
    char *desc;
}Filter;


Filter *createFilter(const char *desc, const char *filter, FilterType type);

//Returns whether or not the specified post matchis the specified filter.
//On return, *outStart is the index of the start of the match.
//*outEnd is the index of the end of the match.
unsigned char postMatchesFilter(Post *post, Filter *filter, unsigned *outStart, unsigned *outEnd);

#endif /* LQDetector_h */
