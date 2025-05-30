// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  acpi_ac.c - ACPI AC Adapter Driver (Revision: 27)
 *
 *  Copyright (C) 2001, 2002 Andy Grover <andrew.grover@intel.com>
 *  Copyright (C) 2001, 2002 Paul Diefenbaugh <paul.s.diefenbaugh@intel.com>
 */

#define pr_fmt(fmt) "ACPI: AC: " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/dmi.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/string_choices.h>
#include <linux/acpi.h>
#include <acpi/battery.h>

#define ACPI_AC_CLASS			"ac_adapter"
#define ACPI_AC_DEVICE_NAME		"AC Adapter"
#define ACPI_AC_FILE_STATE		"state"
#define ACPI_AC_NOTIFY_STATUS		0x80
#define ACPI_AC_STATUS_OFFLINE		0x00
#define ACPI_AC_STATUS_ONLINE		0x01
#define ACPI_AC_STATUS_UNKNOWN		0xFF

MODULE_AUTHOR("Paul Diefenbaugh");
MODULE_DESCRIPTION("ACPI AC Adapter Driver");
MODULE_LICENSE("GPL");

static int acpi_ac_add(struct acpi_device *device);
static void acpi_ac_remove(struct acpi_device *device);
static void acpi_ac_notify(acpi_handle handle, u32 event, void *data);

static const struct acpi_device_id ac_device_ids[] = {
	{"ACPI0003", 0},
	{"", 0},
};
MODULE_DEVICE_TABLE(acpi, ac_device_ids);

#ifdef CONFIG_PM_SLEEP
static int acpi_ac_resume(struct device *dev);
#endif
static SIMPLE_DEV_PM_OPS(acpi_ac_pm, NULL, acpi_ac_resume);

static int ac_sleep_before_get_state_ms;
static int ac_only;

static struct acpi_driver acpi_ac_driver = {
	.name = "ac",
	.class = ACPI_AC_CLASS,
	.ids = ac_device_ids,
	.ops = {
		.add = acpi_ac_add,
		.remove = acpi_ac_remove,
		},
	.drv.pm = &acpi_ac_pm,
};

struct acpi_ac {
	struct power_supply *charger;
	struct power_supply_desc charger_desc;
	struct acpi_device *device;
	unsigned long long state;
	struct notifier_block battery_nb;
};

#define to_acpi_ac(x) power_supply_get_drvdata(x)

/* AC Adapter Management */
static int acpi_ac_get_state(struct acpi_ac *ac)
{
	acpi_status status = AE_OK;

	if (!ac)
		return -EINVAL;

	if (ac_only) {
		ac->state = 1;
		return 0;
	}

	status = acpi_evaluate_integer(ac->device->handle, "_PSR", NULL,
				       &ac->state);
	if (ACPI_FAILURE(status)) {
		acpi_handle_info(ac->device->handle,
				"Error reading AC Adapter state: %s\n",
				acpi_format_exception(status));
		ac->state = ACPI_AC_STATUS_UNKNOWN;
		return -ENODEV;
	}

	return 0;
}

/* sysfs I/F */
static int get_ac_property(struct power_supply *psy,
			   enum power_supply_property psp,
			   union power_supply_propval *val)
{
	struct acpi_ac *ac = to_acpi_ac(psy);

	if (!ac)
		return -ENODEV;

	if (acpi_ac_get_state(ac))
		return -ENODEV;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = ac->state;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static enum power_supply_property ac_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

/* Driver Model */
static void acpi_ac_notify(acpi_handle handle, u32 event, void *data)
{
	struct acpi_device *device = data;
	struct acpi_ac *ac = acpi_driver_data(device);

	switch (event) {
	default:
		acpi_handle_debug(device->handle, "Unsupported event [0x%x]\n",
				  event);
		fallthrough;
	case ACPI_AC_NOTIFY_STATUS:
	case ACPI_NOTIFY_BUS_CHECK:
	case ACPI_NOTIFY_DEVICE_CHECK:
		/*
		 * A buggy BIOS may notify AC first and then sleep for
		 * a specific time before doing actual operations in the
		 * EC event handler (_Qxx). This will cause the AC state
		 * reported by the ACPI event to be incorrect, so wait for a
		 * specific time for the EC event handler to make progress.
		 */
		if (ac_sleep_before_get_state_ms > 0)
			msleep(ac_sleep_before_get_state_ms);

		acpi_ac_get_state(ac);
		acpi_bus_generate_netlink_event(device->pnp.device_class,
						  dev_name(&device->dev), event,
						  (u32) ac->state);
		acpi_notifier_call_chain(device, event, (u32) ac->state);
		kobject_uevent(&ac->charger->dev.kobj, KOBJ_CHANGE);
	}
}

static int acpi_ac_battery_notify(struct notifier_block *nb,
				  unsigned long action, void *data)
{
	struct acpi_ac *ac = container_of(nb, struct acpi_ac, battery_nb);
	struct acpi_bus_event *event = (struct acpi_bus_event *)data;

	/*
	 * On HP Pavilion dv6-6179er AC status notifications aren't triggered
	 * when adapter is plugged/unplugged. However, battery status
	 * notifications are triggered when battery starts charging or
	 * discharging. Re-reading AC status triggers lost AC notifications,
	 * if AC status has changed.
	 */
	if (strcmp(event->device_class, ACPI_BATTERY_CLASS) == 0 &&
	    event->type == ACPI_BATTERY_NOTIFY_STATUS)
		acpi_ac_get_state(ac);

	return NOTIFY_OK;
}

static int __init thinkpad_e530_quirk(const struct dmi_system_id *d)
{
	ac_sleep_before_get_state_ms = 1000;
	return 0;
}

static int __init ac_only_quirk(const struct dmi_system_id *d)
{
	ac_only = 1;
	return 0;
}

/* Please keep this list alphabetically sorted */
static const struct dmi_system_id ac_dmi_table[]  __initconst = {
	{
		/* Kodlix GK45 returning incorrect state */
		.callback = ac_only_quirk,
		.matches = {
			DMI_MATCH(DMI_PRODUCT_NAME, "GK45"),
		},
	},
	{
		/* Lenovo Thinkpad e530, see comment in acpi_ac_notify() */
		.callback = thinkpad_e530_quirk,
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "LENOVO"),
			DMI_MATCH(DMI_PRODUCT_NAME, "32597CG"),
		},
	},
	{},
};

static int acpi_ac_add(struct acpi_device *device)
{
	struct power_supply_config psy_cfg = {};
	struct acpi_ac *ac;
	int result;

	ac = kzalloc(sizeof(struct acpi_ac), GFP_KERNEL);
	if (!ac)
		return -ENOMEM;

	ac->device = device;
	strcpy(acpi_device_name(device), ACPI_AC_DEVICE_NAME);
	strcpy(acpi_device_class(device), ACPI_AC_CLASS);
	device->driver_data = ac;

	result = acpi_ac_get_state(ac);
	if (result)
		goto err_release_ac;

	psy_cfg.drv_data = ac;

	ac->charger_desc.name = acpi_device_bid(device);
	ac->charger_desc.type = POWER_SUPPLY_TYPE_MAINS;
	ac->charger_desc.properties = ac_props;
	ac->charger_desc.num_properties = ARRAY_SIZE(ac_props);
	ac->charger_desc.get_property = get_ac_property;
	ac->charger = power_supply_register(&ac->device->dev,
					    &ac->charger_desc, &psy_cfg);
	if (IS_ERR(ac->charger)) {
		result = PTR_ERR(ac->charger);
		goto err_release_ac;
	}

	pr_info("%s [%s] (%s-line)\n", acpi_device_name(device),
		acpi_device_bid(device), str_on_off(ac->state));

	ac->battery_nb.notifier_call = acpi_ac_battery_notify;
	register_acpi_notifier(&ac->battery_nb);

	result = acpi_dev_install_notify_handler(device, ACPI_ALL_NOTIFY,
						 acpi_ac_notify, device);
	if (result)
		goto err_unregister;

	return 0;

err_unregister:
	power_supply_unregister(ac->charger);
	unregister_acpi_notifier(&ac->battery_nb);
err_release_ac:
	kfree(ac);

	return result;
}

#ifdef CONFIG_PM_SLEEP
static int acpi_ac_resume(struct device *dev)
{
	struct acpi_ac *ac = acpi_driver_data(to_acpi_device(dev));
	unsigned int old_state;

	old_state = ac->state;
	if (acpi_ac_get_state(ac))
		return 0;
	if (old_state != ac->state)
		kobject_uevent(&ac->charger->dev.kobj, KOBJ_CHANGE);

	return 0;
}
#else
#define acpi_ac_resume NULL
#endif

static void acpi_ac_remove(struct acpi_device *device)
{
	struct acpi_ac *ac = acpi_driver_data(device);

	acpi_dev_remove_notify_handler(device, ACPI_ALL_NOTIFY,
				       acpi_ac_notify);
	power_supply_unregister(ac->charger);
	unregister_acpi_notifier(&ac->battery_nb);

	kfree(ac);
}

static int __init acpi_ac_init(void)
{
	int result;

	if (acpi_disabled)
		return -ENODEV;

	if (acpi_quirk_skip_acpi_ac_and_battery())
		return -ENODEV;

	dmi_check_system(ac_dmi_table);

	result = acpi_bus_register_driver(&acpi_ac_driver);
	if (result < 0)
		return -ENODEV;

	return 0;
}

static void __exit acpi_ac_exit(void)
{
	acpi_bus_unregister_driver(&acpi_ac_driver);
}
module_init(acpi_ac_init);
module_exit(acpi_ac_exit);
