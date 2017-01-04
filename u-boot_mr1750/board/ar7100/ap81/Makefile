include $(TOPDIR)/config.mk

LIB	= lib$(BOARD).a

ifeq ($(CONFIG_AP81_CARRIER_CUS109), 1)
OBJS	= $(BOARD).o flash.o ../common/ar7100_flash.o ../common/athrf1_phy.o
else
OBJS	= $(BOARD).o flash.o ../common/ar7100_flash.o ../common/athrs26_phy.o
endif

SOBJS	= ../common/lowlevel_init_ar9100.o

$(LIB):	.depend $(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
