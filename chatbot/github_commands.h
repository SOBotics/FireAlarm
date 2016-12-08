//
//  github_commands.h
//  chatbot
//
//  Created on December 4th 2016 by Ashish Ahuja.
//  Copyright © 2016 Fortunate-MAN (Ashish Ahuja). All rights reserved.
//

#ifndef github_commands_h
#define github_commands_h

void latestCommit (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    char *message = malloc (sizeof (char) * 1024);
    sprintf (message, "[%s](%s)", getShortSha ("master"), getLatestShaLink ("master"));
    postMessage (bot->room, message);
    free (message);
    return;
}

#endif /* github_commands_h */
