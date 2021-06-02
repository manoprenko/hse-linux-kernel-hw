#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/atomic.h>
#include <linux/interrupt.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Spying kernel module");
MODULE_VERSION("0.01");

enum {
  INTERVAL_SECONDS = 30
};

static void timer_routine(struct timer_list *);

static atomic_t cnt = ATOMIC_INIT(0);
static int last_cnt = 0;
static int my_err = 0;
static DEFINE_TIMER(timer, timer_routine);

static irqreturn_t handler(int irq, void *dev_id) {
  if (dev_id != &cnt) {
    return IRQ_NONE;
  }

  atomic_inc(&cnt);
  return IRQ_HANDLED;
}

static void setup_timer(void) {
  timer.expires = jiffies + INTERVAL_SECONDS * HZ;
  add_timer(&timer);
}

static void timer_routine(struct timer_list *data) {
  int current_cnt = atomic_read(&cnt);
  printk(KERN_INFO "Keys pressed in last minute: %d\n", current_cnt - last_cnt);
  last_cnt = current_cnt;
  setup_timer();
}

static int __init lkm_spy_init(void) {
  setup_timer();

  my_err = request_irq(
      1,
      handler,
      IRQF_SHARED,
      "test_memes",
      &cnt
  );

  printk(KERN_INFO "Spy deploed\n");

  return 0;
}

static void __exit lkm_spy_exit(void) {
  printk(KERN_INFO "Spy fired\n");

  if (my_err == 0) {
    free_irq(1, &cnt);
  }
  del_timer_sync(&timer);
}

module_init(lkm_spy_init);
module_exit(lkm_spy_exit);
