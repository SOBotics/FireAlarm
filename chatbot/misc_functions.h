//
//  misc_functions.h
//  chatbot
//
//  Created on 5/31/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include <stdio.h>

void lowercase (char *str);
void removeSpaces (char* source);
int isTagProgrammingRelated (char *tag);
int postHasTags (ChatBot *bot, Post *post, char *tag);
/* unsigned isTagInFilter (ChatBot *bot, char *tag);
void removeChar (char *str, char c); */
unsigned isStringContainingNumbers2 (char *str);
unsigned isFileEmpty (FILE *file);
char *getCurrentTime ();
void reverseString(char *str);
char *readLine (FILE *file, int lineNum);
long long getCurrentTimeInSeconds ();
char *getCurrentUTCTime ();
char *concat (const char *s1, const char *s2);
char *executeCommand (char *command);
void stripNewlines(char *s);
void replaceSubString(char *target, const char *needle, const char *replacement);
unsigned getTotalPunctuation (char *str);
