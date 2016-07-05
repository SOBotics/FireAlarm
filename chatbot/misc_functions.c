//
//  misc_functions.c
//  chatbot
//
//  Created by Ashish Ahuja on 28/5/16.
//  Copyright © 2016 Ashish Ahuja (Fortunate-MAN). All rights reserved.
//

#include <ctype.h>

#include <curl/curl.h>
#include "Client.h"
#include "ChatBot.h"

void lowercase (char *str)
{
    while (*str)
    {
        *str = tolower(*str);
        str++;
    }
    
    return;
}

