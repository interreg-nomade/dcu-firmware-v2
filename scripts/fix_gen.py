import re

## Fix the issue with the SD Transceiver
# CubeMX Generates it eventhough we need it set to 0. 
transceiverPath = '../Inc/stm32h7xx_hal_conf.h'
transceiverLineRead  = "#define  USE_SD_TRANSCEIVER           1U               /*!< use uSD Transceiver */\n"
transceiverLineWrite = "#define  USE_SD_TRANSCEIVER           0U               /*!< use uSD Transceiver */\n"

sdDmaPath = '../Src/sd_diskio.c'
sdDmaLineRead = "/* #define ENABLE_SD_DMA_CACHE_MAINTENANCE  1 */\n"
sdDmaLineWrite = "#define ENABLE_SD_DMA_CACHE_MAINTENANCE  1\n"



import fileinput
import sys

def replaceAll(file,searchExp,replaceExp):
    res = False
    for line in fileinput.input(file, inplace=1):
        if searchExp in line:
            line = line.replace(searchExp,replaceExp)
            res = True
        sys.stdout.write(line)
    return res

result = replaceAll(transceiverPath, transceiverLineRead, transceiverLineWrite)

if (result):
    print("Found the string: " + transceiverLineRead)
    print("And replaced with: "+ transceiverLineWrite)
else:
    print("Did not find the string: " + transceiverLineRead)


result = replaceAll(sdDmaPath, sdDmaLineRead, sdDmaLineWrite)
