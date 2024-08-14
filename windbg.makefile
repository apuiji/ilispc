include Makefile

DEST = debug
TARGET = libilispc.a

${DEST}/${TARGET}: $(addprefix ${DEST}/, ${OBJS})
	ar -cr $@ $^

${DEST}/%.o: %.cc ${HHS}
	clang++ -c -O2 -o $@ -std=c++2b -stdlib=libc++ $<

clean:
	echo>${DEST}\${TARGET}
	echo>${DEST}\a.o
	del ${DEST}\${TARGET}
	del ${DEST}\*.o
