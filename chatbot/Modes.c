//
//  chatbot
//  Modes.c
//
//  Created by Ashish Ahuja on 22/6/2016
//  Copyright © 2016 Fortunate-MAN. All right reserved.
//

#include "Modes.h"

void changeModeOff (ChatBot *bot, int modeType)
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
