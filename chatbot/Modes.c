//
//  chatbot
//  Modes.c
//
//  Created by Ashish Ahuja on 22/6/2016
//  Copyright © 2016 Fortunate-MAN. All right reserved.
//

#include "Modes.h"

void disableMode (ChatBot *bot, int modeType)
{
    Modes *modes = bot->modes;
    
    if (modeType == MODE_REPORTING)
    {
        modes->reportMode = 0;
        return;
    }
    else if (modeType == MODE_KEYWORD)
    {
        modes->keyowrdFilter = 0;
        return;
    }
    else if (modeType == MODE_LENGTH)
    {
        modes->lengthFilter = 0;
        return;
    }
    else if (modeType == MODE_MESSAGE)
    {
        modes->messagePost = 0;
        return;
    }
    
    fprintf (stderr, "Invalid mode type %d.", modeType);
    return;
}

void enableMode (ChatBot *bot, int modeType)
{
    Modes *modes = bot->modes;
    
    if (modeType == MODE_REPORTING)
    {
        modes->reportMode = 1;
        return;
    }
    else if (modeType == MODE_KEYWORD)
    {
        modes->keyowrdFilter = 1;
        return;
    }
    else if (modeType == MODE_LENGTH)
    {
        modes->lengthFilter = 1;
        return;
    }
    else if (modeType == MODE_MESSAGE)
    {
        modes->messagePost = 1;
        return;
    }
    
    fprintf (stderr, "Invalid mode type %d.", modeType);
    return;
}

Modes *createMode (int reportMode, int keywordFilter, int lengthFilter, int messagePost)
{
    Modes *mode;
    
    mode->reportMode = reportMode;
    mode->keywordFilter = keywordFilter;
    mode->lengthFilter = lengthFilter;
    mode->messagePost = messagePost;
    
    return mode;
}
