#include "CChip.hpp"
#include "CNimble.h"

#include "CNonVolatileStorage.hpp"




void print_chip_info()
{
	sys::CChip chip{};

	std::printf("\nChip information");
	std::printf("\nRevision: %s", chip.revision().c_str());
	std::printf("\nNumber of Cores: %i", chip.cores());
	std::printf("\nFlash Memory: %s", chip.embedded_psram() ? "Embedded" : "External");
	std::printf("\nPSRAM: %s", chip.embedded_flash_memory() ? "Embedded" : "External");
	std::printf("\nSupports Wifi 2.4ghz: %s", chip.wifi() ? "True" : "False");
	std::printf("\nSupports Bluetooth LE: %s", chip.bluetooth_le() ? "True" : "False");
	std::printf("\nSupports Bluetooth Classic: %s", chip.bluetooth_classic() ? "True" : "False");
	std::printf("\nSupports IEEE 802.15.4: %s", chip.IEEE_802_15_4() ? "True" : "False");
	std::printf("\nCurrent minimum free heap: %lu bytes\n\n", sys::min_free_heap());
}

void test_nimble() 
{

	// server
	// server has: nvs, nimble
	// nimble has host/controller
	// host has gat and gapp
	// CNimble
	// CNimbleHost
	// CNimbleHostGap -> connections, bonding etc
	// CNimbleHostGatt -> services, data transfer etc

	// nimble has host
	// host has gap and gatt

nimble::CNimble ble {};

}

extern "C" void app_main(void)

{
	print_chip_info();
	storage::CNonVolatileStorage nvs {};

	test_nimble();
	//esp_err_t ret = nimble_port_init();
}
