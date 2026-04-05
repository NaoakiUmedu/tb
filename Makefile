CC		=	g++
CFLAGS	=
TARGET	=	tb
SRCS	=	tb.cpp
OBJS	=	$(SRCS:.cpp=.o)
INCDIR	=	-I./
LIBDIR	=
LIBS	=

$(TARGET):	$(OBJS)
	$(CC) -o $@ $^ $(LIBDIR) $(LIBS)

all: clean $(OBJS) $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) *.d
