#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

# NitroFS directory MUST be set before including ds_rules
NITRO_FILES	:=	$(CURDIR)/nitrofiles
export NITRO_FILES
NITRODIR	:=	$(CURDIR)/nitrofiles
export NITRODIR

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
TARGET		:=	dscraft3d
BUILD		:=	build
SOURCES		:=	source
DATA		:=	data
INCLUDES	:=	include
NITRO_FILES	:=	$(CURDIR)/nitrofiles
export NITRO_FILES

#---------------------------------------------------------------------------------
ARCH		:=	-march=armv5te -mtune=arm946e-s -mthumb
CFLAGS		:=	-g -Wall -O2 -ffunction-sections -fdata-sections $(ARCH) -D__NDS__ -DARM9
CFLAGS		+=	$(INCLUDE)
CXXFLAGS	:=	$(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS		:=	-g $(ARCH)
LDFLAGS		=	-specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

# Libraries
LIBS		:= 	-Wl,--start-group -lfat -lcalico_ds9 -lnds9 -lm -Wl,--end-group
LIBDIRS		:=	$(LIBNDS) $(DEVKITPRO)/calico $(DEVKITPRO)/libfat

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds

#---------------------------------------------------------------------------------
else

$(OUTPUT).nds	: 	$(OUTPUT).elf
$(OUTPUT).elf	:	$(OFILES)

-include $(DEPSDIR)/*.d

#---------------------------------------------------------------------------------
endif
