//
//  test_commands.h
//  chatbot
//
//  Created by Jonathan Keller on 5/9/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

#ifndef test_commands_h
#define test_commands_h


void test1Callback(RunningCommand *command, void *ctx) {
    puts("Test 1");
    sleep(10);
}

void testVarCallback(RunningCommand *command, void *ctx) {
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
    sleep(10);
}





#endif /* test_commands_h */
