# Placements affines et coordonnées précises

Cette extension conserve les anciens JSON Visual Map et ajoute une transformation par membre. Elle ne modifie pas les layouts SignalRGB ni les identités OpenRGB. Les liaisons matérielles restent explicites dans le fichier du plan.

## Identité des composants

Un membre peut désigner une zone entière (`zone_idx`, `is_segment: false`) ou un
segment précis (`zone_idx`, `is_segment: true`, `segment_idx`). Les champs sont au
même niveau que `controller` et `settings`. Deux segments d'une même zone restent
deux membres indépendants dans le chargement, la comparaison, l'ajout, le retrait
et la sauvegarde. Leurs coordonnées et gains ne sont pas fusionnés.

Sans `is_segment`, un ancien JSON désigne une zone entière. L'index d'un segment
doit être explicite et non négatif ; un index manquant ne sélectionne pas le
premier composant. Les anciens fichiers qui avaient perdu l'identité d'un segment
doivent donc la récupérer depuis un inventaire vérifié. Le routage des segments
additionne le début de la zone et le décalage du segment dans celle-ci, y compris
pour l'identification visuelle des composants. Ils ne deviennent pas des surfaces
image natives indépendantes.

## Schéma JSON

```json
"settings": {
  "shape": 2,
  "x": 10.25,
  "y": 20.5,
  "scale": 1,
  "led_spacing": 1,
  "reverse": false,
  "affine": {
    "scale_x": 2,
    "scale_y": 3,
    "rotation": 37,
    "flip_x": true,
    "flip_y": false
  },
  "point_origin": "cell",
  "brightness": 1,
  "custom_shape": {
    "w": 16.25,
    "h": 6.5,
    "led_positions": [
      {"led_num": 0, "x": 0.125, "y": 1.875},
      {"led_num": 1, "x": 0.125, "y": 1.875}
    ]
  }
}
```

Les deux LED de l'exemple partagent une coordonnée mais conservent leurs indices indépendants. Aucun dédoublonnage par position ne fusionne les anneaux de ventilateurs. Les dimensions et les coordonnées personnalisées restent des nombres réels, y compris après sauvegarde et clonage.

`scale_x` et `scale_y` multiplient l'échelle uniforme historique `scale`. Les valeurs par défaut sont 1, rotation 0, miroirs désactivés. Le pivot est le centre de la surface déjà dimensionnée : `(w × scale × scale_x / 2, h × scale × scale_y / 2)`. Les miroirs sont appliqués autour du centre local avant rotation. La rotation positive est horaire dans le repère écran où Y croît vers le bas. `x/y` restent le coin supérieur gauche **avant rotation**, et ne sont pas remplacés par le coin de la boîte englobante tournée.

`brightness` est un gain normalisé de 0 à 1, multiplié avec celui du plan et du producteur d'image. Pour importer un pourcentage, diviser par 100. Les échelles doivent être positives et les coordonnées, angles et gains finis ; les entrées invalides sont refusées.

`point_origin: "cell"` conserve la convention Visual Map historique : `(x,y)` est le coin de la cellule LED unitaire, échantillonnée en son centre `(x+0.5,y+0.5)`. L'option explicite `"center"` échantillonne la coordonnée elle-même. La géométrie du plan SignalRGB établit les pivots, les échelles et les miroirs ; elle ne prouve pas à elle seule sa convention interne de centre de pixel. Un import ne doit donc pas annoncer une équivalence du sampler Signal sur cette seule base. Le défaut reste `cell`.

Un ancien fichier sans ces champs retrouve exactement ses valeurs historiques. Un ancien exécutable Visual Map ne connaît pas les nouveaux champs et peut les perdre lors d'une sauvegarde : utiliser ce fork pour éditer les plans affines.

## Rendu et routage communs

`LedRouting::LocalTransform` est la source géométrique commune au rendu, à la pondération LED et au routage image. Les cellules tournées sont de vrais polygones ; leur recouvrement est découpé pixel par pixel, sans colorer les coins vides de leur rectangle englobant. Le chemin axis-aligned historique reste direct. La luminosité par membre s'applique aux deux chemins.

Une matrice dense dont toutes les cellules forment une affine conserve la référence à l'image source et transmet une transformation composée au pilote natif. Les formes trouées, irrégulières, partielles ou aux positions superposées restent des routes LED indépendantes. Les échantillons LED du chemin image et la moyenne de surface du chemin LED historique restent deux méthodes distinctes, comme avant cette extension.

L'éditeur expose échelles X/Y, rotation, miroirs et luminosité par membre. Le redimensionnement à la souris conserve l'affine et déplace correctement son point d'ancrage. L'auto-dimensionnement du plan prend la boîte tournée en compte. L'éditeur de formes préserve les points fractionnaires inchangés ; déplacer explicitement une LED garde son accrochage à la grille.

## Vérification

Les tests `tests/room-image-routing/Build-Tests.cmd` utilisent Qt 6.8.3, MSVC x64 et le rendu offscreen. Ils vérifient les pivots/non-uniformités/miroirs, le JSON ancien et nouveau, les fractions, les positions dupliquées, les gains, une cellule en diamant qui ne lit pas les coins de sa boîte englobante, le rendu réel `ControllerZoneItem`, et la transmission d'une affine composée à un destinataire synthétique via le vrai wrapper du cœur. Aucun matériel ni capture d'écran n'est sollicité.

La DLL complète compile après ces tests. L'export des identités matérielles et l'observation physique d'un layout importé constituent des vérifications séparées.

Le test DLL réel du cœur (`OpenRGB-Room/tests/room-plugin-images`) inclut deux
segments d'une même zone non initiale, à des positions et gains différents, en plus du
destinataire image natif. Le parseur de test accepte aussi
`routing-tests.exe --validate-map <fichier>` pour vérifier les identités uniques
et tous les réglages avec le vrai codec JSON, sans lier ni ouvrir de matériel.
