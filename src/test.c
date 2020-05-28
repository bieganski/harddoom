#include <linux/module.h>
#include <linux/pci.h>
#include <linux/cdev.h>
#include <linux/anon_inodes.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/file.h>
#include <linux/kref.h>
#include <linux/interrupt.h>

#include "uharddoom.h"

#define ADLERDEV_MAX_DEVICES 256
#define ADLERDEV_NUM_BUFFERS 16

MODULE_LICENSE("GPL");

struct udoomdev_buffer {
    struct list_head lh;
    struct udoomdev_context *ctx;
    void *data_cpu;
    dma_addr_t data_dma;
    size_t fill_size;
};

struct udoomdev_context {
    struct udoomdev_device *dev;
    int pending_buffers;
    wait_queue_head_t wq;
    uint32_t sum;
};

struct udoomdev_device {
    struct pci_dev *pdev;
    struct cdev cdev;
    int idx;
    struct device *dev;
    void __iomem *bar;
    spinlock_t slock;
    struct list_head buffers_free;
    struct list_head buffers_running;
    wait_queue_head_t free_wq;
    wait_queue_head_t idle_wq;
};

static dev_t udoomdev_devno;
static struct udoomdev_device *udoomdev_devices[ADLERDEV_MAX_DEVICES];
static DEFINE_MUTEX(udoomdev_devices_lock);
static struct class udoomdev_class = {
    .name = "udoomdev",
    .owner = THIS_MODULE,
};

/* Hardware handling. */

static inline void udoomdev_iow(struct udoomdev_device *dev, uint32_t reg, uint32_t val)
{
    iowrite32(val, dev->bar + reg);
 printk(KERN_DEBUG "udoomdev %03x <- %08x\n", reg, val);
}

static inline uint32_t udoomdev_ior(struct udoomdev_device *dev, uint32_t reg)
{
    uint32_t res = ioread32(dev->bar + reg);
 printk(KERN_DEBUG "udoomdev %03x -> %08x\n", reg, res);
    return res;
}

/* IRQ handler.  */

static irqreturn_t udoomdev_isr(int irq, void *opaque)
{
    printk(KERN_DEBUG "ISR: irq:%x, ptr: %p\n", irq, opaque);
    return 2137;
}

/* Main device node handling.  */

static int udoomdev_open(struct inode *inode, struct file *file)
{
    printk(KERN_DEBUG "OPEN\n");
    return 2137;
}

static int udoomdev_release(struct inode *inode, struct file *file)
{
    printk(KERN_DEBUG "RELEASE\n");
    return 2137;
}

static ssize_t udoomdev_write(struct file *file, const char __user *buf,
        size_t len, loff_t *off)
{
    printk(KERN_DEBUG "WRITE\n");
    return 2137;
}

static ssize_t udoomdev_read(struct file *file, char __user *buf,
        size_t len, loff_t *off)
{
    printk(KERN_DEBUG "READ\n");
    return 2137;
}

static const struct file_operations udoomdev_file_ops = {
    .owner = THIS_MODULE,
    .open = udoomdev_open,
    .release = udoomdev_release,
    .write = udoomdev_write,
    .read = udoomdev_read,
};

/* PCI driver.  */

static int udoomdev_probe(struct pci_dev *pdev,
    const struct pci_device_id *pci_id)
{
    int err, i;
    struct list_head *lh, *tmp;


    printk(KERN_DEBUG "PROBE");

    /* Allocate our structure.  */
    struct udoomdev_device *dev = kzalloc(sizeof *dev, GFP_KERNEL);
    if (!dev) {
        err = -ENOMEM;
        goto out_alloc;
    }
    pci_set_drvdata(pdev, dev);
    dev->pdev = pdev;

    /* Locks etc.  */
    spin_lock_init(&dev->slock);
    init_waitqueue_head(&dev->free_wq);
    init_waitqueue_head(&dev->idle_wq);
    INIT_LIST_HEAD(&dev->buffers_free);
    INIT_LIST_HEAD(&dev->buffers_running);

    /* Allocate a free index.  */
    mutex_lock(&udoomdev_devices_lock);
    for (i = 0; i < ADLERDEV_MAX_DEVICES; i++)
        if (!udoomdev_devices[i])
            break;
    if (i == ADLERDEV_MAX_DEVICES) {
        err = -ENOSPC; // XXX right?
        mutex_unlock(&udoomdev_devices_lock);
        goto out_slot;
    }
    udoomdev_devices[i] = dev;
    dev->idx = i;
    mutex_unlock(&udoomdev_devices_lock);

    /* Enable hardware resources.  */
    if ((err = pci_enable_device(pdev)))
        goto out_enable;

    if ((err = pci_set_dma_mask(pdev, DMA_BIT_MASK(32))))
        goto out_mask;
    if ((err = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32))))
        goto out_mask;
    pci_set_master(pdev);

    if ((err = pci_request_regions(pdev, "udoomdev")))
        goto out_regions;

    /* Map the BAR.  */
    if (!(dev->bar = pci_iomap(pdev, 0, 0))) {
        err = -ENOMEM;
        goto out_bar;
    }

    /* Connect the IRQ line.  */
    if ((err = request_irq(pdev->irq, udoomdev_isr, IRQF_SHARED, "udoomdev", dev)))
        goto out_irq;

    /* Allocate some buffers.  */
    for (i = 0; i < ADLERDEV_NUM_BUFFERS; i++) {
        struct udoomdev_buffer *buf = kmalloc(sizeof *buf, GFP_KERNEL);
        if (!buf)
            goto out_cdev;
        if (!(buf->data_cpu = dma_alloc_coherent(&dev->pdev->dev,
                PAGE_SIZE,
                &buf->data_dma, GFP_KERNEL))) {
            kfree(buf);
            goto out_cdev;
        }
        buf->ctx = 0;
        list_add(&buf->lh, &dev->buffers_free);
    }

    udoomdev_iow(dev, ADLERDEV_INTR, 1);
    udoomdev_iow(dev, ADLERDEV_INTR_ENABLE, 1);
    
    /* We're live.  Let's export the cdev.  */
    cdev_init(&dev->cdev, &udoomdev_file_ops);
    if ((err = cdev_add(&dev->cdev, udoomdev_devno + dev->idx, 1)))
        goto out_cdev;

    /* And register it in sysfs.  */
    dev->dev = device_create(&udoomdev_class,
            &dev->pdev->dev, udoomdev_devno + dev->idx, dev,
            "udoom%d", dev->idx);
    if (IS_ERR(dev->dev)) {
        printk(KERN_ERR "udoomdev: failed to register subdevice\n");
        /* too bad. */
        dev->dev = 0;
    }

    return 0;

out_cdev:
    udoomdev_iow(dev, ADLERDEV_INTR_ENABLE, 0);
    list_for_each_safe(lh, tmp, &dev->buffers_free) {
        struct udoomdev_buffer *buf = list_entry(lh, struct udoomdev_buffer, lh);
        dma_free_coherent(&dev->pdev->dev, PAGE_SIZE, buf->data_cpu, buf->data_dma);
        kfree(buf);
    }
    free_irq(pdev->irq, dev);
out_irq:
    pci_iounmap(pdev, dev->bar);
out_bar:
    pci_release_regions(pdev);
out_regions:
out_mask:
    pci_disable_device(pdev);
out_enable:
    mutex_lock(&udoomdev_devices_lock);
    udoomdev_devices[dev->idx] = 0;
    mutex_unlock(&udoomdev_devices_lock);
out_slot:
    kfree(dev);
out_alloc:
    return err;
}

static void udoomdev_remove(struct pci_dev *pdev)
{
    struct list_head *lh, *tmp;
    struct udoomdev_device *dev = pci_get_drvdata(pdev);
    if (dev->dev) {
        device_destroy(&udoomdev_class, udoomdev_devno + dev->idx);
    }
    cdev_del(&dev->cdev);
    udoomdev_iow(dev, ADLERDEV_INTR_ENABLE, 0);
    list_for_each_safe(lh, tmp, &dev->buffers_free) {
        struct udoomdev_buffer *buf = list_entry(lh, struct udoomdev_buffer, lh);
        dma_free_coherent(&dev->pdev->dev, PAGE_SIZE, buf->data_cpu, buf->data_dma);
        kfree(buf);
    }
    free_irq(pdev->irq, dev);
    pci_iounmap(pdev, dev->bar);
    pci_release_regions(pdev);
    pci_disable_device(pdev);
    mutex_lock(&udoomdev_devices_lock);
    udoomdev_devices[dev->idx] = 0;
    mutex_unlock(&udoomdev_devices_lock);
    kfree(dev);
}

static int udoomdev_suspend(struct pci_dev *pdev, pm_message_t state)
{
    unsigned long flags;
    struct udoomdev_device *dev = pci_get_drvdata(pdev);
    spin_lock_irqsave(&dev->slock, flags);
    while (list_empty(&dev->buffers_free)) {
        spin_unlock_irqrestore(&dev->slock, flags);
        wait_event(dev->idle_wq, !list_empty(&dev->buffers_free));
        spin_lock_irqsave(&dev->slock, flags);
    }
    spin_unlock_irqrestore(&dev->slock, flags);
    udoomdev_iow(dev, ADLERDEV_INTR_ENABLE, 0);
    return 0;
}

static int udoomdev_resume(struct pci_dev *pdev)
{
    struct udoomdev_device *dev = pci_get_drvdata(pdev);
    udoomdev_iow(dev, ADLERDEV_INTR, 1);
    udoomdev_iow(dev, ADLERDEV_INTR_ENABLE, 1);
    return 0;
}

static struct pci_device_id udoomdev_pciids[] = {
    { PCI_DEVICE(UHARDDOOM_VENDOR_ID, UHARDDOOM_DEVICE_ID) },
    { 0 }
};

static struct pci_driver udoomdev_pci_driver = {
    .name = "udoomdev",
    .id_table = udoomdev_pciids,
    .probe = udoomdev_probe,
    .remove = udoomdev_remove,
    .suspend = udoomdev_suspend,
    .resume = udoomdev_resume,
};

/* Init & exit.  */

static int udoomdev_init(void)
{
    int err;
    if ((err = alloc_chrdev_region(&udoomdev_devno, 0, ADLERDEV_MAX_DEVICES, "udoomdev")))
        goto err_chrdev;
    if ((err = class_register(&udoomdev_class)))
        goto err_class;
    if ((err = pci_register_driver(&udoomdev_pci_driver)))
        goto err_pci;
    return 0;

err_pci:
    class_unregister(&udoomdev_class);
err_class:
    unregister_chrdev_region(udoomdev_devno, ADLERDEV_MAX_DEVICES);
err_chrdev:
    return err;
}

static void udoomdev_exit(void)
{
    pci_unregister_driver(&udoomdev_pci_driver);
    class_unregister(&udoomdev_class);
    unregister_chrdev_region(udoomdev_devno, ADLERDEV_MAX_DEVICES);
}

module_init(udoomdev_init);
module_exit(udoomdev_exit);

