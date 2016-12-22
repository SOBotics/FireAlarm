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
    sprintf (message, "[%s](%s) (%s)", getShortSha ("master"), getLatestShaLink ("master"), getLatestCommitText ("master"));
    postMessage (bot->room, message);
    free (message);
    return;
}

void status (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    char *message;
    unsigned status = getCurrentStatus ("master");
    if (status == PULL)
    {
        asprintf (&message, "Need to pull.");
    }
    else if (status == PUSH)
    {
        asprintf (&message, "Need to push.");
    }
    else if (status == LATEST)
    {
        asprintf (&message, "Up to date.");
    }
    else if (status == DIVERGED)
    {
        asprintf (&message, "Status diverged!");
    }
    else if (status == 0)
    {
        asprintf (&message, "An error occurred!");
    }

    puts (message);
    postReply (bot->room, message, command->message);
    free (message);

    return;
}

#endif /* github_commands_h */
