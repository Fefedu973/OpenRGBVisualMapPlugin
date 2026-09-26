# Visual Map : plans et images haute résolution

Cette branche ajoute une sortie image facultative à chaque contrôleur Visual Map, avec le contrat secondaire `room_image::RGBControllerImageInterface` du cœur [OpenRGB-Room](https://github.com/Fefedu973/OpenRGB). L’API de plugin historique reste en version 5 ; l’extension `room_image::PluginAPI` est détectée par capacité à l’exécution. Les paquets image SDK du cœur utilisent leur propre négociation de protocole. Ces numéros ne sont pas interchangeables.

Sur le réseau, cette branche emploie l’extension **Room SDK 7**, le bit de capacité **29** et le marqueur **RIMG, schéma 1**. Il faut vérifier les trois ; le numéro 7 seul ne permet pas d’identifier ce format. Ces identifiants appartiennent à ce fork et ne constituent pas une allocation approuvée par OpenRGB amont. Le format historique des commandes LED n’est pas étendu avec des pixels supplémentaires. Voir le [contrat wire du cœur](https://github.com/Fefedu973/OpenRGB/blob/room-integration/FrameRouting/README.md).

## Trois dimensions indépendantes

| Dimension | Usage |
|---|---|
| Plan, par exemple 800×600 | Coordonnées des placements enregistrés ; dimensions annoncées par la sortie image |
| Image source | Résolution fournie par un effet/capture/client ; elle peut différer du plan |
| Grille LED de compatibilité | Anciennes commandes LED ; maximum 127 sur le grand côté, proportions conservées |

Un plan 800×600 utilise ainsi une grille de compatibilité de 127×95 cellules au maximum, **pas 480 000 objets LED**. Une cellule vide ne crée pas de LED. Les plans dont les deux côtés sont au plus 127 conservent leur grille et leur pondération historique. Un ancien plan 128×128 ou plus garde ses coordonnées et son JSON, mais sa grille LED est réduite lorsque l’extension image est disponible. Le plafond de 127 respecte aussi la taille canonique du champ de matrice SDK : `8 + 4 × 127² < 65536`. Sur un hôte sans extension image, le chemin et les dimensions LED historiques restent disponibles.

Les contrôles existants de taille, placement, échelle, inversion et forme personnalisée sont conservés. La plage du plan reste 1…1024 par axe. Les valeurs importées hors plage sont bornées en mémoire.

## Routage géométrique

Chaque nouvelle image est conservée sous forme d’un `Frame` BGRA opaque immuable, partagé entre les destinataires. Le producteur fournit un `Mapping` affine vers sa scène ; Visual Map compose ce mapping avec le placement de chaque membre. Il n’existe aucune exception fondée sur un nom ou modèle d’appareil.

- **Surface native entière** : une matrice complète d’au moins 2×2 cellules, sans trou, dont le placement personnalisé correspond à une affine rectangulaire reçoit le même `Frame` partagé et une affine composée. Les rotations et miroirs enregistrés dans les coordonnées personnalisées sont conservés. La luminosité du plan est multipliée dans le mapping. Une matrice à une seule ligne/colonne reste en routage LED : elle ne fournit pas assez de points indépendants pour reconstituer la surface.
- **LED physiques** : un échantillon bilinéaire est pris au centre de chaque cellule réelle placée, puis écrit à son indice dans la zone ou le segment. Les positions peuvent être irrégulières, inversées ou espacées. Les coordonnées hors image sont noires.
- **Forme non affine, masque troué ou segment** : elle reste en routage LED, même si le contrôleur propose par ailleurs une sortie image entière. Une forme partielle ne devient jamais implicitement un rectangle plein.
- **Ancien flux LED** : la moyenne pondérée par recouvrement des cellules est conservée. Pour un grand plan, les positions sont projetées dans la grille bornée avant cette pondération.

L’échantillonnage au centre du chemin image peut différer de la moyenne de surface du chemin LED historique ; cette distinction est explicite. Le résultat reste spatial, sans uniformiser toute une touche ou tout un écran. Un fournisseur natif `Busy` ou `Invalid` ne déclenche pas d’écriture LED concurrente. Seul `Unsupported` autorise ce repli.

Exemple purement géométrique pour deux sorties natives différentes dans un plan 800×600 :

| Sortie déclarée par le pilote | Matrice de placement | Placement dans le plan | Mapping vers l’image source du plan |
|---|---|---|---|
| 1920×1080, 30 images/s | 32×18 | x=80, y=60, échelle=10 | origine=(0,1 ; 0,1), u=(0,4 ; 0), v=(0 ; 0,3) |
| 480×800, 20 images/s | 18×30 | x=500, y=200, échelle=10 | origine=(0,625 ; 1/3), u=(0,225 ; 0), v=(0 ; 0,5) |

Les deux reçoivent le même `Frame` 800×600 et des mappings distincts ; chaque pilote produit sa résolution native. Les tailles et cadences viennent des capacités annoncées, sans changement de code pour ces exemples. Ces valeurs illustrent le calcul, sans constituer une mesure de performance ou une validation matérielle de ces écrans.

## Cadence, durée de vie et interface

`SubmitImage` valide puis remplace une mailbox unique sans attendre un appareil. Un worker traite au plus 60 images/s et respecte le plafond annoncé par chaque sortie native. Les frames intermédiaires sont remplacées ; aucune file croissante n’est constituée. La cadence physique dépend toujours du pilote destinataire et du matériel.

Une image bénéficie d’un bail de 100…5000 ms. Pendant ce bail, les écritures LED historiques du plan sont suspendues ; après expiration, elles reprennent. L’aperçu core reçoit seulement un pointeur immuable + mapping et indique l’expiration. L’aperçu du plan est limité à 320×200 et environ 15 rafraîchissements/s, puis projeté selon les coordonnées du plan. Aucune copie d’image pleine par périphérique n’est nécessaire dans le routage natif.

Les listes de membres et plans de routage sont protégés ; les coordonnées modifiables par la souris sont transformées en plans immuables pour le worker lors du changement de géométrie. La détection des périphériques vide et draine ces plans avant de libérer les contrôleurs. Les cycles entre plans sont refusés par identité des contrôleurs : un plan ne peut pas se contenir directement ou indirectement. Les plans imbriqués sans cycle restent possibles.

À l’arrêt, le sink image est détaché du wrapper core avant destruction de son objet, la mailbox est fermée et le worker est joint. Le wrapper est ensuite désenregistré et supprimé via l’API du cœur. Cela requiert aussi le correctif core qui annule les opérations d’inscription différées ; un ancien cœur avec des threads d’inscription détachés ne fournit pas cette garantie de durée de vie. Les rappels de reconstruction UI passent directement si le callback est déjà sur le thread GUI, pour éviter de bloquer ce thread sur lui-même. Les tests ne sollicitent aucun pilote ni appareil réel.

## Compilation

Utiliser le même compilateur/kit Qt que le cœur. Exemple depuis un terminal MSVC configuré :

```bat
mkdir .build
cd .build
qmake ..\OpenRGBVisualMapPlugin.pro OPENRGB_ROOM_ROOT=../../OpenRGB-Room QMAKE_STREAM_EDITOR=sed CONFIG+=release CONFIG-=debug CONFIG-=debug_and_release
nmake
```

`OPENRGB_CORE_DIR` est un alias accepté, prioritaire lorsqu’il est renseigné. Il faut un cœur contenant `FrameRouting/OpenRGBImagePluginAPI.h` ; aucun checkout de sous-module supplémentaire n’est nécessaire avec le chemin frère. Le helper `Build-Room.cmd` accepte `QT_ROOT`, `OPENRGB_CORE_DIR` et `VCVARS`. Git for Windows fournit `sed` ; son dossier est ajouté après les outils MSVC pour éviter de sélectionner le programme Unix `link` à la place de l’éditeur de liens Microsoft.

## Validation et limites

Le harness `tests/room-image-routing` compile les vraies classes `VirtualController`, `RGBController_Virtual`, `RGBController`, `ImageRouting` et `LedRouting`. Seuls l’API de plugin, les contrôleurs matériels et la journalisation sont remplacés. Il couvre le plan 800×600, les limites SDK, affines composées/rotation, inversions, formes non affines/trouées, pondération LED historique, propriété des pixels, leases, mailbox, sorties indisponibles et nettoyage.

Le [harness QPluginLoader du cœur](https://github.com/Fefedu973/OpenRGB/blob/room-integration/tests/room-plugin-images/README.md) a aussi chargé les vraies DLL Visual Map et Effects. Pour Visual Map, un seul sink synthétique de matrice 32×18 annonçait une sortie 800×600 : l’attachement RTTI secondaire entre DLL et exécutable, la transmission du même `shared_ptr`, l’affine `(0,1 ; 0,1 ; 0,4 ; 0,3)`, l’aperçu et le déchargement ont réussi, sans écriture LED. Effects a passé Load/GetWidget/Unload sans effet actif. Ce test ne couvre ni une capture réelle, ni une mesure de cadence, ni deux appareils physiques.

Compilation Windows MSVC/Qt 6.8.3 et tests logiciels réalisés le 27 septembre 2026. Aucune capture réelle, aucun contrôle matériel et aucune validation optique de cette branche Visual Map n’ont été effectués. Une API secondaire disponible ne constitue pas une preuve de compatibilité matérielle de tous les pilotes. Les paquets publics amont précompilés n’incluent pas cette extension.
