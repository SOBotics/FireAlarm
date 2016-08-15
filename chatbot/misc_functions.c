//
//  misc_functions.c
//  chatbot
//
//  Created by Ashish Ahuja on 28/5/16.
//  Copyright © 2016 Ashish Ahuja (Fortunate-MAN). All rights reserved.
//

#include <ctype.h>

#include <curl/curl.h>
#include "Client.h"
#include "ChatBot.h"

void lowercase (char *str)
{
    while (*str)
    {
        *str = tolower(*str);
        str++;
    }

    return;
}

void removeSpaces(char* source)
{
  char* i = source;
  char* j = source;
  while(*j != 0)
  {
    *i = *j++;
    if(*i != ' ')
      i++;
  }
  *i = 0;

  return;
}

int isTagProgrammingRelated (char *tag)
{
    char *progTags[] = {
        ".net",
        "accessibility-api",
        "actionscript",
        "actionscript-3",
        "ajax",
        "alljoyn",
        "applescript",
        "android-ndk",
        "android-sdk",
        "android-sdk-2.1",
        "android-sdk-2.2",
        "android-sdk-2.3",
        "anrdoid-sdk-tools",
        "android-studio",
        "angularjs",
        "asp.net",
        "asp.net5",
        "asp.net-mvc",
        "asp.net-mvc-3",
        "asp.net-mvc-4",
        "asp.net-mvc-5",
        "asp.net-mvc6",
        "awk",
        "bash",
        "c",
        "c#",
        "c++",
        "c++11",
        "cakephp",
        "cakephp-1.0",
        "cakephp-1.3",
        "cakephp-2.0",
        "cakephp-2.1",
        "cakephp-2.3",
        "cakephp-3.0",
        "clisp",
        "clojure",
        "cocoa",
        "cocoa-touch",
        "cocos2d",
        "cocos2d-iphone",
        "cocos2d-x",
        "codeigniter",
        "common-lisp",
        "cordova",
        "core-text",
        "css",
        "data-structures",
        "delphi",
        "django",
        "dom",
        "elisp",
        "excel-vba",
        "expect",
        "git",
        "grails",
        "hadoop",
        "haskell",
        "html",
        "html5",
        "jasper-reports",
        "java",
        "java-ee",
        "javascript",
        "jpa",
        "jsf",
        "json",
        "jsp",
        "jquery",
        "jquery-ui",
        "laravel",
        "lisp",
        "magento",
        "matlab",
        "maven",
        "mercurial",
        "mongodb",
        "msbuild",
        "mybatis",
        "mysql",
        "netbeans",
        "node.js",
        "numpy",
        "objective-c",
        "oop",
        "opencv",
        "opengl",
        "oracle",
        "pandas",
        "perl",
        "powershell",
        "php",
        "prolog",
        "python",
        "python-2.7",
        "python-2.x",
        "python-3.x",
        "qt",
        "r",
        "racket",
        "regex",
        "ruby",
        "ruby-on-rails",
        "ruby-on-rails-3",
        "ruby-on-rails-4",
        "sapui",
        "sapui5",
        "scala",
        "scheme",
        "sed",
        "selenium",
        "sh",
        "shell",
        "sockets",
        "spring-mybatis",
        "spring-mvc",
        "sql",
        "sql-server",
        "sql-server-2008",
        "sqlite",
        "swift",
        "symfony1",
        "symfony2",
        "symfony3",
        "tcl",
        "tsql",
        "vb.net",
        "vb6",
        "vba",
        "vbscript",
        "verilog",
        "visual-studio",
        "visual-studio-2010",
        "vscode",
        "winapi",
        "wcf",
        "xaml",
        "xcode",
        "xml",
        "xslt"
    }

    for (int i = 0; i < 139; i ++)
    {
        if (strcasestr (tag, progTags [i]) == tag)
        {
            return 1;
        }
    }

    return 0;
}

int postHasTags (Post *post, char *tag)
{
    char **tags = getTagsByID (post->postID);

    for (int i = 0; i < 5; i ++)
    {
        if (strcmp (tags [i], tag) == 0)
        {
            return 1;
        }
    }

    return 0;
}

unsigned isTagInFilter (ChatBot *bot, char *tag)
{
    Filter **filters = bot->filters;

    for (int i = 0; i < bot->totalFilters; i ++)
    {
        if (strcmp (filters [i]->filter, tag) == 0)
        {
            return 1;
        }
    }

    return 0;
}

void removeChar (char* str, char c) {
    char *pr = str;
    char *pw = str;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';
}

unsigned isStringCotainingNumbers (char *str)
{
    unsigned len = strlen (str);

    for (int i = 0; i < len; i ++)
    {
        if (!(str [i] >= '0' && str [i]<= '9'))
            return 0;
    }

    return 1;
}
