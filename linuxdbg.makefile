include Makefile

DEST = linuxdbg
TARGET = ilispc

${DEST}/${TARGET}: $(addprefix ${DEST}/, ${OBJS})
	clang++ -o $@ $^ -stdlib=libc++

${DEST}/%.o: %.cc ${HHS}
	clang++ -c -O2 -o $@ -std=c++2b -stdlib=libc++ $<

clean:
	touch ${DEST}/${TARGET} ${DEST}/a.o
	rm ${DEST}/${TARGET} ${DEST}/*.o
