# SPDX-License-Identifier: GPL-2.0
#
# Makefile for drivers/platform/x86/intel
# Intel x86 Platform-Specific Drivers
#

obj-$(CONFIG_INTEL_ATOMISP2_PDX86)	+= atomisp2/
obj-$(CONFIG_INTEL_IFS)			+= ifs/
obj-$(CONFIG_INTEL_SAR_INT1092)		+= int1092/
obj-$(CONFIG_INTEL_CHT_INT33FE)		+= int33fe/
obj-$(CONFIG_INTEL_SKL_INT3472)		+= int3472/
obj-$(CONFIG_INTEL_PMC_CORE)		+= pmc/
obj-$(CONFIG_INTEL_PMT_CLASS)		+= pmt/
obj-$(CONFIG_INTEL_TELEMETRY)		+= telemetry/
obj-$(CONFIG_INTEL_SPEED_SELECT_INTERFACE) += speed_select_if/
obj-$(CONFIG_INTEL_WMI)			+= wmi/
obj-$(CONFIG_INTEL_UNCORE_FREQ_CONTROL)	+= uncore-frequency/

# Intel miscellaneous drivers
obj-$(CONFIG_INTEL_ISHTP_ECLITE)	+= ishtp_eclite.o
intel_sdsi-y				:= sdsi.o
obj-$(CONFIG_INTEL_SDSI)		+= intel_sdsi.o
intel_vsec-y				:= vsec.o
obj-$(CONFIG_INTEL_VSEC)		+= intel_vsec.o

# Intel Uncore drivers
intel-rst-y				:= rst.o
obj-$(CONFIG_INTEL_RST)			+= intel-rst.o
intel-smartconnect-y			:= smartconnect.o
obj-$(CONFIG_INTEL_SMARTCONNECT)	+= intel-smartconnect.o
intel_turbo_max_3-y			:= turbo_max_3.o
obj-$(CONFIG_INTEL_TURBO_MAX_3)		+= intel_turbo_max_3.o

# TPMI drivers
intel_vsec_tpmi-y			:= tpmi.o
obj-$(CONFIG_INTEL_TPMI)		+= intel_vsec_tpmi.o

# Intel input drivers
intel-hid-y				:= hid.o
obj-$(CONFIG_INTEL_HID_EVENT)		+= intel-hid.o
intel-vbtn-y				:= vbtn.o
obj-$(CONFIG_INTEL_VBTN)		+= intel-vbtn.o

# Intel miscellaneous drivers
intel_int0002_vgpio-y			:= int0002_vgpio.o
obj-$(CONFIG_INTEL_INT0002_VGPIO)	+= intel_int0002_vgpio.o
intel_oaktrail-y			:= oaktrail.o
obj-$(CONFIG_INTEL_OAKTRAIL)		+= intel_oaktrail.o
