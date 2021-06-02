#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Phonebook kernel module");
MODULE_VERSION("0.01");

#define DEVICE_NAME "phonebook"

/* Prototypes for device functions */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

/* This structure points to all of the device functions */
static struct file_operations file_ops = {
 .read = device_read,
 .write = device_write,
 .open = device_open,
 .release = device_release
};

static int major_num;
static int device_open_count = 0;

#define MAX_LEN 50
#define BUF_SIZE 500

/* Phonebook management utilities */

struct phonebook_entry {
  char *name;
  char *surname;
  int age;
  char *phone_number;
  char *email;

  struct list_head phonebook_list;
};

static LIST_HEAD(phonebook);

static char query_surname[MAX_LEN + 1];
static struct list_head *current_list_entry = NULL;


static char *copy_string(char *str) {
  char *new_str = kmalloc(strlen(str) + 1, GFP_KERNEL);
  strcpy(new_str, str);
  return new_str;
}

static void find_next_entry(void) {
  struct phonebook_entry *current_entry = NULL;

  do {
    current_list_entry = current_list_entry->next;
    if (current_list_entry == &phonebook) {
      break;
    }

    current_entry = list_entry(current_list_entry, struct phonebook_entry, phonebook_list);
  } while (strcmp(current_entry->surname, query_surname) != 0);
}

static void add_entry(char *name, char *surname, int age, char *phone_number, char *email) {
  struct phonebook_entry *new_entry;
  new_entry = kmalloc(sizeof(struct phonebook_entry), GFP_KERNEL);
  new_entry->name = copy_string(name);
  new_entry->surname = copy_string(surname);
  new_entry->age = age;
  new_entry->phone_number = copy_string(phone_number);
  new_entry->email = copy_string(email);

  printk(KERN_INFO "Added new entry: %s %s %d %s %s", name, surname, age, phone_number, email);
  list_add(&new_entry->phonebook_list, &phonebook);
}

static int delete_entry(char *phone_number) {
  struct phonebook_entry *ptr;

  list_for_each_entry(ptr, &phonebook, phonebook_list) {
    if (strcmp(phone_number, ptr->phone_number) == 0) {
      list_del(&ptr->phonebook_list);
      kfree(ptr->name);
      kfree(ptr->surname);
      kfree(ptr->phone_number);
      kfree(ptr->email);
      kfree(ptr);

      printk(KERN_INFO "Deleted entry with phone number %s\n", phone_number);
      return 0;
    }
  }

  printk(KERN_INFO "No entries found with phone number %s\n", phone_number);
  return 1;
}


/* When a process reads from our device, this gets called. */
static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset) {
  char buf[BUF_SIZE];
  int msg_len;
  int i;
  struct phonebook_entry *current_entry;

  if (current_list_entry == NULL) {
    return 0;
  }

  find_next_entry();
  if (current_list_entry == &phonebook) {
    current_list_entry = NULL;
    return 0;
  }


  current_entry = list_entry(current_list_entry, struct phonebook_entry, phonebook_list);
  sprintf(buf, "%s %s %d %s %s\n",
      current_entry->name,
      current_entry->surname,
      current_entry->age,
      current_entry->phone_number,
      current_entry->email
  );
  msg_len = strlen(buf);
  for (i = 0; i < msg_len && i < len; ++i) {
    put_user(buf[i], buffer++);
  }
  return i;
}

/* Called when a process tries to write to our device */
static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset) {
  char name[MAX_LEN + 1];
  char surname[MAX_LEN + 1];
  int age;
  char phone_number[MAX_LEN + 1];
  char email[MAX_LEN + 1];

  current_list_entry = NULL;

  if (sscanf(buffer, "add %50s %50s %d %50s %50s", name, surname, &age, phone_number, email) == 5) {
    add_entry(name, surname, age, phone_number, email);
    return len;
  }

  if (sscanf(buffer, "del %50s", phone_number) == 1) {
    if (delete_entry(phone_number) == 0) {
      return len;
    } else {
      return -EINVAL;
    }
  }

  if (sscanf(buffer, "get %50s", surname) == 1) {
    strcpy(query_surname, surname);
    current_list_entry = &phonebook;
    return len;
  }

  return -EINVAL;
}

/* Called when a process opens our device */
static int device_open(struct inode *inode, struct file *file) {
  /* If device is open, return busy */
  if (device_open_count) {
    return -EBUSY;
  }
  device_open_count++;
  try_module_get(THIS_MODULE);
  return 0;
}

/* Called when a process closes our device */
static int device_release(struct inode *inode, struct file *file) {
  /* Decrement the open counter and usage count. Without this, the module would not unload. */
  device_open_count--;
  module_put(THIS_MODULE);
  return 0;
}

static int __init lkm_phonebook_init(void) {
  major_num = register_chrdev(0, "phonebook", &file_ops);
  if (major_num < 0) {
    printk(KERN_ALERT "Could not register device: %d\n", major_num);
    return major_num;
  } else {
    printk(KERN_INFO "lkm_phonebook: module loaded with device major number %d\n", major_num);
    return 0;
  }
}

static void __exit lkm_phonebook_exit(void) {
 /* Remember - we have to clean up after ourselves. Unregister the character device. */
 unregister_chrdev(major_num, DEVICE_NAME);
}

/* Register module functions */
module_init(lkm_phonebook_init);
module_exit(lkm_phonebook_exit);
