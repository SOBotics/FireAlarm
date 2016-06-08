//
//  Command.h
//  chatbot
//
//  Created on 5/3/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef Command_h
#define Command_h

#include <stdio.h>
#include "RunningCommand.h"

typedef struct _Command {
    char *name;
    int isPrivileged;   // 1 if the command is privileged, 0 if not
    void (*callback)(RunningCommand *, void *);
}Command;

Command *createCommand(char *name, int isPrivileged, void (*callback)(RunningCommand *, void *));

#endif /* Command_h */
