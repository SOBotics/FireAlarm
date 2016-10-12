all:
	clang chatbot/*.c -lcurl -lwebsockets -lpthread -lm -o firealarm -ferror-limit=100

