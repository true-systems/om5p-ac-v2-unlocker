
include $(TOPDIR)/config.mk
LIB = lib$(BOARD).a
OBJS = $(BOARD).o ../common/ar7100_flash.o ../common/generic_i2c.o ../common/generic_spi.o ../common/vsc73xx.o ../common/vsc8601_phy.o pb44_pci.o
SOBJS = lowlevel_init.o

$(LIB): .depend $(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

.depend: Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
	$(CC) -M $(CFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

