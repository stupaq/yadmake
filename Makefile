# versatile Makefile
CXX			:= g++
CXXFLAGS	:= -Wall -Wextra -pedantic -std=gnu++0x -lboost_iostreams
LDFLAGS		:= -Wall -lboost_iostreams

# sources
SOURCES		:= $(shell find -name "*.cpp")
# objects containing main() definition
MAINOBJECTS	:= $(subst .cpp,.o,$(shell grep -l "int main" $(SOURCES)))
# executables (linked from MAINOBJECTS)
ALL			:= $(subst .o,,$(MAINOBJECTS))
# submakefiles
DEPENDS		:= $(subst .cpp,.d,$(SOURCES))
# all objects
ALLOBJECTS	:= $(subst .cpp,.o,$(SOURCES))
# objects not containing main() definition
OBJECTS		:= $(filter-out $(MAINOBJECTS),$(ALLOBJECTS)) 

all: $(DEPENDS) $(ALL)

# create submakefiles
$(DEPENDS) : %.d : %.cpp
	$(CXX) $(CXXFLAGS) -MM $< > $@
	@echo -e "\t"$(CXX) $(CXXFLAGS) -c $(CFLAGS) $< >> $@

# link objects
$(ALL) : % : %.o $(OBJECTS)
	$(CXX) -s $(LDFLAGS) -o $@ $^

# include submakefiles
-include $(DEPENDS)

clean:
	-rm -f *.o *~ $(ALL) $(ALLOBJECTS) $(DEPENDS)

debug:
	@echo $(OBJECTS)

