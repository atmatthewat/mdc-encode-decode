
mdc_test:	mdc_test.c mdc_decode.o mdc_encode.o
		cc -g -o mdc_test mdc_test.c mdc_decode.o mdc_encode.o
		./mdc_test

mdc_decode.o:	mdc_decode.c mdc_decode.h mdc_common.c
		cc -c mdc_decode.c

mdc_encode.o:	mdc_encode.c mdc_encode.h mdc_common.c
		cc -c mdc_encode.c

clean:
	rm -f mdc_decode.o mdc_encode.o mdc_test
	
