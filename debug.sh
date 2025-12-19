#!/bin/bash

# Simple OpenOCD + GDB debugging script for STM32F411

echo "=== STM32F411 OpenOCD Debug Script ==="
echo ""

# Check if OpenOCD is installed
if ! command -v openocd &> /dev/null; then
    echo "❌ OpenOCD not found! Install with: sudo apt install openocd"
    exit 1
fi

# Check if GDB is installed
if ! command -v arm-none-eabi-gdb &> /dev/null; then
    echo "❌ arm-none-eabi-gdb not found! Install ARM toolchain"
    exit 1
fi

# Check if ELF file exists
if [ ! -f "build/ILI9341_stm32f411.elf" ]; then
    echo "❌ ELF file not found! Run 'make' first"
    exit 1
fi

echo "✅ Starting OpenOCD..."
openocd -f openocd.cfg &
OPENOCD_PID=$!

# Wait for OpenOCD to start
sleep 2

echo ""
echo "✅ Starting GDB..."
echo "Commands:"
echo "  target remote localhost:3333"
echo "  monitor reset halt"
echo "  break main"
echo "  continue"
echo ""

arm-none-eabi-gdb build/ILI9341_stm32f411.elf -ex "target remote localhost:3333" -ex "monitor reset halt"

echo ""
echo "=== Debug session ended ==="

# Kill OpenOCD
kill $OPENOCD_PID 2>/dev/null
