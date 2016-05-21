This is an idea I came up with for autogenerating filters.

The bot keeps a list of recent reports.  If the size of this list grows high enough, the bot clears the list and generates filters based on its contents. 

If a certain percentage of posts with a specific word are true positives, that word is added as a filter (although it might have a reduced weight temporarily so it doesn't cause too many false positives.) The existing algorithms will tune the new filter's weight.
