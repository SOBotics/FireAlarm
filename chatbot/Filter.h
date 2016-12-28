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
    FILTER_TEXT = 0,
    FILTER_REGEX = 1,
    FILTER_SHORTBODY = 2,
    FILTER_TAG = 3,
    FILTER_CAPS = 4,
    FILTER_TEXTWALL = 5,
    FILTER_NOQUESTION = 6,
    FILTER_LINK = 7,
    FILTER_OVEREXCLAMATION = 8,
    FILTER_PUNCTUATION = 9
}FilterType;

typedef struct {
    char *filter;
    regex_t regex;
    FilterType type;
    unsigned truePositives;
    unsigned falsePositives;
    char *desc;
    unsigned isDisabled; //1 if the filter is disabled, 0 if the filter is enabled
}Filter;


Filter *createFilter(const char *desc, const char *filter, FilterType type, unsigned truePositives, unsigned falsePositives, unsigned isDisabled);
int filterNamed (char *name);
unsigned postMatchestagFilter (ChatBot *bot, Post *post);
unsigned char matchRegexFilter (Post *post, Filter *filter, unsigned *outStart, unsigned *outEnd);
unsigned matchTagFilter (ChatBot *bot, Post *post, Filter *filter);

//Returns whether or not the specified post matchis the specified filter.
//On return, *outStart is the index of the start of the match.
//*outEnd is the index of the end of the match.
unsigned char postMatchesFilter(ChatBot *bot, Post *post, Filter *filter, unsigned *outStart, unsigned *outEnd);

#endif /* LQDetector_h */
