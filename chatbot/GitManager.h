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

#define PUSH 1
#define PULL 2
#define DIVERGED 3
#define LATEST 4

char *getLatestSha (char *branch);
char *getLatestShaLink (char *branch);
char *getShortSha (char *branch);
char *getLatestCommitLink (char *branch);
void remoteUpdate ();
char *getLatestCommitText (char *branch);
unsigned getCurrentStatus (char *branch);
void addForCommit (char *toAdd);
void emailConfig (char *email);
void commit (char *message);
void push (char *email, char *pass);

#endif /* GitManager.h */
