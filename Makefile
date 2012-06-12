# versatile Makefile
CXX			:= g++ -I include
CXXFLAGS	:= -Wall -Wextra -pedantic -std=gnu++0x
LDFLAGS		:= -Wall -lboost_iostreams -lboost_unit_test_framework -lssh

# sources
SOURCES		:= $(shell find -name "*.cc")
# objects containing main() definition
MAINOBJECTS	:= $(subst .cc,.o,$(shell grep -l "int main" $(SOURCES)))
# executables (linked from MAINOBJECTS)
ALL			:= $(subst .o,,$(MAINOBJECTS))
# submakefiles
DEPENDS		:= $(subst .cc,.d,$(SOURCES))
# all objects
ALLOBJECTS	:= $(subst .cc,.o,$(SOURCES))
# objects not containing main() definition
OBJECTS		:= $(filter-out $(MAINOBJECTS),$(ALLOBJECTS)) 

all: $(DEPENDS) $(ALL)

# create submakefiles
$(DEPENDS) : %.d : %.cc
	$(CXX) $(CXXFLAGS) -MT $(<:.cc=.o) -MM $< > $@
	@echo -e "\t"$(CXX) $(CXXFLAGS) -c $(CFLAGS) $< -o $(<:.cc=.o) >> $@

# link objects
$(ALL) : % : %.o $(OBJECTS)
	$(CXX) -s $(LDFLAGS) -o $@ $^

# include submakefiles
-include $(DEPENDS)

clean:
	-rm -f *.o $(ALL) $(ALLOBJECTS) $(DEPENDS)
	-rm -rf gendocs/*

gendocs:
	doxygen doxygen.conf

.PHONY: gendocs
