SRC = regcomp.c regerror.c regexec.c regfree.c 
OBJ = $(SRC:.c=.o)
INCLUDES = -I. -I..

CFLAGS := -O2 -D_FORTIFY_SOURCE=2
CFLAGS += -fPIC

all: $(REGEX_LIB)

%.o: %.c
ifeq ($(VERBOSE),0)
	@echo "[CC] $<"
endif
	$(MAKECMDPREFIX)$(CC) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(REGEX_LIB): $(OBJ)
ifeq ($(VERBOSE),0)
	@echo "[AR] $<"
endif
	$(MAKECMDPREFIX)$(AR) rcs $(REGEX_LIB) $(OBJ)

clean:
	rm -f $(OBJ) $(REGEX_LIB)
