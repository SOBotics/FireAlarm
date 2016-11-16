//
//  test_commands.h
//  chatbot
//
//  Created on 5/9/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef test_commands_h
#define test_commands_h

void getTagsTest (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    unsigned long postID = strtol (command->argv [0], NULL, 10);

    char *tags = getTagsByID (bot, postID);
    char *str;
    asprintf (&str, "The tags are: %s", tags);
    postMessage (bot->room, str);
    free (str);
    return;
}

void test1Callback(RunningCommand *command, void *ctx) {
    puts("Test 1");
    getTagsTest (command, ctx);
}

void testVarCallback(RunningCommand *command, void *ctx) {
    puts("testVarCallback");
    for (int i = 0; i < command->argc; i++) {
        puts(command->argv[i]);
    }
    sleep(10);
}

void testArgCallback(RunningCommand *command, void *ctx) {
    puts(command->argv[0]);
    sleep(10);
}

void test2Callback(RunningCommand *command, void *ctx) {
    ChatBot *bot = ctx;
    for ( int i = 0; i < 35; i ++)
    {
        int quota = getApiQuota (bot);
        getApiQuota (bot);
        getApiQuota (bot);
        getApiQuota (bot);
        //printf ("quota is %d\n", quota);
    }
    return;
}



#endif /* test_commands_h */
