# FLAGS

DEBUG = -Wall -Wextra

FLAGS = -lpthread -pthread

CC = g++


all: NPSimulator

NPSimulator: ./src/*
	$(CC) -o NPSimulator $(FLAGS) ./src/main.c #$(DEBUG)





