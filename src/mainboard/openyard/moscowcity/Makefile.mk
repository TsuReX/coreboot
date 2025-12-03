## SPDX-License-Identifier: GPL-2.0-only

bootblock-y += bootblock.c
romstage-y += romstage.c
romstage-y += ht_config.c
romstage-y += iio_config.c
romstage-y += sata_config.c
romstage-y += oysetup_config.c
ramstage-y += ramstage.c
CPPFLAGS_common += -I$(src)/mainboard/$(MAINBOARDDIR)/include
