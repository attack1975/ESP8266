TOOLCHAIN_BASE := ./
MODULES_BASE := ../../

SDK_CLOSED := ./bsp/esp_1_5_0/
SDK_BASE := ../../modules/modules_1_5_0/
CUSTOM_LIBS := 
LIBS :=  c gcc gcov hal pp phy net80211 lwip_p wpa crypto ssl main driver
OPTIMIZE_FLAG =  -Os  ## ANY VALUE OVERWRITES -O2, ex: -c
CFLAGS := -DLWIP_OPEN_SRC

include ../../Makefile
