//
//  Command.h
//  chatbot
//
//  Created by Jonathan Keller on 5/3/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef Command_h
#define Command_h

#include <stdio.h>
#include "RunningCommand.h"

typedef struct _Command {
    char *name;
    void (*callback)(RunningCommand *, void *);
}Command;

Command *createCommand(char *name, void (*callback)(RunningCommand *, void *));

#endif /* Command_h */
