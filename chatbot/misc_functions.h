//
//  misc_functions.h
//  chatbot
//
//  Created by Jonathan Keller on 5/31/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include <stdio.h>

void lowercase (char *str);
void removeSpaces (char* source);
int isTagProgrammingRelated (char *tag);
int postHasTags (Post *post, char *tag);
unsigned isTagInFilter (ChatBot *bot, char *tag);
void removeChar (char* str, char c);
