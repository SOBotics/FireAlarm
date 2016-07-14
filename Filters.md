#Filters

The bot detects low-quality posts by running each new post through filters.

Each filter has a weight.  If the sum of the weights of all matching filters is greater than a threshold,
the post is reported.  If a report is marked as a true positive, the weight of each filter that detected it is increased.  If it is marked as a false positive, the weights of the filters are decreased.

List of filters I plan to implement, sorted by percentage of posts matching these filters that are closed:

- "where do I get" 50%
- "plz" 25%
- "write a program that" 25%
- "downvote" 25%
- "battery" 25%
- "find answer" 20%
- "unclear" 20%
- "get started" 17%
- "telephone" 16%
- "opinion" 16%
- "dumb" 16%
- "screw" 14%
- "electr" 14%
- "write code" 13%
- "where do" 13%
- "how will" 13%
- "radio" 12%
- "the best" 12%
- "happy" 11%
- "where can i find" 11%
- "sell" 11%
- "sorry" 10%
- "algo" 10%
- "homework" 10%
- "topic" 10%
- "help me" 10%
- "sorry" 10%
- "usb" 10%
- "computer" 10%
- "legal" 10%
- "answer" 10%
- "cricket" 10%
- "software" 10%
- "to start" 9%
- "cant" 9%
- "where to" 9%
- "english" 9%
- "hardware" 9%
- "good" 8%
- "print" 8%
- "cable" 8%
- "vote" 8%
- "tutorial" 8%
- "thank you" 8%
- "possible" 8%
- "keyboard" 7%
- "laptop" 7%
- "buy" 7%
- "thanks" 7%
- "help" 7%
- "i need" 7%
- "exam" 7%
- "how to" 7%
- "can" 7%
- "will" 7%
- "person" 7%

To convert this list into JSON readable by the program, put the list in a file called `filters.txt`, and run this command:
    
    awk -F \" -f awkscript filters.txt > filters.json

with `awkscript` containing the following:

    { PERCENT_INDEX=index($3,"%") }
    { PERCENTAGE = substr($3, 2, PERCENT_INDEX - 2) }
    BEGIN { print "\[" }
    NR > 1 { print "," }
    { printf "{\"description\": \"%s\",", $2}
    { printf "\"expression\": \"%s\",", $2}
    { printf "\"type\": 0," }
    { printf "\"truePositives\": %s,", PERCENTAGE }
    { printf "\"falsePositives\": -1" }
    { printf "}" }
    END {print "\]" }
    
This will generate a file called `filters.json` that can be read by the chatbot.  The chatbot will look for this file in `~/.chatbot/`.
