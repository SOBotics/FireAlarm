//
//  chatbot
//  Modes.h
//  
//  Created by Ashish Ahuja on 23/6/2016
//  Copyright © Fortunate-MAN. All rights reserved.
//

#ifndef Modes_h
#define Modes_h

typedef struct _ChatBot ChatBot;

typedef enum {
    MODE_REPORTING,
    MODE_KEYWORD,
    MODE_LENGTH,
    MODE_MESSAGE,
}modeType;

typedef struct {
    // All the variables are 1 if the mode is on, and 0 if off
    int reportMode;
    int keywordFilter;
    int lengthFilter;
    int messagePost;
}Modes;

void disableMode (ChatBot *bot, int modeType);
void enableMode (ChatBot *bot, int modeType);
Modes *createMode (int reportMode, int keywordFilter, int lengthFilter, int messagePost);

#endif /* Modes.h */
