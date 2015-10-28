bulk_loading.out : am.o amfns.o amsearch.o aminsert.o amstack.o amglobals.o misc.o  ../pflayer/pflayer.o insert.o amscan.o amprint.o
	cc -o bulk_loading.out am.o amfns.o amsearch.o aminsert.o  amstack.o amglobals.o misc.o ../pflayer/pflayer.o insert.o amscan.o amprint.o

normal_insertion.out : am.o amfns.o amsearch.o aminsert.o amstack.o amglobals.o misc.o  ../pflayer/pflayer.o main.o amscan.o amprint.o
	cc -o normal_insertion.out am.o amfns.o amsearch.o aminsert.o  amstack.o amglobals.o misc.o  ../pflayer/pflayer.o main.o amscan.o amprint.o

amlayer.o : am.o amfns.o amsearch.o aminsert.o amstack.o amglobals.o amscan.o amprint.o
	ld -r am.o amfns.o amsearch.o aminsert.o  amstack.o amglobals.o amscan.o amprint.o  -o amlayer.o

am.o : am.c am.h pf.h
	cc -c am.c

amfns.o : amfns.c am.h pf.h
	cc -c amfns.c

amsearch.o : amsearch.c am.h pf.h
	cc -c amsearch.c

aminsert.o : aminsert.c am.h pf.h
	cc -c aminsert.c

amscan.o : amscan.c am.h pf.h
	cc -c amscan.c

amstack.o : amstack.c am.h pf.h
	cc -c amstack.c

amglobals.o : amglobals.c am.h
	cc -c amglobals.c


amprint.o : amprint.c am.h pf.h 
	cc -c amprint.c
	
main.o : main.cpp am.h pf.h
	cc -c main.cpp
misc.o : misc.c am.h pf.h
	cc -c misc.c


insert.o : insert.c am.h pf.h
	c99 -c insert.c

clean: 
	rm -f *.o
	rm -f *.out
