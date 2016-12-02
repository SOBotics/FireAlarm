//
//  LQDetector.c
//  chatbot
//
//  Created on 5/18/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include <string.h>
#include <stdlib.h>

#include "Filter.h"

Filter *createFilter(const char *desc, const char *filter, FilterType type, unsigned truePositives, unsigned falsePositives, unsigned isDisabled) {
    Filter *f = malloc(sizeof(Filter));
    f->desc = malloc(strlen(desc) + 1);
    strcpy(f->desc, desc);
    f->filter = malloc(strlen(filter) + 1);
    strcpy(f->filter, filter);

    f->type = type;
    f->truePositives = truePositives;
    f->falsePositives = falsePositives;
    f->isDisabled = isDisabled;

    int error;
    if (type == FILTER_REGEX && (error = regcomp(&f->regex, f->filter, REG_ICASE))) {
        const unsigned max = 1024;
        char msg[max];
        regerror(error, &f->regex, msg, max);
        fprintf(stderr, "Error compiling regex %s: %s\n", filter, msg);
        exit(EXIT_FAILURE);
    }
    return f;
}

int filterNamed (char *name)
{
    if (!strcmp (name, "keyword") || !strcmp (name, "keywords"))
    {
        return FILTER_TEXT;
    }
    else if (!strcmp (name, "length") || !strcmp (name, "body"))
    {
        return FILTER_SHORTBODY;
    }
    else if (!strcmp (name, "tag") || !strcmp (name, "tags"))
    {
        return FILTER_TAG;
    }
    else if (!strcmp (name, "cap") || !strcmp (name, "caps"))
    {
        return FILTER_CAPS;
    }
    else if (!strcmp (name, "textwall") || !strcmp (name, "wall"))
    {
        return FILTER_TEXTWALL;
    }
    else if (!strcmp (name, "exclamation"))
    {
        return FILTER_OVEREXCLAMATION;
    }
    return -1;
}

unsigned char matchRegexFilter(Post *post, Filter *f, unsigned *outStart, unsigned *outEnd) {
    regmatch_t match;
    int error = regexec(&f->regex, post->body, 1, &match, 0);
    if (error == REG_NOMATCH) {
        return 0;
    }
    if (error) {
        const unsigned max = 1024;
        char msg[max];
        regerror(error, &f->regex, msg, max);
        fprintf(stderr, "Error executing regex %s: %s\n", f->filter, msg);
        exit(EXIT_FAILURE);
    }
    if (outStart) *outStart = (unsigned)match.rm_so;
    if (outEnd) *outEnd = (unsigned)match.rm_eo;
    return 1;
}

unsigned char postMatchesFilter(ChatBot *bot, Post *post, Filter *filter, unsigned *outStart, unsigned *outEnd) {
    unsigned titleLength;
    char *body;

    if (filter->isDisabled)
    {
       // puts ("Filter is disabled!");
        return 0;
    }
    if (post->owner->userRep > 500)
    {
        return 0;
    }

    switch (filter->type) {
        case FILTER_TEXT:
            ;char *start = strstr(post->body, filter->filter);
            if (start) {
                if (outStart) *outStart = (unsigned)(start - post->body);
                if (outEnd) *outEnd = (unsigned)((start - post->body) + strlen(filter->filter));
                return 1;
            }
            return 0;
        case FILTER_REGEX:
            return matchRegexFilter(post, filter, outStart, outEnd);
        case FILTER_SHORTBODY:
            return strlen(post->body) < 500;
        case FILTER_TAG:
            return matchTagFilter (bot, post, filter);
        case FILTER_CAPS:
            titleLength = strlen (post->title);
            unsigned bodyLength = strlen (post->body);
            unsigned totalCaps = 0;
            totalCaps = getCapsInString (post->title);
            if ((totalCaps/titleLength) * 100 > 25)
            {
                puts ("Too many caps in title!");
                printf ("%d\n", (totalCaps/titleLength) * 100);
                return 1;
            }
            totalCaps = getCapsInString (post->body);
            if ((totalCaps/bodyLength) * 100 > 20)
            {
                puts ("Too many caps in body!");
                printf ("%d\n", (totalCaps/bodyLength) * 100);
                return 1;
            }
            return 0;
        case FILTER_TEXTWALL:
            if (!strstr (post->body, "<code>") && post->body > 50)
            {
                return 1;
            }
            return 0;
        case FILTER_NOQUESTION:
            if (!strstr (post->body, "?"))
            {
                return 1;
            }
            return 0;
        case FILTER_LINK:
            if ((strstr (post->body, "https://") || strstr (post->body, "http://")) && !strstr (post->body, "<code>"))
            {
                return 1;
            }
            return 0;
        case FILTER_OVEREXCLAMATION:
            body = malloc (post->body + 1);
            strcpy (body, post->body);
            char *i = body;
            char *j = body;
            while(*j != 0)
            {
                *i = *j++;
                if(*i == '!')
                    i++;
            }
            *i = 0;

            unsigned length = strlen (body);
            if (length > 4 && !strstr (post->body, "<code>"))
            {
                free (body);
                return 1;
            }
            free (body);
            return 0;
        default:
            fprintf(stderr, "Invalid filter type %d\n", filter->type);
            //exit(EXIT_FAILURE);
            return 0;
    }
}

unsigned matchTagFilter (ChatBot *bot, Post *post, Filter *filter)
{
    char **tags = getTagsByID (bot, post->postID);

    for (int i = 0; i < 5; i ++)
    {
        if (strcmp (tags [i], filter->filter) == 0)
        {
            return 1;
        }
    }

    return 0;
}
