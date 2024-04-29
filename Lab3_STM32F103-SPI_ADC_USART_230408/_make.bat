xmake
pause
if exist "C:/Program Files (x86)/STMicroelectronics/STM32 ST-Link Utility/ST-Link Utility/ST-LINK_CLI.exe" "C:/Program Files (x86)/STMicroelectronics/STM32 ST-Link Utility/ST-Link Utility/ST-LINK_CLI.exe" -c SWD -ME -P "target/target.bin" 0x08000000 -V -Rst

if exist "C:/Program Files/STMicroelectronics/STM32 ST-Link Utility/ST-Link Utility/ST-LINK_CLI.exe" "C:/Program Files/STMicroelectronics/STM32 ST-Link Utility/ST-Link Utility/ST-LINK_CLI.exe" -c SWD -ME -P "target/target.bin" 0x08000000 -V -Rst

del target\target.elf, target\target.lss, target\target.map, target\target.sym
