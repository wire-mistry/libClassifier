CC=gcc

all:main.o stringVals.o
	$(CC) -g -o LibInfoTool main.o stringVals.o
main.o:
	$(CC) -g -c  main.c
stringVals.o:
	$(CC) -g -c  stringVals.c

.PHONY:clean
clean:
	rm -rf LibInfoTool main.o stringVals.o
