# FLAGS

DEBUG = -Wall -Wextra

FLAGS = -lpthread -pthread

CC = g++


all: NPSimulator

NPSimulator: ./src/*
	$(CC) -g -o sim $(FLAGS) ./src/main.c #$(DEBUG)

