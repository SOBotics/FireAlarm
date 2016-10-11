all:
	clang chatbot/*.c -lcurl -lwebsockets -lpthread -lm -o firealarm
