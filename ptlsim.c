#include <exec/address-spaces.h>
#include <exec/memory.h>

uint64_t ptlsim_mmio_read(void *opaque, hwaddr addr, unsigned size);
void ptlsim_mmio_write(void *opaque, hwaddr addr, uint64_t data, unsigned size);

const MemoryRegionOps ptlsim_mmio_ops = {
  .read = ptlsim_mmio_read,
  .write = ptlsim_mmio_write,

  .endianness = DEVICE_NATIVE_ENDIAN,

  /* Don't bother to support the "old" callback format. */
  .old_mmio = {{NULL, NULL, NULL}, {NULL, NULL, NULL}}
};

