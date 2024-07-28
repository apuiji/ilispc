include Makefile

DEST = windbg
TARGET = ilispc.exe

${DEST}/${TARGET}: $(addprefix ${DEST}/, ${OBJS})
	clang++ -o $@ $^ -stdlib=libc++

${DEST}/%.o: %.cc ${HHS}
	clang++ -c -O2 -o $@ -std=c++2b -stdlib=libc++ $<

clean:
	echo>${DEST}\${TARGET}
	echo>${DEST}\a.o
	del ${DEST}\${TARGET}
	del ${DEST}\*.o
