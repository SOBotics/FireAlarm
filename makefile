all:
	gcc chatbot/*.c -lcurl -lwebsockets -lpthread -lm -o firealarm
