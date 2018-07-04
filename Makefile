objs = main.o uart.o at_parse.o 
srcs = main.c uart.c at_parse.c
parse : ${objs}
	gcc  /usr/local/lib/libserialport.dylib  main.o uart.o at_parse.o    -o parse
	./parse
${objs}: ${srcs}
	gcc -c ${srcs}
.PHONY:clean exe pre
clean:
	rm parse ${objs}
exe:
	./parse
pre:
	gcc -E main.c
