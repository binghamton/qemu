
# SCons File to compile QEMU

from subprocess import call

Import('qemu_env')
Import('qemu_target')
Import('ptlsim_lib')
Import('plugins')
Import('ptlsim_inc_dir')
env = qemu_env
target = qemu_target

target_base_arch = "%s" % target['arch']
if target_base_arch == "x86_64":
    target_base_arch = "i386"

target_path = "%s/target-%s" % (env['source_path'], target_base_arch)
target_build_path = "%s/%s-softmmu" % (env['source_path'], target['arch'])
env['CPPPATH'].append("%s/include" % env['source_path'])
env['CPPPATH'].append([".", "..", target_path, env['source_path']])
env['CPPPATH'].append(target['dir'])
env['CPPPATH'].append(ptlsim_inc_dir)
env['CPPPATH'].append("%s/linux-headers" % env['source_path'])
env['CPPPATH'].append("/usr/include/pixman-1")
env.Append(CCFLAGS = "-MMD -MP -DNEED_CPU_H".split())
env.Append(CCFLAGS = '-DMARSS_QEMU ')
env.Append(CCFLAGS = '-DTARGET_NAME=\\\"i386\\\"')
env.Append(CCFLAGS = '-DTARGET_X86_64 ')

num_sim_cores = ARGUMENTS.get('c', 1)
env.Append(CCFLAGS = '-DNUM_SIM_CORES=%d' % int(num_sim_cores))

debug = ARGUMENTS.get('debug', 0)
if int(debug):
    env.Append(CCFLAGS = '-g')

    # If debugging level is 1 then do optimize
    if int(debug) == 1:
        env.Append(CCFLAGS = '-O')
else:
    env.Append(CCFLAGS = '-g3')
    env.Append(CCFLAGS = '-O3')
    env.Append(CCFLAGS = '-march=native')

google_perftools = ARGUMENTS.get('gperf', None)
if google_perftools != None:
    env.Append(LIBS = 'tcmalloc')
    env.Append(LIBPATH = google_perftools)
    env.Append(LINKFLAGS = '-Wl,-rpath')
    env.Append(LINKFLAGS = '-Wl,%s' % google_perftools)


error_str = "failure"

# Read in user configuration from 'default-config/*.mak' files
# First create config-devices.mak file
target_mak_file = "%s/%s-softmmu/config-devices.mak" % (env['source_path'],
        target['arch'])
src_mak_file = "%s/default-configs/%s-softmmu.mak" % (env['source_path'],
        target['arch'])
call("sh %s/scripts/make_device_config.sh %s %s" % (env['source_path'],
    target_mak_file, src_mak_file), shell=True)
# FIXME later - if config-devices.mak already exitsts then rename it..
call("mv %s.tmp %s" % (target_mak_file, target_mak_file), shell=True)

# Now read in the file and set env variables
t_mak = open(target_mak_file, 'r')
for line in t_mak.readlines():
    if line.startswith("CONFIG_") and \
            line.strip().endswith("y"):
        var = line.split('=')[0].split('_')[1:]
        var = '_'.join(var)
        var = var.lower()
        env[var] = True

# if compiling with g++ add -x c flag
if "g++" in env['CC'] :
    env.Append(CCFLAGS = '-x c'.split())

# If we add user space emulator support then 
# add user space compilation from QEMU's makefile

qemu_prog_name = "qemu-system-%s" % target['arch']

# Set helper CFLAGS
HELPER_CFLAGS = ""
if target['arch'] == "i386":
    HELPER_CFLAGS += " -fomit-frame-pointer"

env.Append(CPPFLAGS = "-D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 \
        -D_LARGEFILE_SOURCE")
env.Append(LIBS = "m")
env.Append(LIBS = "pixman-1")
env.Append(LIBS = "png")
env.Append(LIBS = "jpeg")
env.Append(LIBS = "aio")

#######################
# QObject
q_object_files = "qobject/qint.c qobject/qstring.c qobject/qdict.c qobject/qlist.c qobject/qfloat.c qobject/qbool.c "
q_object_files += " qobject/qjson.c qobject/json-lexer.c qobject/json-streamer.c qobject/json-parser.c util/qemu-openpty.c "
q_object_files += " qobject/qerror.c util/error.c util/fifo8.c util/hbitmap.c thread-pool.c util/uri.c "

#######################
# qom
qom_files = "qom/object.c qom/container.c qom/qom-qobject.c "
qom_files += " qom/cpu.c"

#######################
# oslib-obj
oslib_files = "util/osdep.c util/oslib-posix.c util/qemu-thread-posix.c"

#######################
# coroutines
coroutines = "qemu-coroutine.c qemu-coroutine-lock.c qemu-coroutine-io.c "
coroutines += "qemu-coroutine-sleep.c "
if env['coroutine'] == "ucontext":
    coroutines += "coroutine-ucontext.c "
elif env['coroutines'] == "sigaltstack":
    coroutines += "coroutine-sigalstack.c "
    #TODO : Windows support

#######################
# block-obj
block_files = "util/cutils.c util/cache-utils.c util/qemu-option.c util/module.c ui/qemu-pixman.c"
block_files += " nbd.c block.c blockjob.c util/aes.c util/qemu-progress.c util/qemu-sockets.c"
block_files += " util/qemu-config.c async.c hw/block/block.c "
if env['linux_aio'] == True:
    block_files += " linux-aio.c"

block_files += coroutines

block_files += " " + " ".join(Glob("block/*.c", strings=True))
block_files = block_files.replace("block/raw-win32.c", "")
block_files = block_files.replace("block/win32-aio.c", "")
block_files = block_files.replace("block/rbd.c", "")
block_files = block_files.replace("block/gluster.c", "")
block_files = block_files.replace("block/ssh.c", "")

if env['curl'] == False:
    block_files = block_files.replace("block/curl.c ", " ")
if not env['libiscsi']:
    block_files = block_files.replace("block/iscsi.c", "")

net_files = " ".join(Glob("net/*.c", strings=True))
# This is hack, we compile only for linux so we remove all other system based
# net files without any check.
for file in ['tap-win32.c', 'tap-bsd.c', 'tap-aix.c', 'tap-solaris.c',
        'tap-haiku.c']:
    net_files = net_files.replace('net/%s' % file, "")

if env['slirp'] != True:
    net_files = net_files.replace('net/slirp.c', '')
if env['vde'] != True:
    net_files = net_files.replace('net/vde.c', '')

#######################
# fsdev-obj
fsdev_files = " ".join(Glob("fsdev/*.c", strings=True))

#######################
# libqemu_common.a

comm_lib_env = env.Clone()

comm_files = "readline.c ui/console.c tcg-runtime.c ioport.c util/host-utils.c "
comm_hw_files = "core/irq.c i2c/core.c i2c/smbus.c i2c/smbus_eeprom.c nvram/eeprom93xx.c "
comm_hw_files += " scsi/scsi-disk.c block/cdrom.c scsi/scsi-generic.c scsi/scsi-bus.c "
comm_hw_files += " input/ps2.c core/qdev.c "

# List of files that are not going to used for x86[_64]
# max7310.c max111x.c wm8750.c twl92230.c tsc2005.c lm832x.c
# tmp105.c stellaris_input.c ads7846.c ssd0303.c ssd0323.c ds1338.c 
# ssi.c ssi-sd.c sd.c 

comm_files += " migration.c migration-tcp.c xbzrle.c tpm.c util/unicode.c "
comm_files += " qemu-char.c qemu-log.c qemu-io-cmds.c dump.c memory_mapping.c "
comm_files += " savevm.c util/cache-utils.c bt-host.c bt-vhci.c util/hexdump.c "
comm_files += " migration-exec.c migration-unix.c migration-fd.c"
comm_files += " block-migration.c blockdev.c blockdev-nbd.c ui/cursor.c "
comm_files += " util/qemu-error.c ui/input.c iohandler.c hw/block/hd-geometry.c "
comm_files += " util/bitmap.c util/bitops.c target-i386/arch_dump.c target-i386/arch_memory_mapping.c "

# We only add POSIX/Linux related files
comm_files += " os-posix.c "

# USB Files
comm_hw_files += " usb/core.c "
comm_hw_files += " usb/dev-hub.c usb/dev-hid.c usb/desc.c "
comm_hw_files += " usb/dev-wacom.c usb/dev-serial.c "
comm_hw_files += " usb/dev-network.c usb/bus.c usb/dev-audio.c "
comm_hw_files += " usb/host-%s.c" % env['usb']
comm_hw_files += " usb/dev-bluetooth.c"

comm_libs = ""
comm_ldflags = ""
if env['brlapi']:
    comm_hw_files += " baum.c"
    comm_libs += " brlapi"

audio_files = "audio.c noaudio.c wavaudio.c mixeng.c"
audio_pt = False
audio_pt_int = False
if env['sdl'] == True:
    audio_files += " sdlaudio.c"
if 'oss' in env and env['oss']:
    audio_files += " ossaudio.c"
if 'coreaudio' in env and env['coreaudio']:
    audio_files += " coreaudio.c"
if 'alsa' in env and env['alsa']:
    audio_files += " alsaaudio.c"
if 'dsound' in env and env['dsound']:
    audio_files += " dsoundaudio.c"
if 'fmod' in env and env['fmod']:
    audio_files += " fmodaudio.c"
if 'esd' in env and env['esd']:
    audio_pt = True
    audio_pt_int = True
    audio_files += " esdaudio.c"
if 'pa' in env and env['pa']:
    audio_pt = True
    audio_pt_int = True
    audio_files += " paaudio.c"

if audio_pt:
    comm_ldflags = "-pthread"
if audio_pt_int:
    audio_files += " audio_pt_int.c"

audio_files += " wavcapture.c"
audio_list = []
for a_f in Split(audio_files):
    audio_list.append("audio/%s" % a_f)

ui_files = " keymaps.c"
if env['sdl'] == True:
    ui_files += " sdl.c sdl_zoom.c x_keymap.c"
    comm_lib_env.ParseConfig("sdl-config --cflags")
if env['curses'] == True:
    ui_files += " curses.c"
ui_files += " vnc.c d3des.c vnc-enc-zlib.c "
ui_files += " vnc-enc-hextile.c vnc-enc-tight.c vnc-palette.c "
ui_files += " vnc-enc-zrle.c"
if env['vnc_tls'] == True:
    ui_files += " vnc-tls.c vnc-auth-vencrypt.c"
if env['vnc_sasl'] == True:
    ui_files += " vnc-auth-sasl.c"
ui_files += " vnc-jobs.c"

ui_list = []
for ui_file in Split(ui_files):
    ui_list.append("ui/%s" % ui_file)

# if env['cocoa'] == True:
    # comm_files += " cocoa.c"
if env['iothread'] == True:
    comm_files += " qemu-thread.c"

comm_files += " util/acl.c util/iov.c util/crc32c.c"
comm_files += " util/notify.c util/compatfd.c"
comm_files += " qemu-timer.c util/qemu-timer-common.c"
comm_files += " hmp.c qmp.c"
comm_files += " main-loop.c"
comm_files += " util/event_notifier-posix.c"

slirp_files = " "
if env['slirp'] == True:
    slirp_files = " ".join(Glob('slirp/*.c', strings=True))
    comm_lib_env['CPPPATH'].append("slirp")

if env['xen'] == True:
    comm_files += " xen_backend.c xen_devconfig.c xen_console.c xenfb.c"
    comm_files += " xen_disk.c xen_nic.c"
else:
    comm_files += " xen-stub.c"

comm_files += " " + qom_files

comm_hw_list = []
for h in Split(comm_hw_files):
    comm_hw_list.append("hw/%s" % h)

q_objs = comm_lib_env.Object(Split(q_object_files))
block_objs = comm_lib_env.Object(Split(block_files))
net_objs = comm_lib_env.Object(Split(net_files))
oslib_objs = comm_lib_env.Object(Split(oslib_files))
msmouse_obj = comm_lib_env.Object("backends/msmouse.c")
qdev_properties_obj = comm_lib_env.Object("hw/core/qdev-properties.c")
qdev_properties_system_obj = comm_lib_env.Object("hw/core/qdev-properties-system.c")
qdev_monitor_obj = comm_lib_env.Object("qdev-monitor.c")
monitor_obj = comm_lib_env.Object("monitor.c")

comm_lib_env['CPPPATH'].append("fpu")
libqemu_comm = comm_lib_env.Library('qemu_common', Split(comm_files) +
        audio_list + Split(slirp_files) + comm_hw_list + block_objs +
        net_objs + q_objs + ui_list + oslib_objs + msmouse_obj +
        qdev_properties_obj + qdev_monitor_obj + monitor_obj +
        qdev_properties_system_obj )

#######################
# Qemu Binary Builder

qemu_bld_action = "$CC $_LIBDIRFLAGS $LINKFLAGS -o $TARGET $SOURCES $ARLIB_BEGIN $ARLIB $ARLIB_END $_LIBFLAGS"
qemu_bld = Builder(action = Action(qemu_bld_action,
    cmdstr="$LINKCOMSTR"))
comm_lib_env['BUILDERS']['QEMU_BIN'] = qemu_bld
env['BUILDERS']['QEMU_BIN'] = qemu_bld

hxtool_bld = "sh %s/scripts/hxtool -h < $SOURCE > $TARGET" % env['source_path']
env['BUILDERS']['HXTOOL'] = Builder(action = Action(hxtool_bld,
    cmdstr="$CREATECOMSTR"))

##########################
# trace

trace_h_bld = "%s/scripts/tracetool.py --backend=nop --format=h < $SOURCE > $TARGET " % env['source_path']
trace_c_bld = "%s/scripts/tracetool.py --backend=nop --format=c < $SOURCE > $TARGET " % env['source_path']

env['BUILDERS']['TRACE_H_CR'] = Builder(action = Action(trace_h_bld,
    cmdstr="$CREATECOMSTR"))
env['BUILDERS']['TRACE_C_CR'] = Builder(action = Action(trace_c_bld,
    cmdstr="$CREATECOMSTR"))

trace_h = env.TRACE_H_CR('trace.h', 'trace-events')
trace_c = env.TRACE_C_CR('trace.c', 'trace-events')

trace_files = ""

if env['trace_backend'] == 'dtrace':
    trace_files += "trace-dtrace.c "

if env['trace_default']:
    trace_files += "trace/default.c "

if env['trace_backend'] == 'simple':
    trace_files += "trace/simple.c "
    trace_files += "util/qemu-timer-common.c "

if env['trace_backend'] == 'stderr':
    trace_files += "trace/stderr.c "

trace_files += "trace/control.c "
trace_files += "trace/generated-events.c "

#######################
# qemu-img :

# qemu-img-cmds.h
qemu_img_cmds_h = env.HXTOOL('qemu-img-cmds.h', 'qemu-img-cmds.hx')

# osdep_o = env.Object("osdep.c")
tool_files = "main-loop.c util/notify.c iohandler.c "
tool_files += "util/compatfd.c "
tool_files += trace_files

qemu_tool_o = comm_lib_env.Object(Split(tool_files))

if env['trace_backend'] != 'dtrace':
    qemu_tool_o += comm_lib_env.Object(trace_c)

qemu_err_o = comm_lib_env.Object("util/qemu-error.c")
qemu_timer_comm_o = comm_lib_env.Object("util/qemu-timer-common.c")
qemu_timer_comm_o += comm_lib_env.Object("qemu-timer.c")
qemu_img_comm_objs = qemu_tool_o + block_objs + q_objs + oslib_objs
qemu_img_comm_objs += qemu_err_o
qemu_img_comm_objs += qemu_timer_comm_o
qemu_img_o = comm_lib_env.Object("qemu-img.c")

#######################
# cpu emulator library

cpu_emu_objs = "exec.c translate-all.c page_cache.c "
cpu_emu_objs += " %s/translate.c" % target_path
cpu_emu_objs += " %s/cpu.c" % target_path
# TCG code generator
cpu_emu_objs += " tcg/tcg.c" # tcg/tcg-runtime.c"
cpu_emu_objs += " tcg/optimize.c" # tcg/tcg-runtime.c"
env['CPPPATH'].append("%s/tcg" % env['source_path'])
env['CPPPATH'].append("%s/tcg/%s" % (env['source_path'], target_base_arch))

cpu_emu_objs += " fpu/softfloat.c"

env['CPPPATH'].append("%s/fpu" % env['source_path'])
helper_cflags = []
if type(env['CCFLAGS']) == list:
    helper_cflags.extend(env['CCFLAGS'])
else:
    helper_cflags.append(env['CCFLAGS'])
helper_cflags.append(HELPER_CFLAGS)

#op_helper_obj = env.Object('%s/op_helper.c' % target_path, 
#        CCFLAGS = helper_cflags)
cc_helper_obj = env.Object('%s/cc_helper.c' % target_path,
        CCFLAGS = helper_cflags)
excp_helper_obj = env.Object('%s/excp_helper.c' % target_path,
        CCFLAGS = helper_cflags)
fpu_helper_obj = env.Object('%s/fpu_helper.c' % target_path,
        CCFLAGS = helper_cflags)
int_helper_obj = env.Object('%s/int_helper.c' % target_path,
        CCFLAGS = helper_cflags)
mem_helper_obj = env.Object('%s/mem_helper.c' % target_path,
        CCFLAGS = helper_cflags)
misc_helper_obj = env.Object('%s/misc_helper.c' % target_path,
        CCFLAGS = helper_cflags)
seg_helper_obj = env.Object('%s/seg_helper.c' % target_path,
        CCFLAGS = helper_cflags)
smm_helper_obj = env.Object('%s/smm_helper.c' % target_path,
        CCFLAGS = helper_cflags)
svm_helper_obj = env.Object('%s/svm_helper.c' % target_path,
        CCFLAGS = helper_cflags)
helper_obj = env.Object('%s/helper.c' % target_path,
        CCFLAGS = helper_cflags)
cpu_exec_obj = env.Object('cpu-exec.c', 
        CCFLAGS = helper_cflags)

cpu_emu_objs += " %s/cc_helper.o %s/excp_helper.o %s/fpu_helper.o %s/int_helper.o %s/mem_helper.o %s/misc_helper.o %s/seg_helper.o %s/smm_helper.o %s/svm_helper.o %s/helper.o" % (target_path, target_path, target_path, target_path, target_path, target_path, target_path, target_path, target_path, target_path)
cpu_emu_objs += " cpu-exec.o"

if target['user_only']:
    cpu_emu_objs += " mmu.c"

# disassembler code needed for debugging only
cpu_emu_objs += " disas.c"
# We are compiling for i386 or x86_64 only so set USE_I386_DIS to true
USE_I386_DIS = True
cpu_emu_objs += " disas/i386.c gdbstub.c target-i386/gdbstub.c stubs/gdbstub.c "

# libqemu
libqemu = env.Library('qemu', Split(cpu_emu_objs))

# Make sure that linux_user flag is not set
if target['linux_user']:
    print("ERROR: Linux user flag is set.")
    print("This configuration is not supported in MARSSx86")
    Return('error_str')

if target['darwin_user']:
    print("ERROR: Darwin user flag is set.")
    print("This configuration is not supported in MARSSx86")
    Return('error_str')

if target['user_only']:
    print("ERROR: user mode flag is set.")
    print("This configuration is not supported in MARSSx86")
    Return('error_str')

obj_files = "ptlsim.c dma-helpers.c"
obj_files += " %s/machine.c" % target_path
obj_files += " cpus.c arch_init.c balloon.c "
obj_files += " memory.c cputlb.c "
obj_files += " qtest.c"
hw_files = "pci/pci.c pci/pci_host.c core/loader.c core/sysbus.c isa/isa-bus.c cpu/icc_bus.c"
hw_files += " pci/pci_bridge.c pci/pci-hotplug-old.c pci/msi.c pci/pcie_host.c "
hw_files += " pci-bridge/ioh3420.c pci-bridge/xio3130_upstream.c pci-bridge/xio3130_downstream.c "
# hw_files += " ecc.c nand.c"
# virtio in hw
hw_files += " virtio/virtio.c virtio/virtio-pci.c block/virtio-blk.c virtio/virtio-balloon.c "
hw_files += " net/virtio-net.c char/virtio-console.c char/virtio-serial-bus.c virtio/virtio-bus.c "
hw_files += " net/vhost_net.c"
hw_files += " nvram/fw_cfg.c watchdog/watchdog.c watchdog/wdt_i6300esb.c pci/msix.c "
hw_files += " scsi/virtio-scsi.c "

if env['kvm']:
    obj_files += " kvm-all.c"
    obj_files += " %s/kvm.c" % target_path
    obj_files += " hw/i386/kvmvapic.c"
    obj_files += " hw/i386/kvm/clock.c hw/i386/kvm/apic.c hw/i386/kvm/i8259.c"
    obj_files += " hw/i386/kvm/ioapic.c hw/i386/kvm/i8254.c"
    obj_files += " %s/hyperv.c" % target_path
else:
    obj_files += " kvm-stub.c"

if env['aio']:
    obj_files += " aio-posix.c"
#else:
#    obj_files += " block-raw-posix.c"

env.Append(LIBS = "z")

# Audio configuration - Not converted fully
if env['aud_lib'] != "":
    env.Append(LIBS = env['aud_lib'])

sound_hw_files = "audio/sb16.c audio/es1370.c audio/ac97.c"

if 'adlib' in env:
    sound_hw_files += " audio/fmopl.c audio/adlib.c"
    #adlib_obj = env.Object('hw/adlib.c')
    #fpmol_obj = env.Object('hw/fpmol.c')
    env.Append(CCFLAGS = '-DBUILD_Y8950=0')

if 'gus' in env:
    sound_hw_files += " audio/gus.c audio/gusemu_hal.c audio/gusemu_mixer.c"

if 'cs4231a' in env:
    sound_hw_files += " audio/cs4231a.c"

hw_files += " %s" % sound_hw_files
# VNC TLS flags are added in configuration
#if env['vnc_tls']:

# Bluez flags are added in configuration
hw_files += " bt/hci.c bt/core.c bt/hid.c bt/l2cap.c bt/sdp.c "

hw_files += " scsi/lsi53c895a.c scsi/esp.c"
hw_files += " usb/libhw.c"
# hw_files += " eeprom93xx.c"
hw_files += " net/eepro100.c net/ne2000.c net/pcnet-pci.c net/pcnet.c net/rtl8139.c net/e1000.c"
# hw_files += " msmouse.c"

if target_base_arch == "i386":
    hw_files += " input/pckbd.c display/vga.c display/vga-isa.c display/vga-pci.c intc/ioapic.c"
    hw_files += " block/fdc.c timer/mc146818rtc.c char/serial.c intc/i8259.c timer/i8254.c acpi/piix4.c "
    hw_files += " audio/pcspk.c i386/pc.c display/cirrus_vga.c intc/apic.c char/parallel.c acpi/core.c i386/smbios.c"
    hw_files += " input/vmmouse.c misc/vmport.c net/ne2000-isa.c dma/i8257.c misc/pvpanic.c "
    hw_files += " display/vmware_vga.c timer/hpet.c char/serial-isa.c "
    hw_files += " i2c/pm_smbus.c isa/apm.c"
    hw_files += " i386/multiboot.c char/debugcon.c "
    hw_files += " i386/pc_piix.c misc/sga.c pci-host/piix.c pci-host/pam.c "
    hw_files += " intc/apic_common.c i386/pc_sysfw.c intc/i8259_common.c "
    hw_files += " timer/i8254_common.c intc/ioapic_common.c "
    env.Append(CPPFLAGS = " -DHAS_AUDIO -DHAS_AUDIO_CHOICE")
else:
    print("ERROR: Unsupported arch.")
    print("MARSSx86 Only supports x86_64 architecture")

if env['usb_ohci']:
    hw_files += " usb/hcd-ohci.c"
if env['usb_uhci']:
    hw_files += " usb/hcd-uhci.c"
if env['usb_ehci']:
    hw_files += " usb/hcd-ehci.c"
if env['usb_xhci']:
    hw_files += " usb/hcd-xhci.c"

hw_files += " pci/pcie.c pci/pcie_aer.c pci/pcie_port.c "
hw_files += " input/hid.c "

ide_files = " ".join(env.Glob("hw/ide/*.c", strings=True))

# remove mac io file from ide
ide_files = ide_files.replace("hw/ide/macio.c", "")

#if env['gdbstub']:
#    obj_files += " gdbstub.c"
    # We are not including any gdbstub xml files as 
    # its not required in arch that we support

hw_files += " block/pflash_cfi01.c block/pflash_cfi02.c "

##########################
# qemu-options.def
qemu_options_def = env.HXTOOL('qemu-options.def', 'qemu-options.hx')

# qemu-monitor.h
# qemu_monitor_h = env.HXTOOL('qemu-monitor.h', 'qemu-monitor.hx')

# hmp-commands.h
hmp_commands_h = env.HXTOOL('hmp-commands.h', 'hmp-commands.hx')

# qmp-commands.h
qmp_commands_old_h = env.HXTOOL(
        '%s/qmp-commands-old.h' % (target_build_path), 'qmp-commands.hx')

##########################
# qapi and qga
qapi_type_bld = "python %s/scripts/qapi-types.py -o %s < $SOURCE " % (
        env['source_path'], env['source_path'])
qapi_visit_bld = "python %s/scripts/qapi-visit.py -o %s < $SOURCE " % (
        env['source_path'], env['source_path'])
qapi_command_bld = "python %s/scripts/qapi-commands.py -m -o %s < $SOURCE " % (
        env['source_path'], env['source_path'])

qga_type_bld = "python %s/scripts/qapi-types.py -o %s/qapi-generated " \
        "-p \"qga-\" < $SOURCE " % (
     env['source_path'], env['source_path'])
qga_visit_bld = "python %s/scripts/qapi-visit.py -o %s/qapi-generated " \
        "-p \"qga-\" < $SOURCE " % (
     env['source_path'], env['source_path'])
qga_command_bld = "python %s/scripts/qapi-commands.py " \
        "-o %s/qapi-generated -p \"qga-\" < $SOURCE " % (
        env['source_path'], env['source_path'])

env['BUILDERS']['QAPI_TYPE'] = Builder(action = Action(qapi_type_bld,
    cmdstr="$CREATECOMSTR"))
env['BUILDERS']['QAPI_VISIT'] = Builder(action = Action(qapi_visit_bld,
    cmdstr="$CREATECOMSTR"))
env['BUILDERS']['QAPI_CMD'] = Builder(action = Action(qapi_command_bld,
    cmdstr="$CREATECOMSTR"))

env['BUILDERS']['QGA_TYPE'] = Builder(action = Action(qga_type_bld,
    cmdstr="$CREATECOMSTR"))
env['BUILDERS']['QGA_VISIT'] = Builder(action = Action(qga_visit_bld,
    cmdstr="$CREATECOMSTR"))
env['BUILDERS']['QGA_CMD'] = Builder(action = Action(qga_command_bld,
    cmdstr="$CREATECOMSTR"))

qapi_type_ch = env.QAPI_TYPE(
        'qapi-types.c', 'qapi-schema.json')
qapi_visit_ch = env.QAPI_VISIT(
        'qapi-visit.c', 'qapi-schema.json')
qapi_cmd_ch = env.QAPI_CMD(
        'qmp-marshal.c', 'qapi-schema.json')

qga_type_ch = env.QGA_TYPE(
        'qapi-generated/qga-qapi-types.c', 'qapi-schema.json')
qga_visit_ch = env.QGA_VISIT(
        'qapi-generated/qga-qapi-visit.c', 'qapi-schema.json')
qga_cmd_ch = env.QGA_CMD(
        'qapi-generated/qga-qmp-marshal.c', 'qapi-schema.json')

env.Depends(['config-host.h'], [qapi_type_ch, qapi_cmd_ch])
env.Depends(qapi_type_ch, qapi_visit_ch)
env.Depends(qga_type_ch, qga_visit_ch)

qapi_n_files = "qapi-visit-core.c qapi-dealloc-visitor.c "
qapi_n_files += "qmp-input-visitor.c qmp-output-visitor.c "
qapi_n_files += "qmp-registry.c qmp-dispatch.c "
qapi_n_files += "string-input-visitor.c string-output-visitor.c "
qapi_n_files += "opts-visitor.c "

qapi_files_list = []
for q_f in Split(qapi_n_files):
    qapi_files_list.append("qapi/%s" % q_f)

qga_n_files = "commands.c guest-agent-command-state.c "
qga_n_files += "commands-posix.c channel-posix.c "

##########################
# pc-bios and option-rom
Export('env')
pc_bios_objs = SConscript('%s/pc-bios/SConscript' % env['source_path'])
SConscript('%s/pc-bios/optionrom/SConscript' % env['source_path'])

# We dont support cocoa yet, if we do in future then add 
# LIB and CLFAGS for cocoa

if env['slirp']:
    env['CPPPATH'].append("%s/slirp" % env['source_path'])

if env['static']:
    env.Append(LINKFLAGS = "-static")

env.Append(LIBS = "util")

hw_files_list = []
for hw_f in Split(hw_files):
    hw_files_list.append("hw/%s" % hw_f)
hw_files_list.append("device-hotplug.c")

hw_files_o = env.Object(Split(obj_files) + hw_files_list + Split(ide_files))
trace_o = comm_lib_env.Object(Split(trace_files))
if env['trace_backend'] != 'dtrace':
    trace_o += comm_lib_env.Object(trace_c)

generated_objs = env.Object([qapi_type_ch, qapi_visit_ch, qapi_cmd_ch])
    # qga_type_ch, qga_visit_ch, qga_cmd_ch])

universal_objs = env.Object(qapi_files_list)

#qemu_img = comm_lib_env.QEMU_BIN('qemu-img', generated_objs +
#        qemu_img_o + qemu_img_comm_objs + hw_files_o + universal_objs,
#        LIBS = comm_lib_env['LIBS'] + ["z"] + libqemu + libqemu_comm)

#qemu_nbd_o = comm_lib_env.Object("qemu-nbd.c")
#qemu_nbd = comm_lib_env.QEMU_BIN('qemu-nbd', generated_objs +
#        qemu_nbd_o + qemu_img_comm_objs + hw_files_o + universal_objs,
#        LIBS = comm_lib_env['LIBS'] + ["z"] + libqemu + libqemu_comm)

#qemu_io_o = comm_lib_env.Object(Split("qemu-io.c"))
#qemu_io = comm_lib_env.QEMU_BIN('qemu-io', generated_objs +
#        qemu_io_o + qemu_img_comm_objs + hw_files_o + universal_objs,
#        LIBS = comm_lib_env['LIBS'] + ["z"] + libqemu + libqemu_comm)

if env['gprof']:
    vl_obj = env.Object('vl.c', CCFLAGS = env['CCFLAGS'] + "-p")
    obj_files += " vl.o"
    env.Append(LINKFLAGS = "-p")
else:
    obj_files += " vl.c"
hw_files_o = env.Object(Split(obj_files) + hw_files_list + Split(ide_files))

env.Append(LIBPATH = "%s/../plugins" % env['source_path'])
env.Append(LINKFLAGS = "-Wl,-rpath=%s/../plugins" % env['source_path'])
env.Append(LINKFLAGS = "-Wl,--no-as-needed")
env.Append(LIBS = plugins)
env['ARLIB'] = libqemu_comm + libqemu + ptlsim_lib
qemu_bin = env.QEMU_BIN(qemu_prog_name, hw_files_o + pc_bios_objs +
        generated_objs + universal_objs +
        trace_o)

env.Depends(qemu_bin, env['ARLIB'])

# print env.Dump()

Return('qemu_bin')
