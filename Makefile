# -------------------------------------------------
# Variables setting
# -------------------------------------------------
SYSTEM = $(shell uname)
#OPT = release
OPT = debug
CXX = mpic++
TARGET = test_mpicall

INCDIR=	\
		-I./inc \
		-I/usr/local/include \
		-I/usr/mpi/gcc/openmpi-1.10.3rc4/include

SRCDIR=	\
		src

LIBDIR=	\
		-lcrypto -lpthread -lm -lstdc++

OBJDIR = obj.$(SYSTEM).$(OPT).$(CXX)
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)%.cpp,$(OBJDIR)%.o,$(SRC))
$(shell if [ ! -d ${OBJDIR} ]; then mkdir ${OBJDIR}; fi)

ifeq ($(CXX), mpic++)
	GCC = mpic++
endif

ifeq ($(OPT), release)
	#CPPFLAGS = -c -fpic -O3 -DNDEBUG  -DPPLM_FLOAT -DPYTHON_HEAD_FILE_INCLUDE
	CPPFLAGS = -O3 -fopenmp -mavx -m64 -D_GUN_SOURCE -std=c++11 -D_STRICT_FLOAT_COMPARISON  -Wfloat-equal -Wall -Winline --param large-function-growth=10000 -pipe
endif
ifeq ($(OPT), debug)
	#CPPFLAGS = -c -fpic -Ddebug -DNDEBUG  -DPPLM_FLOAT -DPYTHON_HEAD_FILE_INCLUDE
	CPPFLAGS = -O0 -g -Wall -D_DEBUG -std=c++11 -D_STRICT_FLOAT_COMPARISON -D_GUN_SOURCE -Wfloat-equal -Wall -Winline -pipe
endif

# -------------------------------------------------
# Code generation
# -------------------------------------------------
$(TARGET) : ${OBJ}
	$(GCC) -g -o $@ $^ $(LIBDIR) $(INCDIR)

${OBJDIR}/%.o : $(SRCDIR)/%.cpp
	$(GCC) $(CPPFLAGS) -g -c $< -o $@ $(INCDIR)

clean:
	rm -rf ${OBJDIR} $(TARGET)
