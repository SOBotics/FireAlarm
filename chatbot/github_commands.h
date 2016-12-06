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
    //char *sha = executeCommand("git ls-remote https://github.com/NobodyNada/FireAlarm | awk \"/master/ {print \$1}\"");
    char *sha = executeCommand("git ls-remote https://github.com/NobodyNada/FireAlarm | awk '{ print $1 }'");
    puts (sha);
    postMessage (bot->room, sha);
    return;
}

#endif /* github_commands_h */
