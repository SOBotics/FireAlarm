//
//  misc_functions.c
//  chatbot
//
//  Created on 28/5/16.
//  Copyright © 2016 Ashish Ahuja. All rights reserved.
//

#include <ctype.h>

void lowercase (char *str)
{
    while (*str)
    {
        *str = tolower(*str);
        str++;
    }
    
    return;
}

/* End of file: misc_functions.h */
