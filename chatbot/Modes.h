//
//  chatbot
//  Modes.h
//  
//  Created by Ashish Ahuja on 23/6/2016
//  Copyright © Fortunate-MAN. All rights reserved.
//

#ifdef Modes_h
#define Modes_h

typedef enum {
    MODE_REPORTING = 0;
    MODE_KEYWORD = 1;
    MODE_LENGTH = 2;
    MODE_MESSAGE = 3;
}modeType;

typedef struct {
    // All the variables are 1 if the mode is on, and 0 if off
    int reportMode;
    int keywordFilter;
    int lengthFilter;
    int messagePost;
}Modes;

#endif /* Modes.h */
