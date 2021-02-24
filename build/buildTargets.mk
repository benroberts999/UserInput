# All makefile targets

#Default make rule:
all: checkObj checkXdir $(XD)/main

################################################################################
# Automatically generate dependency files for each cpp file, + compile:
# I make all files depend on '$(SD)/git.info'. This achieves two things:
# (1) It forces git.info to be built first (even in a parallel build)
# (2) It forces a clean make when changing branches
# The '|' means 'order only' pre-req

# uncomment if there are any sub-directories
# $(BD)/%.o: $(SD)/*/%.cpp
# 	$(COMP)

$(BD)/%.o: $(SD)/%.cpp
	$(COMP)

-include $(BD)/*.d

################################################################################
# Link + build all final programs

# List all objects in sub-directories (i.e., that don't conatin a main())
OBJS = $(addprefix $(BD)/,$(notdir $(subst .cpp,.o,$(wildcard $(SD)/*/*.cpp))))

$(XD)/main: $(BD)/main.o $(OBJS)
	$(LINK)

################################################################################
################################################################################
.PHONY: clean doxy checkObj checkXdir

checkObj:
	@if [ ! -d $(BD) ]; then \
	  echo '\n ERROR: Directory: '$(BD)' doesnt exist - please create it!\n'; \
	  false; \
	fi

checkXdir:
	@if [ ! -d $(XD) ]; then \
		echo '\n ERROR: Directory: '$(XD)' doesnt exist - please create it!\n'; \
		false; \
	fi

clean:
	rm -f $(ALLEXES) $(BD)/*.o $(BD)/*.d

doxy:
	doxygen ./src/Doxyfile
