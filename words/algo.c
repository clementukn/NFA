/* MOT */
/* Fonction qui créée un automate basique, sans retour intelligent */
void createAutoBasique(char * mot, state * debut) {
	if (mot[0] != '\0')
	{
		transition * newTrans;
		state * newState;

		newTrans = createNewTransition(mot[0]);

		if (mot[1] == '\0')
			newState = createNewState(VRAI);
		else
			newState = createNewState(FAUX);
		
		debut->transitions = newTrans;
		newTrans->suivant = newState;
		/* On affiche la transition */
		afficheTrans(newTrans, debut);
		createAutoBasique(mot+1, newState);
	}	
}


/* MOT */
/* Fonction qui créé des retour intelligents dans l'automate : l'algorithme est basé sur la reconnaissance d'une symétrie dans le motif
	On recherhe des calques de valeurs supérieurs à 1 et inférieur à la taille du mot présentant une identité à un près du mot. 
	Le caractère qui diffère sera le caractère de la transition */
void createAutoLink(char * calque, char * mot, state * debut) {
	if (strlen(calque) > 0)
	{
		/* l : position dans le mot */
		int l = 1, i, j;				
		int lgmot = (signed)strlen(mot);
		/* Curseur se déplace dans l'automate en fonction du mot */
		state * curseur;			
		char * motTemp;
		motTemp = mot;
	
		while (l <= (lgmot - strlen(calque) + 1) ){
			curseur = debut;
			motTemp = motTemp + 1;

			/* Positionnement du curseur */
			for (j = 0; j < l; j++)
				curseur = (curseur->transitions)->suivant;
			
			i = 0;
			while ((calque[i] == motTemp[i]) && (i <= (signed)strlen(calque) - 2)) {
				curseur = (curseur->transitions)->suivant;
				i++;
			}
			
			if ((calque[i] != motTemp[i]) && (i == (signed)strlen(calque) - 1))
			{
			    BOOLEAN isDetected  = FAUX;
				/* Vérification de l'existence d'une même transition */
				/* S'il n'y a aucune transition, on la créé */
				if (curseur->transitions != NULL)
				{
					/* Pointeur temporaire pour scanner les transitions */
					transition * noeudScan;	

					noeudScan = curseur->transitions;
					/* On vérifie qu'on ne va pas surcharger l'état */
					do
					{
						if (noeudScan->symbole == calque[i])
							isDetected = VRAI;
						noeudScan = noeudScan->suivante;
					} while (noeudScan != NULL);
				}
				if (isDetected == FAUX)
				{
					transition * newTrans, *transTemp;
					/* se déplace dans l'automate en fonction du calque */
					state * noeudTemp;		
					noeudTemp = debut;
					
					/* Positionnement du noeudTemp */
					for (j = 0; j < (signed)strlen(calque); j++)
						noeudTemp = (noeudTemp->transitions)->suivant;
						
					newTrans = (transition *)malloc(sizeof(transition));
					newTrans->symbole = calque[i];
					newTrans->suivant = noeudTemp;
					newTrans->suivante = NULL; 
					
					/* Si c'est l'état final, on créé une transition */
					if (curseur->transitions == NULL)		
						curseur->transitions = newTrans;
					else
					{
						transTemp = curseur->transitions;
					/* Recherche d'une transition vide */
						while (transTemp->suivante != NULL)
							transTemp = transTemp->suivante;
						transTemp->suivante = newTrans;
					}
					afficheTrans(newTrans, curseur);
				}
				l++;
			}
			else
				l++;
		}
		/* On créé le calque d'ordre n-1 */
		calque[strlen(calque) - 1] = '\0';
		/* Appel récursif avec un calque d'ordre n-1 */
		createAutoLink(calque, mot, debut);
	}
}
