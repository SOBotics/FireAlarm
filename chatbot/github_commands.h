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

    switch (status) {
        case PULL:
            asprintf (&message, "Need to pull.");
            break;
        case PUSH:
            asprintf (&message, "Need to push.");
            break;
        case LATEST:
            asprintf (&message, "Up to date.");
            break;
        case DIVERGED:
            asprintf (&message, "Status diverged. This is most probably bad.");
            break;
        case COMMIT:
            asprintf (&message, "Need to commit.");
            break;
        default:
            asprintf (&message, "Unknown status `%u` returned by `getCurrentStatus`!", status);
    }

    puts (message);
    postReply (bot->room, message, command->message);
    free (message);

    return;
}

void commandPull (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;

    unsigned status = getCurrentStatus("master");

    switch (status)
    {
        case PUSH:
            postReply (bot->room, "Need to push locally, cannot pull.", command->message);
            return;
        case DIVERGED:
            postReply (bot->room, "Status diverged. This is most probably bad.", command->message);
            return;
        case LATEST:
            postReply (bot->room, "Nothing to pull.", command->message);
            return;
        case PULL:
            pull ("master");
            if (!build())
            {
                postReply (bot->room, "Compile error.", command->message);
                return;
            }
            else if (build())
            {
                bot->stopAction = ACTION_REBOOT;
            }
            break;
        default:
            fprintf (stderr, "Unknown status: %u\n", status);
            return;
    }
    return;
}

#endif /* github_commands_h */
