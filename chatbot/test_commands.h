//
//  test_commands.h
//  chatbot
//
//  Created on 5/9/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef test_commands_h
#define test_commands_h


void test1Callback(RunningCommand *command, void *ctx) {
    puts("Test 1");
    sleep(10);
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
    puts("Test 2");
    ChatBot *bot = ctx;
    char *str;
    asprintf (&str, "bot->reportsUntilAnalysis is %d", bot->reportsUntilAnalysis);
    postMessage (bot->room, str);
    free (str);
    return;
}

void getTagsTest (RunningCommand *command, void *ctx)
{
    ChatBot *bot = ctx;
    unsigned long postID = strtol (command->argv [0], NULL, 10);

    char **tags = getTagsByID (bot, postID);
    char *str;
    asprintf (&str, "The tags are: %s %s %s", tags [0], tags [1], tags [2]);
    postMessage (bot->room, str);
    free (str);
    return;
}



#endif /* test_commands_h */
