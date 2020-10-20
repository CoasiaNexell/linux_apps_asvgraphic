######################################################################
# Build	options
INCLUDE	= -I./	-I./import/3rdparty/include/khronos/ -I./import/include/
INCLUDE += -I$(SDKTARGETSYSROOT)/usr/include/libdrm
LIBRARY	+= -lMali -lEGL -lGLESv1_CM  -ldl -lm 
LIBRARY	+= -lstdc++	-lpthread -lkms -ldrm -ldrm_nexell -lnx_renderer -lnx_drm_allocator

######################################################################
# Target
COBJS	:=
CPPOBJS	:= OGLEarthSphere.o ES_texture.o V6_sphere_980.o V6_sphere_9680.o Fake_OS.o GLFont.o __font_modify_tga.o main.o
TARGET	:= asv_graphic

CFLAGS += -Wno-narrowing
CXXFLAGS += -Wno-narrowing

######################################################################
# Build
OBJS	:= $(COBJS)	$(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(CXXFLAGS) $(OBJS) -o $@	$(LIBRARY)

clean:
	@find $(OBJTREE) -type f \
		\( -name '*.o'	-o -name $(TARGET) -o -name	'*.map'	\) -print \
		| xargs	rm -f
	rm -f dependency_list_$(TARGET)

#########################################################################
# Dependency
ifeq (dependency_list_$(TARGET),$(wildcard dependency_list_$(TARGET)))
include	dependency_list_$(TARGET)
endif

SRCS :=	$(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS :=	$(INCLUDE)
depend dep:
	$(CC)  -M  $(CXXFLAGS) $(CFLAGS) $(INCS) $(INCLUDE)	$(SRCS)	> dependency_list_$(TARGET)


#########################################################################
# 	Generic Rules
#########################################################################
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<
