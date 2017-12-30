TOOLCHAIN_BASE := ./
MODULES_BASE := ../../

#EXTRA_INCLUDES := -I ../../modules/modules_1_5_4/include/lwip -I ../../modules/modules_1_5_4/include/lwip/ipv4 -I ../../modules/modules_1_5_4/include/lwip/ipv6
SDK_CLOSED := ./bsp/esp_1_5_4/
SDK_BASE := ../../modules/modules_1_5_4/
CUSTOM_LIBS := 
LIBS :=  c gcc gcov hal pp phy net80211 lwip_p wpa crypto ssl main driver
OPTIMIZE_FLAG =  -Os  ## ANY VALUE OVERWRITES -O2, ex: -c
CFLAGS := -DLWIP_OPEN_SRC



include ../../Makefile
