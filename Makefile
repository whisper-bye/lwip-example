CCDEP=$(CC)

#CFLAGS += -DLWIP_DEBUG
ARFLAGS=rs

TOPDIR=.
TARGETDIR=$(TOPDIR)/tmplib
CONTRIBDIR=$(TOPDIR)/lwip/contrib
LWIPARCH=$(TOPDIR)

#Set this to where you have the lwip core module checked out from CVS
#default assumes it's a dir named lwip at the same level as the contrib module
LWIPDIR=$(TOPDIR)/lwip/src
MCHINCDIR=$(TOPDIR)/include

CFLAGS += $(CPPFLAGS) -I$(LWIPDIR)/include -I.              \
	-I$(LWIPARCH)/include/lwip -I$(LWIPARCH)/include/arch        \
	-I$(LWIPDIR)/include/ipv4 -I$(MCHINCDIR) -I$(LWIPDIR)/include/ipv6

# COREFILES, CORE4FILES: The minimum set of files needed for lwIP.
COREFILES=$(LWIPDIR)/core/mem.c             \
    $(LWIPDIR)/core/memp.c              \
    $(LWIPDIR)/core/netif.c             \
    $(LWIPDIR)/core/pbuf.c              \
    $(LWIPDIR)/core/raw.c               \
    $(LWIPDIR)/core/stats.c             \
    $(LWIPDIR)/core/sys.c               \
    $(LWIPDIR)/core/tcp.c               \
    $(LWIPDIR)/core/tcp_in.c            \
    $(LWIPDIR)/core/tcp_out.c           \
    $(LWIPDIR)/core/timers.c            \
    $(LWIPDIR)/core/udp.c               \
    $(LWIPDIR)/core/dhcp.c              \
    $(LWIPDIR)/core/init.c              \
    $(LWIPDIR)/core/inet_chksum.c

CORE4FILES=$(LWIPDIR)/core/ipv4/icmp.c          \
    $(LWIPDIR)/core/ipv4/ip4.c           \
    $(LWIPDIR)/core/ipv4/ip4_addr.c          \
    $(LWIPDIR)/core/ipv4/ip_frag.c

# NETIFFILES: Files implementing various generic network interface functions.'
NETIFFILES=$(LWIPDIR)/netif/etharp.c

MAIN_FILES=$(LWIPARCH)/mch_main.c       \
	$(LWIPARCH)/src/sys_arch.c


# LWIPFILES: All the above.
LWIPFILES=$(MAIN_FILES) $(COREFILES) $(CORE4FILES) $(NETIFFILES)
OBJS=$(LWIPFILES:.c=.o)

LWIPMAIN=test_lwip

all compile: $(LWIPMAIN)
	mkdir -p $(TARGETDIR)
#	install $(LWIPLIB) $(TARGETDIR)

.PHONY: all depend compile clean

%.o:
	$(CC) $(CFLAGS) -c $(@:.o=.c) -o $@

clean:
	rm -f *.o $(LWIPLIB) $(LWIPMAIN) .depend*
	find . -name \*.o |xargs --no-run-if-empty rm

$(LWIPMAIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $(LWIPMAIN) $?

depend dep: .depend

include .depend

.depend: $(LWIPFILES)
	$(CCDEP) $(CFLAGS) -MM $^ > .depend || rm -f .depend
