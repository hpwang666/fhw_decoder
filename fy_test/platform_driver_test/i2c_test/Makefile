include ../../system.mk

test:
	arm-mol-linux-uclibcgnueabihf-gcc i2c_eeprom_io.c -static -o i2c_eeprom
	cp i2c_eeprom $(APP_OUTPUT_DIR)

clean:
	rm -f *.o i2c_eeprom
