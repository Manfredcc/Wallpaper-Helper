CFLAGS = -pthread
objs = wpHelperService.o wpHelper.o
wpHelperService : $(objs)
	g++ -o wpHelperService $(objs) $(CFLAGS)

wpHelperService.o :
	g++ -c wpHelperService.cpp -I ./json/include -I ./wpHelper $(CFLAGS)

wpHelper.o :
	g++ -c ./wpHelper/wpHelper.cpp $(CFLAGS)

.PHONY : clean
clean :
	rm wpHelperService $(objs)