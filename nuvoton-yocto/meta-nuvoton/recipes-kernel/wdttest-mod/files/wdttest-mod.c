// SPDX-License-Identifier: GPL-2.0+
/*
 * The kernel module is only a simple example to register SoC's GPIO interrupt,
 * insert the module will request a PIN(GPIO5) to irq & set a PIN(GPIO6) to output.
 * 
 * Copyright (C) 2024 Nuvoton Technology Corp.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

static struct gpio_desc *gpio_irq1, *gpio_irq2, *gpio_out;

static ssize_t Output_pulse(struct file *filep, struct kobject *kobj, struct bin_attribute *bin_attr,
                            char *buf, loff_t off, size_t len)
{
    gpiod_set_value(gpio_out, 0);
    msleep(100);
    gpiod_set_value(gpio_out, 1);

    return len;
}

static struct bin_attribute test_bin_attr = {
    .attr = { 
	.name = "Output_pulse",
	.mode = 0220,
    },
    .write = Output_pulse,
};

static irqreturn_t test_handler(int irq, void * ident)
{
    pr_info("%s: irq = %d\n", __FUNCTION__, irq);
    return IRQ_HANDLED;
}

static int my_probe(struct platform_device *pdev) {
    struct kobject *kobj_ref;
    int irq, ret;

    gpio_irq1 = devm_gpiod_get_index(&pdev->dev, "irqtest", 0, GPIOD_IN);
    if (IS_ERR(gpio_irq1)) {
	pr_err("Failed to get gpio_irq1 descriptor!\n");
	return PTR_ERR(gpio_irq1);
    }

    gpio_irq2 = devm_gpiod_get_index(&pdev->dev, "irqtest", 1, GPIOD_IN);
    if (IS_ERR(gpio_irq2)) {
        pr_err("Failed to get gpio_irq2 descriptor!\n");
        return PTR_ERR(gpio_irq2);
    }

    gpio_out = devm_gpiod_get_index(&pdev->dev, "irqtest", 2, GPIOD_OUT_HIGH);
    if (IS_ERR(gpio_out)) {
        pr_err("Failed to get gpio_out descriptor!\n");
        return PTR_ERR(gpio_out);
    }

    irq = gpiod_to_irq(gpio_irq1);
    if (!irq) {
        pr_err("failed to get irq1 number!\n");
        return irq;
    }

    ret = devm_request_irq(&pdev->dev, gpiod_to_irq(gpio_irq1), test_handler, IRQF_TRIGGER_FALLING, "test_irq1", NULL);
    if (ret) {
        pr_err("Failed to register IRQ1!\n");
        return ret;
    }

    irq = gpiod_to_irq(gpio_irq2);
    if (!irq) {
        pr_err("failed to get irq2 number!\n");
        return irq;
    }

    ret = devm_request_irq(&pdev->dev, gpiod_to_irq(gpio_irq2), test_handler, IRQF_TRIGGER_FALLING, "test_irq2", NULL);
    if (ret) {
        pr_err("Failed to register IRQ2!\n");
        return ret;
    }

    kobj_ref = kobject_create_and_add("Test_kobject", kernel_kobj);
    if (!kobj_ref) {
        pr_err("Failed to create kobject!\n");
        return PTR_ERR(kobj_ref);
    }

    ret = sysfs_create_bin_file(kobj_ref, &test_bin_attr);
    if (ret) {
        pr_err("Failed to create bin file!\n");
        kobject_put(kobj_ref);
        return -ENOENT;
    }

    platform_set_drvdata(pdev, kobj_ref);

    dev_info(&pdev->dev, "%s: Probe device: %s\n", __func__, pdev->name);

    return 0;
}

static int my_remove(struct platform_device *pdev) {
    struct kobject *kobj_ref = platform_get_drvdata(pdev);
    dev_info(&pdev->dev, "%s: Remove device",  __func__);
    kobject_put(kobj_ref);
    sysfs_remove_bin_file(kernel_kobj, &test_bin_attr);

    return 0;
}

static struct of_device_id my_driver_ids[] = {
    {
        .compatible = "tmtest,irqtest",
    },{}
};
MODULE_DEVICE_TABLE(of, my_driver_ids);

static struct platform_driver my_driver = {
    .probe = my_probe,
    .remove = my_remove,
    .driver = {
	.name = "irqtestdrv",
        .of_match_table = my_driver_ids,
    },
};

static int __init irqtest_init(void) {
    if(platform_driver_register(&my_driver)) {
    	pr_err("Failed to register platform driver!");
        return -1;
    }

    return 0;
}

static void __exit irqtest_exit(void) {
        platform_driver_unregister(&my_driver);
}

module_init(irqtest_init);
module_exit(irqtest_exit);

MODULE_DESCRIPTION("A test interrupt controller driver for SoC GPIO");
MODULE_AUTHOR("Tzu-Ming Yu <tmyu0@nuvoton.com>");
MODULE_LICENSE("GPL");
