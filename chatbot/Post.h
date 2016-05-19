//
//  Post.h
//  chatbot
//
//  Created on 5/18/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef Post_h
#define Post_h

typedef struct {
    char *title;
    char *body;
    unsigned long postID;
    unsigned char isAnswer;
    unsigned long userID;
}Post;

Post *createPost(const char *title, const char *body, unsigned long postID, unsigned char isAnswer, unsigned long userID);
void deletePost(Post *p);

#endif /* Post_h */
