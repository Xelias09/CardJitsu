//
// Created by alexis on 05/06/25.
//
//
// Created by alexis on 03/12/24.
//

#ifndef LG_LIBRARY_H
#define LG_LIBRARY_H

#include <stdlib.h>
#include <stdio.h>

// Quantité de roles présent
typedef struct {
    int quantite[9];  // Quantité disponible de chaque rôle
    const char* roles[9]; // Noms des rôles
} QteRoles;

// Structure des joueurs
typedef struct {
    int pid;                 // PID
    int IDpartie;            // ID partie en cours
    char pseudo[50];         // Pseudo (max 50 caractères)
    char role[20];           // Rôle (max 20 caractères)
    char status[20];         // Statut (max 20 caractères)
    int choix;               // Choix du joueur (ex : joueur a éliminé)
} joueur;

// Structure des parties
typedef struct {
    int idPartie;            // ID de la partie
    int nbJoueurs;           // Nombre de joueurs dans la partie
    int nbJoueursMax;        // Nombre de joueurs maximum
    int status;              // Status de la partie
    int cycle;               // Etat du cycle jour / nuit
    joueur liste_joueur[10]; // Liste des joueurs dans la partie
} partie;

// Structure des messages
typedef struct {
    joueur joueur;           // Joueur (PID / PSEUDO / ROLE)
    partie partie;
    int requete;             // Requête
    char msg[2048];           // Zone de message Texte
} corp;
typedef struct {
    long Type;               // Type de message
    corp Corp;               // Corps du message
} message;

// Règles du jeu :
void regles() {
    system("clear");  // Effacer l'écran
    // Afficher les règles du jeu
    printf("\nRègles du jeu :\n");
    printf("========================================\n");
    printf("Le Loup Garou est un jeu de rôle où les joueurs sont répartis en deux camps :\n");
    printf("- Les Loups Garous : Ils doivent tuer les villageois la nuit.\n");
    printf("- Les Villageois : Ils doivent éliminer les Loups Garous durant la journée.\n");
    printf("\nLe jeu se déroule en alternance entre le jour et la nuit :\n");
    printf("1. La nuit, les Loups Garous choisissent une victime.\n");
    printf("2. Le jour, tous les joueurs votent pour éliminer un suspect.\n");
    printf("Le but du jeu est pour chaque camp de faire éliminer l'autre.\n");
    printf("========================================\n");
    printf("\nAppuyez sur une touche pour revenir au menu...");
    getchar(); // Attendre l'entrée de l'utilisateur pour revenir au menu
}

// Description des rôles :
void _roleLoupGarou() {
    system("clear");
    printf("\n============================================================\n");
    printf("                        RÔLE : LOUP-GAROU                    \n");
    printf("============================================================\n");

    // Section Général
    printf("\nGénéral :\n");
    printf("------------------------------------------------------------\n");
    printf("Le loup-garou est un être mi-homme mi-loup qui se transforme\n");
    printf("la nuit pour tuer les villageois. C'est le personnage\n");
    printf("emblématique du jeu de Loup Garou original.\n\n");
    printf("Le loup-garou incarne l'un des rôles de l'équipe des loups-garous.\n");
    printf("Il connaît l'identité des autres loups-garous et doit essayer\n");
    printf("de tuer tous les villageois sans se faire découvrir. Il se\n");
    printf("réunit chaque nuit avec les autres loups-garous pour décider\n");
    printf("de leur victime. Il gagne si tout le village est éliminé.\n");

    // Section Stratégies
    printf("\nStratégies :\n");
    printf("------------------------------------------------------------\n");

    printf("\nEn tant que Loup-Garou :\n");
    printf("- Discrétion :\n");
    printf("  - Éviter d'attirer l'attention pendant les débats\n");
    printf("  - Participer aux discussions de manière modérée\n");
    printf("  - Ne pas systématiquement défendre les autres loups-garous\n");

    printf("\n- Choix des victimes :\n");
    printf("  - Cibler en priorité les rôles de voyance (Voyante, Montreur d'ours)\n");
    printf("  - Éliminer les joueurs les plus méfiants ou influents\n");
    printf("  - Éviter de tuer les joueurs qui attirent la suspicion du village\n");

    printf("\n- Comportement en journée :\n");
    printf("  - Se comporter comme un villageois convaincu\n");
    printf("  - Voter occasionnellement contre d'autres loups pour paraître crédible\n");
    printf("  - Ne pas hésiter à accuser des villageois de manière argumentée\n");

    printf("\n- Sacrifice tactique :\n");
    printf("  - Accepter d'être éliminé si cela permet de sauver un loup plus\n");
    printf("    important (Infecte Père des Loups)\n");
    printf("  - Particulièrement pertinent si vous êtes déjà suspecté\n");

    printf("\nEn tant que Villageois face aux Loups-Garous :\n");
    printf("- Observation :\n");
    printf("  - Noter les comportements suspects\n");
    printf("  - Repérer les alliances et les défenses systématiques\n");
    printf("  - Être attentif aux votes et aux prises de parole\n");

    printf("\n- Argumentation :\n");
    printf("  - Construire des accusations fondées sur des faits\n");
    printf("  - Partager ses soupçons de manière claire et logique\n");
    printf("  - Éviter les accusations sans preuves qui peuvent décrédibiliser\n");

    printf("\n============================================================\n");
    printf("\nAppuyez sur une touche pour revenir au menu...");
    getchar(); // Attendre l'entrée de l'utilisateur pour revenir au menu
}

void _roleCupidon() {
    system("clear");
    printf("\n============================================================\n");
    printf("                        RÔLE : CUPIDON                      \n");
    printf("============================================================\n");

    // Section Général
    printf("\nGénéral :\n");
    printf("------------------------------------------------------------\n");
    printf("Cupidon est appelé uniquement la première nuit afin d'unir\n");
    printf("un couple. Le Meneur de jeu le contacte le moment venu, puis\n");
    printf("Cupidon désigne deux noms parmi les joueurs. Ces deux personnes\n");
    printf("seront les amoureux. Si un des deux qui sont en couple meurt,\n");
    printf("l'autre meurt avec son amant.\n\n");
    printf("Pour ce faire, Cupidon choisit deux personnes, elles sont alors\n");
    printf("prévenues, et découvrent avec qui elles sont désormais mariées.\n");

    // Section Stratégies
    printf("\nStratégies :\n");
    printf("------------------------------------------------------------\n");
    printf("Il n'existe pas encore de stratégie pour ce rôle.\n");

    printf("\n============================================================\n");
    printf("\nAppuyez sur une touche pour revenir au menu...");
    getchar(); // Attendre l'entrée de l'utilisateur pour revenir au menu
}

void _roleChasseur() {
    system("clear");
    printf("\n============================================================\n");
    printf("                        RÔLE : CHASSEUR                     \n");
    printf("============================================================\n");

    // Section Général
    printf("\nGénéral :\n");
    printf("------------------------------------------------------------\n");
    printf("Le Chasseur n'a aucun rôle particulier à jouer tant qu'il\n");
    printf("est vivant. Mais dès qu'il meurt, qu'il soit tué dans la nuit\n");
    printf("(Loups-garous, sorcière) ou à la suite d'une décision des\n");
    printf("villageois, il doit désigner une personne qui mourra également,\n");
    printf("sur-le-champ, d'une balle de son fusil.\n\n");
    printf("Si un Chasseur amoureux est éliminé, il peut quand même tuer\n");
    printf("une personne, ce qui peut mener à une partie sans survivants,\n");
    printf("puisque trois personnes mourront simultanément.\n\n");
    printf("Le Chasseur peut choisir ou non de tirer sa dernière balle,\n");
    printf("ce qui, dans certains cas, est une réflexion à ne pas prendre\n");
    printf("à la légère.\n");

    // Section Stratégies
    printf("\nStratégies :\n");
    printf("------------------------------------------------------------\n");

    printf("\n- Qui cibler ?\n");
    printf("  - Lorsque son tour sera venu, le Chasseur doit bien se\n");
    printf("    remémorer tous les comportements qui lui ont paru suspects\n");
    printf("    durant la partie. Il convient aussi de prêter attention à\n");
    printf("    ce que disent les joueurs, peut-être que quelqu’un aura une\n");
    printf("    information à transmettre, une Voyante désireuse de donner\n");
    printf("    un élément crucial, par exemple. Attention aux Loups-Garous !\n");

    printf("\n============================================================\n");
    printf("\nAppuyez sur une touche pour revenir au menu...");
    getchar(); // Attendre l'entrée de l'utilisateur pour revenir au menu
}

void _rolePetiteFille() {
    system("clear");
    printf("\n============================================================\n");
    printf("                    RÔLE : PETITE FILLE                     \n");
    printf("============================================================\n");

    // Section Général
    printf("\nGénéral :\n");
    printf("------------------------------------------------------------\n");
    printf("La Petite Fille est un rôle très difficile à jouer. Lorsqu'ils\n");
    printf("sont appelés pendant la nuit, les Loups-Garous peuvent être\n");
    printf("espionnés par elle. La Petite Fille a donc la possibilité\n");
    printf("d'observer et de déduire qui sont les Loups-Garous.\n\n");

    printf("Cependant, ce pouvoir n'est pas sans risque :\n");
    printf("  - Si les Loups-Garous surprennent la Petite Fille en train\n");
    printf("    de les espionner, elle est immédiatement tuée à la place\n");
    printf("    de leur victime désignée.\n\n");

    // Section Stratégies
    printf("\nStratégies :\n");
    printf("------------------------------------------------------------\n");
    printf("Aucune stratégie définie pour ce rôle n'existe encore.\n");
    printf("Cependant, la Petite Fille doit être prudente et attentive :\n");
    printf("  - Espionner discrètement les Loups-Garous sans se faire repérer.\n");
    printf("  - Partager les soupçons avec les villageois sans dévoiler\n");
    printf("    ouvertement son rôle, car cela pourrait la rendre\n");
    printf("    vulnérable.\n");

    printf("\n============================================================\n");
    printf("\nAppuyez sur une touche pour revenir au menu...");
    getchar(); // Attendre l'entrée de l'utilisateur pour revenir au menu
}

void _roleVillageois() {
    system("clear");
    printf("\n============================================================\n");
    printf("                   RÔLE : SIMPLE VILLAGEOIS                 \n");
    printf("============================================================\n");

    // Section Général
    printf("\nGénéral :\n");
    printf("------------------------------------------------------------\n");
    printf("Le Villageois est un personnage de base dans le jeu. Il incarne\n");
    printf("l'habitant ordinaire d'un village et n'a aucun pouvoir spécial.\n");
    printf("Son rôle est d'aider les autres villageois à découvrir\n");
    printf("l'identité des Loups-Garous et à les éliminer avant qu'ils\n");
    printf("ne tuent tous les villageois.\n\n");
    printf("Le Villageois gagne lorsque tous les Loups-Garous sont éliminés.\n");
    printf("Sa seule capacité est de voter au conseil du village contre\n");
    printf("celui qu'il suspecte être un Loup-Garou.\n");

    // Section Stratégies
    printf("\nStratégies :\n");
    printf("------------------------------------------------------------\n");
    printf("1. **Soyez attentif aux indices** :\n");
    printf("   - Observez les comportements des autres joueurs.\n");
    printf("   - Recherchez des indices qui pourraient révéler les Loups-Garous.\n\n");

    printf("2. **Restez discret** :\n");
    printf("   - Ne révélez pas votre rôle immédiatement.\n");
    printf("   - Faites profil bas pour éviter d'attirer l'attention des Loups-Garous.\n\n");

    printf("3. **Collaborez avec les personnages spéciaux** :\n");
    printf("   - Identifiez la Voyante ou la Sorcière et essayez de tirer parti\n");
    printf("     de leurs pouvoirs pour démasquer les Loups-Garous.\n\n");

    printf("4. **Jouez stratégique** :\n");
    printf("   - Utilisez le bluff et la tromperie pour semer le doute parmi les joueurs.\n");
    printf("   - Ne craignez pas de mentir pour déstabiliser les Loups-Garous.\n");

    printf("\n============================================================\n");
    printf("\nAppuyez sur une touche pour revenir au menu...");
    getchar(); // Attendre l'entrée de l'utilisateur pour revenir au menu
}

void _roleSorciere() {
    system("clear");
    printf("\n============================================================\n");
    printf("                        RÔLE : SORCIÈRE                      \n");
    printf("============================================================\n");

    // Section Général
    printf("\nGénéral :\n");
    printf("------------------------------------------------------------\n");
    printf("La sorcière possède deux potions : une de guérison et une d'empoisonnement.\n");
    printf("Elle ne peut utiliser chacune de ses potions qu'une seule fois au cours de la partie.\n");
    printf("Durant la nuit, lorsque les loups-garous se sont rendormis, le meneur de jeu va appeler la sorcière\n");
    printf("et lui montrer la personne tuée par les loups-garous.\n\n");

    printf("La sorcière a trois possibilités :\n");
    printf("  1. Ressusciter la personne tuée et donc perdre sa seule potion de guérison ;\n");
    printf("  2. Tuer une autre personne en plus de la victime et donc perdre sa seule potion d'empoisonnement.\n");
    printf("  3. Ne rien faire.\n\n");

    printf("La sorcière peut utiliser ses deux potions durant la même nuit si elle le souhaite.\n");

    // Section Stratégies
    printf("\nStratégies :\n");
    printf("------------------------------------------------------------\n");
    printf("1. Utiliser sa potion de vie au premier tour offre la garantie de sauver au moins un joueur durant la partie.\n");
    printf("   De cette manière même si la sorcière meurt, elle aura l'assurance d'avoir utilisé son pouvoir pour sauver quelqu'un.\n");
    printf("2. Sauver un autre joueur permet de connaître un innocent.\n");
    printf("   Attention, les Loups peuvent ruser et voter pour l'un d'eux, et le joueur de flûte peut être pris pour cible.\n");
    printf("   Utiliser la potion de vie sur soi-même est un moyen de s'assurer que l'on restera en vie au moins encore un tour de jeu.\n");
    printf("3. La Sorcière a la possibilité de tuer n'importe quel membre du village, même si celui-ci est protégé par le garde.\n");
    printf("   Afin d'optimiser au mieux les chances de tuer un Loup avec la potion de mort, il est conseillé d'utiliser cette potion en fin de partie.\n");
    printf("   Alternativement, elle peut tenter de tuer un joueur suspect plus tôt dans la partie.\n");

    // Section Notes
    printf("\nNotes :\n");
    printf("------------------------------------------------------------\n");
    printf("La sorcière peut se ressusciter elle-même, si elle a été la victime des loups-garous.\n");
    printf("Elle n'opère que durant la nuit, elle ne peut donc pas tuer ou ressusciter quelqu'un durant le jour.\n");
    printf("Si la sorcière a utilisé sa potion de guérison auparavant, le meneur de jeu ne lui désigne plus la victime des loups-garous,\n");
    printf("mais doit continuer à dire à haute voix la phrase 'je montre à la sorcière la victime des loups-garous' afin d'entretenir le doute\n");
    printf("sur l'utilisation des potions. La sorcière peut alors utiliser sa potion d'empoisonnement sur cette même personne, mais celle-ci sera sans effet.\n");

    printf("\n============================================================\n");
    printf("\nAppuyez sur une touche pour revenir au menu...");
    getchar(); // Attendre l'entrée de l'utilisateur pour revenir au menu
}

void _roleVoleur() {
    system("clear");
    printf("\n============================================================\n");
    printf("                        RÔLE : VOLEUR                        \n");
    printf("============================================================\n");

    // Section Général
    printf("\nGénéral :\n");
    printf("------------------------------------------------------------\n");
    printf("Le Voleur reçoit lors de la première nuit la possibilité de choisir son rôle parmi deux autres rôles non-distribués.\n");
    printf("S'il n'en choisit aucun, il est considéré comme un Simple Villageois.\n");
    printf("C'est un personnage dont les capacités varient énormément en fonction du meneur et des joueurs.\n\n");

    printf("Si le voleur est une des deux cartes non distribuées, le meneur de jeu doit faire comme si c'était l'un des joueurs\n");
    printf("et doit faire le même discours que si quelqu'un avait la carte.\n");

    // Section Stratégies
    printf("\nStratégies :\n");
    printf("------------------------------------------------------------\n");
    printf("Il n'existe pas encore de stratégie spécifique pour ce rôle.\n");
    printf("Cependant, le Voleur doit bien réfléchir à son choix de rôle durant la première nuit,\n");
    printf("car cela peut influencer sa stratégie tout au long de la partie.\n");

    printf("\n============================================================\n");
    printf("\nAppuyez sur une touche pour revenir au menu...");
    getchar(); // Attendre l'entrée de l'utilisateur pour revenir au menu
}

void _roleVoyante() {
    system("clear");
    printf("\n============================================================\n");
    printf("                        RÔLE : VOYANTE                       \n");
    printf("============================================================\n");

    // Section Général
    printf("\nGénéral :\n");
    printf("------------------------------------------------------------\n");
    printf("Chaque nuit, la Voyante est appelée par le maître du jeu et peut découvrir la carte d'un joueur.\n");
    printf("La Voyante est l'un des personnages les plus puissants du camp du village.\n");
    printf("Elle pourra facilement identifier les joueurs en qui elle pourra avoir confiance, mais surtout l'identité de ses ennemis.\n");

    // Section Stratégies
    printf("\nStratégies :\n");
    printf("------------------------------------------------------------\n");
    printf("La stratégie de base de la Voyante consiste à se faire discrète jusqu'à ce qu'elle trouve un loup-garou à\n");
    printf("l'aide de son pouvoir de nuit. Ensuite, elle pourra accuser ce joueur pour tenter de l'éliminer.\n");
    printf("En effet, par peur du grand pouvoir de la Voyante, les loups-garous voudront éliminer celle-ci le plus rapidement possible.\n\n");

    printf("Lorsque la Voyante trouve un villageois, elle peut tenter de communiquer discrètement avec lui pendant la journée.\n");
    printf("Le but de la manœuvre est de faire de ce joueur son porte-parole. Le villageois ainsi sondé par la Voyante pourra\n");
    printf("retransmettre les informations et risquer d'attirer le meurtre des loups sur lui.\n");

    printf("\n============================================================\n");
    printf("\nAppuyez sur une touche pour revenir au menu...");
    getchar(); // Attendre l'entrée de l'utilisateur pour revenir au menu
}

void _roleMaire() {
    system("clear");
    printf("\n============================================================\n");
    printf("                        RÔLE : MAIRE / CAPITAINE               \n");
    printf("============================================================\n");

    // Section Général
    printf("\nGénéral :\n");
    printf("------------------------------------------------------------\n");
    printf("Le Maire ou Capitaine est un personnage introduit lors de la deuxième extension, le village.\n");
    printf("Il se fait élire par les Villageois en début de partie. Lorsqu'il vote, son vote compte double.\n\n");

    printf("Dans les règles originelles, le Capitaine est élu par les fermiers (rôles visibles du village). À la mort du dernier fermier,\n");
    printf("il n'y a plus de Capitaine. Il possède deux votes. Cependant, dans les parties, les joueurs ont pris l'habitude de l'élire même\n");
    printf("sans carte ou sans extension.\n\n");

    printf("Le Maire (ou Capitaine) possède un vote très important, car celui-ci compte double. Il est élu par les joueurs le premier tour,\n");
    printf("avant la phase de votes d'élimination. Son rôle face caché peut influencer la partie, car il est choisi par les villageois.\n");

    // Section Stratégies
    printf("\nStratégies :\n");
    printf("------------------------------------------------------------\n");
    printf("Essayez de prendre des décisions justes et raisonnées dès le début de la partie, car ce rôle est très délicat.\n\n");

    printf("Ne votez pas forcément avec vos deux votes sur la même personne, car si vous êtes du camp des villageois et que vous éliminez\n");
    printf("un villageois par vos votes, vous risquez de vous faire éliminer. Soyez juste et en accord avec les villageois pour éviter toute\n");
    printf("ambiguïté et ne pas être confondu avec un loup-garou.\n");

    printf("\n============================================================\n");
    printf("\nAppuyez sur une touche pour revenir au menu...");
    getchar(); // Attendre l'entrée de l'utilisateur pour revenir au menu
}

void Menu_Roles() { // Afficher le menu principal
    int choix = 0;
    while(choix != 10) {
        system("clear");  // Effacer l'écran
        printf("\n========================================\n");
        printf("   Voici les différents rôles présent   \n");
        printf("========================================\n");
        printf("1. Loup-garou\n");
        printf("2. Cupidon\n");
        printf("3. Chasseur\n");
        printf("4. Petite fille\n");
        printf("5. Simple villageois\n");
        printf("6. Sorcière\n");
        printf("7. Voleur\n");
        printf("8. Voyante\n");
        printf("9. Maire\n");
        printf("10. Quitter\n");
        printf("Veuillez entrer votre choix (1-10) : ");
        scanf("%d", &choix);
        getchar(); // Pour vider le buffer après la lecture du choix

        switch (choix) {
            case 1:
                _roleLoupGarou();
            break;
            case 2:
                _roleCupidon();
            break;
            case 3:
                _roleChasseur();
            break;
            case 4:
                _rolePetiteFille();
            break;
            case 5:
                _roleVillageois();
            break;
            case 6:
                _roleSorciere();
            break;
            case 7:
                _roleVoleur();
            break;
            case 8:
                _roleVoyante();
            break;
            case 9:
                _roleMaire();
            break;
            default:
                break;
        }
    }
}

void AffichageLG() {
    // Affichage du loup ASCII
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    printf("@@@@@@@@@@@@@@@@@@@@@@@'~~~     ~~~`@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    printf("@@@@@@@@@@@@@@@@@@'                     `@@@@@@@@@@@@@@@@@@@@\n");
    printf("@@@@@@@@@@@@@@@'                           `@@@@@@@@@@@@@@@@@\n");
    printf("@@@@@@@@@@@@@'                               `@@@@@@@@@@@@@@@\n");
    printf("@@@@@@@@@@@'                                   `@@@@@@@@@@@@@\n");
    printf("@@@@@@@@@@'                                     `@@@@@@@@@@@@\n");
    printf("@@@@@@@@@'                                       `@@@@@@@@@@@\n");
    printf("@@@@@@@@@                                         @@@@@@@@@@@\n");
    printf("@@@@@@@@'                      n,                 `@@@@@@@@@@\n");
    printf("@@@@@@@@                     _/ | _                @@@@@@@@@@\n");
    printf("@@@@@@@@                    /'  `'/                @@@@@@@@@@\n");
    printf("@@@@@@@@a                 <~    .'                a@@@@@@@@@@\n");
    printf("@@@@@@@@@                 .'    |                 @@@@@@@@@@@\n");
    printf("@@@@@@@@@a              _/      |                a@@@@@@@@@@@\n");
    printf("@@@@@@@@@@a           _/      `.`.              a@@@@@@@@@@@@\n");
    printf("@@@@@@@@@@@a     ____/ '   \\__ | |______       a@@@@@@@@@@@@@\n");
    printf("@@@@@@@@@@@@@a__/___/      /__\\ \\ \\     \\___.a@@@@@@@@@@@@@@@\n");
    printf("@@@@@@@@@@@@@/  (___.'\\_______)\\_|_|        \\@@@@@@@@@@@@@@@@\n");
    printf("@@@@@@@@@@@@|\\________                       ~~~~~\\@@@@@@@@@@\n");

    printf("\n");
}

#endif // LG_LIBRARY_H