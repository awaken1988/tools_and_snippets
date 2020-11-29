qemu-system-arm -M vexpress-a9 \
                -m 32M -no-reboot -nographic \
                -s -S \
                -kernel out/program.elf