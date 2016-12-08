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
