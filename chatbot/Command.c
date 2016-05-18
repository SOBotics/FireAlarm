//
//  Command.c
//  chatbot
//
//  Created by Jonathan Keller on 5/3/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#include "Command.h"
#include <stdlib.h>
#include <string.h>

Command *createCommand(char *name, void (*callback)(RunningCommand *, void *ctx)) {
    Command *c = malloc(sizeof(Command));
    c->name = name;
    c->callback = callback;
    return c;
}