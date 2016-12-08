//
//  GitManager.h
//  chatbot
//
//  Created by Ashish Ahuja on December 7th 2016
//
//

#ifndef GitManager_h
#define GitManager_h

#include <stdio.h>
#include <stdlib.h>

char *getLatestSha (char *branch);
char *getLatestShaLink (char *branch);
char *getShortSha (char *branch);

#endif /* GitManager.h */
