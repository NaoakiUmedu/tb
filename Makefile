CC		=	g++
CFLAGS	=	-DDEBUG
TARGET	=	tb
SRCS	=	tb.cpp
OBJS	=	$(SRCS:.cpp=.o)
INCDIR	=	-I./
LIBDIR	=
LIBS	=

$(TARGET):	$(OBJS)
#	$(CC) -o $@ $^ $(CFLAGS) $(LIBDIR) $(LIBS)
	$(CC) -o $(TARGET) $(SRCS) $(CFLAGS) $(LIBDIR) $(LIBS)

all: clean $(OBJS) $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) *.d
