########################################################################

ROOTINCL  := $(shell root-config --incdir)
ROOTLIB  := $(shell root-config --libdir)
#ROOTSYS =/home/kazlou/Downloads/ROOT/root/

LIBS          := -L -lpthread -lutil -lusb-1.0 -L $(ROOTLIB) -lGeom -lRGL -lGed -lTreePlayer -lCore -lHist -lGraf -lGraf3d -lMathCore -lGpad -lTree -lRint -lRIO -lPostscript -lMatrix -lPhysics -lMinuit -lGui -lASImage -lASImageGui -pthread -lm -ldl -rdynamic -lstdc++
#CFLAGS        = -g -O2 -Wall -Wuninitialized -fno-strict-aliasing -I./include -I/usr/local/include -I $(ROOTINCL) -DLINUX -fPIC -lCAENDigitizer
#CXXFLAGS        = -std=c++17 -g -O2 -Wall -Wuninitialized -fno-strict-aliasing -I./include -I/usr/local/include -I $(ROOTINCL) -DLINUX -fPIC -lCAENDigitizer


#CFLAGS        += -stdlib=libstdc++
  


CXX           = g++ -fPIC -DLINUX -O2 -I./include

#CC           = gcc -fPIC -DLINUX -O2 -I./include

#CXXFLAGS     += $(ROOTCFLAGS)


.PHONY: all clean
	
all: clean MTCRoot
	
clean:
		@rm -rf MTCRoot *.o *.cxx *.so

guiDict.cxx : include/MTCRoot.h include/LinkDef.h
	@rootcling -f $@ $^
	$(info [-10%]  Dictionary)	


MTCRoot: MTCconfig.o  MTCFunc.o MTCRoot.o 
		$(info [70%]  Linking)
		@$(CXX) -o MTCRoot src/MTCRoot.o guiDict.cxx src/MTCFunc.o  src/MTCconfig.o	 $(LIBS) -lCAENDigitizer -lCAENComm -lCAENVME `root-config --cflags --glibs` 
		$(info [100%] Built target MTCRoot)
		

		
MTCconfig.o:  src/MTCconfig.c
		$(info [10%] Generating MTCconfig.o)
		@$(CXX) -o src/MTCconfig.o -c src/MTCconfig.c
		
MTCFunc.o:  src/MTCFunc.c
		$(info [20%] Generating MTCFunc.o)
		@$(CXX) -o src/MTCFunc.o -c src/MTCFunc.c		
		
		
MTCRoot.o: src/MTCRoot.c guiDict.cxx
		$(info [30%] Generation MTCRoot.o)
		@$(CXX) -o src/MTCRoot.o -c src/MTCRoot.c -lCAENDigitizer -lCAENComm -lCAENVME`root-config --cflags --glibs`
		
