#/bin/sh

set -e

PROCESS=$(ps -aux | grep qemu | head -n 1 | awk '{print $2}')

if [ -n "$PROCESS" ]; then
  kill "$PROCESS"
  echo "Process deleted !"
else
  echo "Erreur sur la commande ou aucun process trouvé !"
  exit 1
fi
