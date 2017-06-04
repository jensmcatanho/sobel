CC = gcc
CFLAGS = -lm -I.
OBJS = utils.o neryimg.o
TARGETPROC = sobel-process
TARGETTHREAD = sobel-thread
TARGETOMP = sobel-omp
TARGETS = ${TARGETPROC} ${TARGETTHREAD} ${TARGETOMP}

all: ${TARGETS}

neryimg.o:
	${CC} ${CFLAGS} lib/neryimg.c -c
	
utils.o:
	${CC} ${CFLAGS} lib/utils.c -c
	
${TARGETPROC}: ${TARGETPROC}.c ${OBJS}
	${CC} -o ${TARGETPROC} ${TARGETPROC}.c ${OBJS} ${CFLAGS}

${TARGETTHREAD}: ${TARGETTHREAD} ${OBJS}
	${CC} -o ${TARGETTHREAD} ${TARGETTHREAD}.c ${OBJS} -lpthread ${CFLAGS}

${TARGETOMP}: ${TARGETOMP}.c ${OBJS}
	${CC} -o ${TARGETOMP} ${TARGETOMP}.c ${OBJS} -fopenmp ${CFLAGS}

.PHONY: clean

clean:
	rm ${TARGETS} *.o *~ lib/*~
