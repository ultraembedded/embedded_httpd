###############################################################################
## Simulator Makefile
###############################################################################

# Target
TARGET		= test

# Options
CFLAGS	    = -O2
LDFLAGS     = 
LIBS        = 

# Source Files
SRC_C = http_auth.c
SRC_C+= httpd.c
SRC_C+= http_input.c
SRC_C+= http_output.c
SRC_C+= http_request.c
SRC_C+= http_printf.c
SRC_C+= main.c

OBJ := $(patsubst %.c, %.o, $(SRC_C))

VPATH := ../../httpd:.
CFLAGS+= -I../../httpd

###############################################################################
# Rules
###############################################################################
all: $(TARGET)
	
clean:
	-rm *.o $(TARGET)

%.o : %.c
	gcc -c $(CFLAGS) $< -o $@

%.o : %.cpp
	g++ -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJ) makefile
	g++ $(LDFLAGS) $(OBJ) $(LIBS) -o $@

