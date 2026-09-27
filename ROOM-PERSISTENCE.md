# Cartes persistantes et profils OpenRGB

Les changements de position, de forme, de transformation et d'options d'une carte
déjà enregistrée sont sauvegardés après 500 ms sans nouvelle modification. Le
fichier canonique reste dans `plugins/settings/virtual-controllers/`. Une carte
sans fichier demande encore un premier **Save** pour choisir son nom.

L'état en mémoire est actualisé immédiatement : une reconstruction de la liste
des périphériques ne recharge plus les anciennes coordonnées. Les membres
temporairement absents et les métadonnées importées sont conservés. Une suppression
explicite dans l'éditeur reste une suppression. Les fichiers sont écrits avec
`QSaveFile` ; un échec est signalé, ne remplace pas le fichier précédent et laisse
les changements en mémoire pour une nouvelle tentative.

Un profil OpenRGB sélectionne le fichier, sans embarquer d'anciennes positions :

```json
{
  "plugins": {
    "OpenRGB Visual Map Plugin": {
      "version": 1,
      "active_map": "Music - Tri Band.json"
    }
  }
}
```

`active_map` peut être tout nom de carte locale existante, par exemple
`Full Scale.json`, ou `null` pour ne laisser aucune carte émettrice. Les chemins
externes ne sont pas acceptés. Les fichiers actuels sont relus après sauvegarde
des éditions en attente ; charger un vieux profil ne remet donc pas d'anciennes
positions. Un ancien profil sans cette section, ou avec la valeur historique
`null`, conserve la carte active. Un fichier référencé manquant ou invalide est
signalé et laisse les sorties des cartes désactivées.

Une seule carte émet à la fois. Le changement suspend et draine l'ancien routage
LED/image et invalide ses images en attente avant d'activer la nouvelle carte.
Les cartes inactives et leurs fichiers restent disponibles. Le choix est aussi
enregistré dans `plugins/settings/visual-map-workspace.json` et repris au prochain
chargement du plugin, sans réécrire les options `auto_load`/`auto_register` des
cartes. Les effets et leurs états sont gérés par leur propre section du profil
OpenRGB ; Visual Map ne crée ni ne démarre d'effet.

Les hooks de profil exécutent leurs opérations synchrones sur le thread GUI.
Le signal core `ACTIVE_PROFILE_CHANGED` termine une transition, y compris pour
un profil ancien dépourvu de section Visual Map. Le checkpoint de session du
fork core utilise le même contrat.

## Validation sans matériel

Construire la DLL avec `Build-Room.cmd`, puis lancer
`tests/room-persistence/Build-Tests.cmd` avec `QT_ROOT` configuré. Le test charge la
vraie DLL par `QPluginLoader`, utilise les vrais widgets Qt hors écran, un répertoire
temporaire et des sorties synthétiques. Il couvre les coordonnées décimales,
l'autosave, une reconstruction avant le délai de sauvegarde, les membres absents,
la relecture canonique, les profils anciens, le blocage des sorties LED/image
inactives et la reprise de la sélection après déchargement/rechargement de la DLL.
Ces tests ne constituent pas une validation de la session matérielle de l'utilisateur.
