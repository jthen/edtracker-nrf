TARGET   = edtracker.hex
CFLAGS   = --model-large -Imcu-lib -Iinvenlib -DNRF24LE1 -DEMPL_TARGET_NRF24LE1 -DDBG_MODE=0
#CFLAGS  += -DDBG_MODE=0
LFLAGS   = --code-loc 0x0000 --code-size 0x4000 --xram-loc 0x0000 --xram-size 0x400
ASFLAGS  = -plosgff
RELFILES = edtracker.rel inv_mpu.rel inv_mpu_dmp_motion_driver.rel nrf_shim.rel nrfdbg.rel nrfutils.rel nRF24L.rel crtxinit.rel

VPATH   = mcu-lib:invenlib

hex: $(TARGET)

$(TARGET): $(RELFILES)
	sdcc $(CFLAGS) $(LFLAGS) $(RELFILES) -o $(TARGET)
	grep "ROM/EPROM" keyboard.mem

%.rel: %.c
	sdcc $(CFLAGS) -c $<

%.rel: %.asm
	sdas8051 $(ASFLAGS) $@ $< 

clean:
	rm --force *.hex *.lnk *.lst *.map *.rel *.rst *.sym *.mem *.lk *.asm *.lk *.cdb *.omf

all: clean hex

flash: clean hex
	nrfburn -f 16 -w $(TARGET)
