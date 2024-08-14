include Makefile

DEST = debug
TARGET = libilispc.a

${DEST}/${TARGET}: $(addprefix ${DEST}/, ${OBJS})
	ar -cr $@ $^

${DEST}/%.o: %.cc ${HHS}
	clang++ -c -O2 -o $@ -std=c++2b -stdlib=libc++ $<

clean:
	touch ${DEST}/${TARGET} ${DEST}/a.o
	rm ${DEST}/${TARGET} ${DEST}/*.o
