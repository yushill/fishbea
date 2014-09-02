CXX=g++

CPPFLAGS=-I.
CXXFLAGS=-g3 -Wall
LIBS=-lSDL -lSDL_image

SRCS=top.cc timeline.cc video.cc action.cc gallery.cc dmmap.cc epmap.cc expmap.cc slideroom.cc pebblemap.cc

OBJS=$(patsubst %.cc,$(BUILD)/%.o,$(SRCS))
PPIS=$(patsubst %.cc,$(BUILD)/%.i,$(SRCS))
DEPS=$(patsubst %.o,%.d,$(OBJS))

BUILD=build

EXE=$(BUILD)/exe

.PHONY: all clean
all: $(EXE)

$(OBJS):$(BUILD)/%.o:%.cc
	@mkdir -p `dirname $@`
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -o $@ -c $<

$(PPIS):$(BUILD)/%.i:%.cc
	@mkdir -p `dirname $@`
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -o $@ -E $<

$(EXE): $(OBJS)
	@mkdir -p `dirname $@`
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

clean:
	rm -Rf $(BUILD)

-include $(DEPS)
