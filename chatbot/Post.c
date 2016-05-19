//
//  Post.c
//  chatbot
//
//  Created on 5/18/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include "Post.h"
#include <string.h>
#include <stdlib.h>

Post *createPost(const char *title, const char *body, unsigned long postID, unsigned long userID) {
    Post *p = malloc(sizeof(Post));
    p->title = malloc(strlen(title) + 1);
    strcpy(p->title, title);
    
    p->body = malloc(strlen(body) + 1);
    strcpy(p->body, body);
    
    p->postID = postID;
    p->userID = userID;
    
    return p;
}

void deletePost(Post *p) {
    free(p->title);
    free(p->body);
}