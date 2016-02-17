TARGET		= psxnet
SOURCES		= ./

CC			= gcc
CXX			= g++

CFLAGS		= -Wall
CXXFLAGS	= $(CFLAGS)

INCLUDE		=

LIBS		=
LIBDIRS		=

CFILES		= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

OFILES		= $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)

all: $(OFILES)
	$(CXX) $(OFILES) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OFILES) $(TARGET)
