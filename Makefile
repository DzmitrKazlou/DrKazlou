########################################################################

ROOTINCL  := $(shell root-config --incdir)
ROOTLIB  := $(shell root-config --libdir)
#ROOTSYS =/home/kazlou/Downloads/ROOT/root/

LIBS          := -L -lpthread -lutil -lusb-1.0 -L $(ROOTLIB) -lGeom -lGed -lTreePlayer -lCore -lHist -lGraf -lGraf3d -lMathCore -lGpad -lTree -lRint -lRIO -lPostscript -lMatrix -lPhysics -lMinuit -lGui -lASImage -lASImageGui -pthread -lm -ldl -rdynamic -lstdc++ -lX11
#CFLAGS        = -g -O2 -Wall -Wuninitialized -fno-strict-aliasing -I./include -I/usr/local/include -I $(ROOTINCL) -DLINUX -fPIC -lCAENDigitizer
#CXXFLAGS        = -std=c++17 -g -O2 -Wall -Wuninitialized -fno-strict-aliasing -I./include -I/usr/local/include -I $(ROOTINCL) -DLINUX -fPIC -lCAENDigitizer


#CFLAGS        += -stdlib=libstdc++
  


CXX           = g++ -g -fPIC -DLINUX -O2 -I./include

#CC           = gcc -fPIC -DLINUX -O2 -I./include

#CXXFLAGS     += $(ROOTCFLAGS)


.PHONY: all clean
	
all: clean MTCRoot
	
clean:
		@rm -rf MTCRoot *.o *.cxx *.so

guiDict.cxx : include/MTCRoot.h include/MTCFrame.h include/MTCLogic.h include/MTCParams.h include/MTCOpt.h include/LinkDef.h
	@rootcling -f $@ $^
	$(info [-10%]  Dictionary)	


MTCRoot: MTCFunc.o MTCReadout.o MTCOpt.o MTCLogic.o MTCParams.o MTCFrame.o MTCRoot.o     
		$(info [70%]  Linking)
		@$(CXX) -o MTCRoot src/MTCRoot.o src/MTCFrame.o src/MTCLogic.o src/MTCParams.o src/MTCOpt.o src/MTCReadout.o src/MTCFunc.o guiDict.cxx 	 $(LIBS)  -lCAENDigitizer -lCAENComm -lCAENVME `root-config --cflags --glibs` 
		$(info [100%] Built target MTCRoot)
		
MTCFunc.o:  src/MTCFunc.c
		$(info [9%] Generating MTCFunc.o)
		@$(CXX) -o src/MTCFunc.o -c src/MTCFunc.c	-lCAENDigitizer -lCAENComm -lCAENVME	`root-config --cflags --glibs`			 

MTCReadout.o:  src/MTCReadout.c
		$(info [13%] Generating MTCReadout.o)
		@$(CXX) -o src/MTCReadout.o -c src/MTCReadout.c	`root-config --cflags --glibs`			 

MTCOpt.o:  src/MTCOpt.c guiDict.cxx
		$(info [20%] Generating MTCOpt.o)
		@$(CXX) -o src/MTCOpt.o -c src/MTCOpt.c	 `root-config --cflags --glibs`			

MTCLogic.o:  src/MTCLogic.c guiDict.cxx
		$(info [30%] Generating MTCLogic.o)
		@$(CXX) -o src/MTCLogic.o -c src/MTCLogic.c	 `root-config --cflags --glibs`			
		
MTCParams.o:  src/MTCParams.c guiDict.cxx
		$(info [40%] Generating MTCParams.o)
		@$(CXX) -o src/MTCParams.o -c src/MTCParams.c	 `root-config --cflags --glibs`					
		
MTCFrame.o:  src/MTCFrame.c guiDict.cxx
		$(info [50%] Generating MTCFrame.o)
		@$(CXX) -o src/MTCFrame.o -c src/MTCFrame.c	 `root-config --cflags --glibs`					
		
MTCRoot.o: src/MTCRoot.c guiDict.cxx
		$(info [60%] Generation MTCRoot.o)
		@$(CXX) -o src/MTCRoot.o -c src/MTCRoot.c `root-config --cflags --glibs`
		
