# mmio

Read and write /dev/mem device

## Example Usage

```bash
# Read 32-bits at 0x11000000
# Equivalent to 'print *(unsigned int)0x11000000'
mmio load 32 0x11000000

# Write 16-bits at 0x11000000
# Equivalent to '*(unsigned short)0x30000e0 = 0x2f'
mmio store 16 0x30000e0 0x2f

# Set 32-bits flags 0x10 at 0x3000180
# Equivalent to '*(unsigned int)0x3000180 = 0x10 | *(unsigned int)0x3000180'
mmio set 32 0x3000180 0x10

# Pretty print memory at 0x44000000 with length 128 bytes
mmio read 0x44000000 128 | hexdump -C

# Load file into memory at 0x81000000
mmio write 0x81000000 0x1000 mem.bin
```

## Example Service

```ini
[Unit]
Description=Tell CPLD we are up
DefaultDependencies=no

[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=mmio set 8 0x8000004d 0x20

[Install]
WantedBy=sysinit.target
```
