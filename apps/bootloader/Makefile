all:
	cd ../.. ; patch --forward --input=upgrader/nrf_dfu_ble.patch nRF5_SDK_17.1.0_ddde560/components/libraries/bootloader/ble_dfu/nrf_dfu_ble.c ; cd -
	make -C secure_bootloader/pca10040_s132_ble/armgcc SDK_ROOT=$(PWD)/../../nRF5_SDK_17.1.0_ddde560 GITHUB_WORKSPACE=$(PWD)/../..

flash:
	make -C secure_bootloader/pca10040_s132_ble/armgcc SDK_ROOT=$(PWD)/../../nRF5_SDK_17.1.0_ddde560 GITHUB_WORKSPACE=$(PWD)/../.. flash

flash_softdevice:
	make -C secure_bootloader/pca10040_s132_ble/armgcc SDK_ROOT=$(PWD)/../../nRF5_SDK_17.1.0_ddde560 GITHUB_WORKSPACE=$(PWD)/../.. flash_softdevice

