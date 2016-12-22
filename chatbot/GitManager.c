//
//  GitManager.c
//  chatbot
//
//  Created by Ashish Ahuja on December 7th 2016
//
//

#include "GitManager.h"

char *getLatestSha (char *branch) //Pass the branch whose latest sha you want, for example branch "master"
{
    char *sha = executeCommand("git ls-remote https://github.com/NobodyNada/FireAlarm | awk '{ print $1 }'");
    if (!sha)
    {
        fputs ("An error occurred while retrieving the sha!", stderr);
        return NULL;
    }

    return sha;
}

char *getLatestShaLink (char *branch)
{
    char *sha = getLatestSha (branch);
    if (!sha)
    {
        return NULL;
    }

    unsigned len = strlen (sha);
    len = 50 + len + 1;
    char *link = malloc (sizeof (char) * len);
    snprintf (link, len,
              "https://github.com/NobodyNada/FireAlarm/commit/%s", sha);
    free (sha);
    stripNewlines(link);
    return link;
}

char *getShortSha (char *branch)
{
    char *sha = getLatestSha (branch);
    unsigned len = strlen (sha);
    if (len > 7)
    {
        sha [7] = 0;
    }
    printf ("test short sha: %s\n", sha);

    return sha;
}

char *getLatestCommitLink (char *branch)
{
    char *link = malloc (sizeof (char) * 256);
    sprintf (link, "[%s](%s)", getShortSha ("master"), getLatestShaLink ("master"));
    return link;
}

void remoteUpdate ()
{
    executeCommand ("git remote update");
    return;
}

char *getLatestCommitText (char *branch)
{
    remoteUpdate ();
    char *command;
    asprintf (&command, "git show-branch %s", branch);
    char *output = executeCommand(command);
    free (command);
    char *toRemove;
    asprintf (&toRemove, "[%s] ", branch);
    removeSubstring(output, toRemove);
    free (toRemove);

    stripNewlines(output);

    return output;
}

unsigned getCurrentStatus (char *branch)
{
    remoteUpdate ();
    char *status = executeCommand("./../shellscripts/currentstatus.sh");
    stripNewlines(status);
    puts ("in get..");
    puts (status);
    puts ("STATUS OUTPUT hello blah: ");


    puts (executeCommand("git status -v"));

    if (strstr (executeCommand("git status -v"), "diff"))
    {
        return COMMIT;
    }
    else if (!strcmp (status, "Up-to-date"))
    {
        return LATEST;
    }
    else if (!strcmp (status, "Need to pull"))
    {
        return PULL;
    }
    else if (!strcmp (status, "Need to push"))
    {
        return PUSH;
    }
    else if (!strcmp (status, "Diverged"))
    {
        return DIVERGED;
    }
    else
    {
        fprintf (stderr, "%s is not a valid status type!\n", status);
        free (status);
        return 0;
    }
}

void addForCommit (char *toAdd)
{
    char *str;
    asprintf (&str, "git add %s", toAdd);
    executeCommand(str);
    free (str);
    return;
}

void emailConfig (char *email)
{
    char *str;
    asprintf (&str, "git config --global user.email=\"%s\"", email);
    executeCommand (str);
    free (str);
    return;
}

void commit (char *message)
{
    char *str;
    asprintf (&str, "git commit -m \"%s\"", message);
    executeCommand(str);
    free (str);
    return;
}

void push (char *email, char *pass)
{
    char *str;
    puts ("EMAIL: ");
    puts (email);
    puts (pass);
    asprintf (&str, "git push https://%s:%s@github.com/NobodyNada/FireAlarm.git", email, pass);
    executeCommand(str);
    puts ("IN PUSH:");
    puts (str);
    free (str);
    return;
}
