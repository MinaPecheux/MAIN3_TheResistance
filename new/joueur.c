#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pthread.h>

#include <gtk/gtk.h>

pthread_t server_thread;
int server_thread_ret;
int localServerThreadPortno;
char server_thread_buffer[1000];

char mainServerAddr[100];
char mainServerPort[100];
char localServerAddr[100];
char localServerPort[100];
char username[100];

GtkWidget *labelAddrServer;
GtkWidget *labelPortServer;
GtkWidget *labelUsername;
GtkWidget *labelMissionNumber;
GtkWidget *labelMissionMeneur;
GtkWidget *labelMissionNbj;
GtkWidget *labelMissionWin;
GtkWidget *labelMissionLose;
char mission_number_text[50];
char mission_meneur_text[50];
int mission_nbj = -1;
char mission_nbj_text[50];
char mission_win_text[5];
char mission_lose_text[5];
GtkWidget *labelPlayer[8];
GtkWidget *rolePlayer[8];
GtkWidget *checkboxPlayer[8];
GtkWidget *votePlayer[8];
GtkWidget *radiovotePlayer[2];
GtkWidget *boutonProposition;

GtkTextBuffer *buffer;
GtkWidget *text_view;
GtkWidget *scrolled_window;

void sendMessageToMainServer(char *mess);

void *server_func(void *ptr)
{
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    // Cree le socket

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
	      exit(1);
    }

    // Bind le socket avec server_thread_portno

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(localServerThreadPortno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    // Ecoute sur la socket

    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
          printf("ERROR on accept\n");
	        exit(1);
        }

        bzero(server_thread_buffer,256);
        n = read(newsockfd,server_thread_buffer,255);
        if (n < 0) {
      	  printf("ERROR reading from socket\n");
      	  exit(1);
        }

        printf("received from main_server '%s'\n",server_thread_buffer);

      	if (server_thread_buffer[0]=='1') {
      		int j_index = 0;
      		sscanf(server_thread_buffer, "1 %d", &j_index);
      		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkboxPlayer[j_index]), TRUE);
      		gtk_widget_set_sensitive (radiovotePlayer[0], TRUE);
      		gtk_widget_set_sensitive (radiovotePlayer[1], TRUE);
      	}
      	else if (server_thread_buffer[0]=='0') {
      		int j_index = 0;
      		sscanf(server_thread_buffer, "0 %d", &j_index);
      		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkboxPlayer[j_index]), FALSE);
      		gtk_widget_set_sensitive (radiovotePlayer[0], TRUE);
      		gtk_widget_set_sensitive (radiovotePlayer[1], TRUE);
      	}
      	else if (server_thread_buffer[0]=='2') {
      		int i;
      		for (i = 0; i < 5; i++) 
      			gtk_widget_set_sensitive (checkboxPlayer[i], TRUE);
      		gtk_widget_set_sensitive (boutonProposition, TRUE);

        	char nom[100];
        	sscanf(server_thread_buffer , "2 %s", nom);
        	sprintf(mission_meneur_text, "Meneur : %s", nom);
        	gtk_label_set_text ((GtkLabel*)labelMissionMeneur, mission_meneur_text);

      		gtk_button_set_label(GTK_BUTTON(radiovotePlayer[0]), "Oui");
      		gtk_button_set_label(GTK_BUTTON(radiovotePlayer[1]), "Non");
      	}
      	else if (server_thread_buffer[0]=='3') {
      		int i;
      		for (i = 0; i < 5; i++)
      			gtk_widget_set_sensitive (checkboxPlayer[i], FALSE);
      		gtk_widget_set_sensitive (boutonProposition, FALSE);

        	char nom[100];
        	sscanf(server_thread_buffer , "3 %s", nom);
        	sprintf(mission_meneur_text, "Meneur : %s", nom);
        	gtk_label_set_text ((GtkLabel*)labelMissionMeneur, mission_meneur_text);

      		gtk_button_set_label(GTK_BUTTON(radiovotePlayer[0]), "Oui");
      		gtk_button_set_label(GTK_BUTTON(radiovotePlayer[1]), "Non");
      	}
      	else if (server_thread_buffer[0]=='4')
      		gtk_widget_set_sensitive (boutonProposition, TRUE);
      	else if (server_thread_buffer[0]=='5')
      		gtk_widget_set_sensitive (boutonProposition, FALSE);
      	else if (server_thread_buffer[0]=='6') {
      		int index;
      		sscanf(server_thread_buffer, "6 %d", &index);
      		gtk_label_set_text ((GtkLabel*)rolePlayer[index], "Rebelle");
      	}
      	else if (server_thread_buffer[0]=='7') {
      		int indexEspion1, indexEspion2;
      		sscanf(server_thread_buffer, "7 %d %d", &indexEspion1, &indexEspion2);
      		gtk_label_set_text ((GtkLabel*)rolePlayer[indexEspion1], "Espion");
      		gtk_label_set_text ((GtkLabel*)rolePlayer[indexEspion2], "Espion");

      		int i;
      		for (i = 0; i < 5; i++) {
      			if(i != indexEspion1 && i != indexEspion2)
      				gtk_label_set_text ((GtkLabel*)rolePlayer[i], "Rebelle");
      		}
      	}
      	else if (server_thread_buffer[0]=='C') {
        	char connect;
        	char nom[100];
        	int index;

            printf("Commande C\n");
            sscanf ( server_thread_buffer , "%c %s %d" , &connect, nom, &index);
		
		    printf("nom=%s index=%d\n",nom, index);

		    gtk_label_set_text ((GtkLabel*)labelPlayer[index], nom);
	    }
      else if (server_thread_buffer[0]=='M') {
      		int mission_nb = 0;
            sscanf (server_thread_buffer , "M %d %d", &mission_nb, &mission_nbj);

            sprintf(mission_number_text, "Mission n°%d", mission_nb);
            sprintf(mission_nbj_text, "Nombre de participants : %d", mission_nbj);
            gtk_label_set_text ((GtkLabel*)labelMissionNumber, mission_number_text);
            gtk_label_set_text ((GtkLabel*)labelMissionNbj, mission_nbj_text);
 	    }
      else if (server_thread_buffer[0]=='V') {
        char vote;
        int playerId;
        sscanf(server_thread_buffer, "V %c %d", &vote, &playerId);

        if(vote == 'o')
          gtk_label_set_text ((GtkLabel*)votePlayer[playerId], "O");
        else
          gtk_label_set_text ((GtkLabel*)votePlayer[playerId], "X");
      }
      else if (server_thread_buffer[0]=='L') {
        int role, in_team;
        sscanf(server_thread_buffer, "L %d %d", &role, &in_team);

        int i;
        for(i = 0; i < 5; i++)
          gtk_label_set_text((GtkLabel*)votePlayer[i], "--------");

        if(in_team) {
          gtk_button_set_label(GTK_BUTTON(radiovotePlayer[0]), "Succes");
          gtk_button_set_label(GTK_BUTTON(radiovotePlayer[1]), "Echec");
          gtk_widget_set_sensitive (radiovotePlayer[0], TRUE);
          if(role)
            gtk_widget_set_sensitive (radiovotePlayer[1], TRUE);
        }
      }
      else if (server_thread_buffer[0]=='N') {
        int w, l;
        sscanf(server_thread_buffer, "N %d %d", &w, &l);

        sprintf(mission_win_text, "%d", w);
        sprintf(mission_lose_text, "%d", l);
        gtk_label_set_text ((GtkLabel*)labelMissionWin, mission_win_text);
        gtk_label_set_text ((GtkLabel*)labelMissionLose, mission_lose_text);
      }
      else if (server_thread_buffer[0]=='m') {
        char separator = '_';
        char msg[100];
        char msg_clean[100];
        sscanf (server_thread_buffer , "m %s", msg);
        int i;
        for(i = 0; i < strlen(msg); i++) {
          if(msg[i] == separator) {
            msg_clean[i] = ' ';
          } else {
            msg_clean[i] = msg[i];
          }
        }
        msg_clean[i] = '\0';
        sprintf(msg_clean, "%s\n", msg_clean);

		    GtkTextMark *mark;
		    GtkTextIter iter;

		    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        //GtkTextIter *start, *end;
        //gtk_text_buffer_get_bounds(buffer, start, end);
        //gtk_text_buffer_delete(buffer, start, end);
		    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
		    gtk_text_buffer_insert (buffer, &iter, msg_clean, -1);
	    }

        close(newsockfd);
     }
     close(sockfd);
}

void click_boutonProposition(GtkWidget *widget, gpointer window) 
{
    GtkTextMark *mark;
    GtkTextIter iter;

	printf("click_boutonProposition\n");

	int selected_joueurs_nb = 0;
	int* selected_joueurs = (int*)malloc(sizeof(mission_nbj));
	int i;
	for(i = 0; i < 5; i++) {
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkboxPlayer[i]))) {
			selected_joueurs[selected_joueurs_nb] = i;
			selected_joueurs_nb++;
			if(selected_joueurs_nb > mission_nbj)
				break;
		}
	}

	if(selected_joueurs_nb == mission_nbj) {
        char msg[100];
        strcpy(msg, "P");
        for(i = 0; i < selected_joueurs_nb; i++) {
        	sprintf(msg, "%s %d", msg, selected_joueurs[i]);
        }
		sendMessageToMainServer(msg);
  		int i;
  		for (i = 0; i < 5; i++)
  			gtk_widget_set_sensitive (checkboxPlayer[i], FALSE);
		gtk_widget_set_sensitive (boutonProposition, FALSE);
	} else {
	    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
	    gtk_text_buffer_insert (buffer, &iter, "Nombre de joueurs choisis incorrect !\n", -1);
	}

    free(selected_joueurs);
}

void voteOui(GtkWidget *widget, gpointer window) {
	char msg[100];
  sprintf(msg, "V oui %s", username);
	sendMessageToMainServer(msg);
	gtk_widget_set_sensitive (radiovotePlayer[0], FALSE);
	gtk_widget_set_sensitive (radiovotePlayer[1], FALSE);
}

void voteNon(GtkWidget *widget, gpointer window) {
	char msg[100];
  sprintf(msg, "V non %s", username);
	sendMessageToMainServer(msg);
	gtk_widget_set_sensitive (radiovotePlayer[0], FALSE);
	gtk_widget_set_sensitive (radiovotePlayer[1], FALSE);
}

void sendMessageToMainServer(char *mess)
{
    int sockfd, mainServerPortno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(mainServerAddr);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", mainServerAddr);
        exit(1);
    }

    mainServerPortno=atoi(mainServerPort);

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(mainServerPortno);

    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
      error("ERROR connecting");

    /* send the message line to the server */
    n = write(sockfd, mess, strlen(mess));
    if (n < 0)
      error("ERROR writing to socket");

    close(sockfd);

}

int main(int argc, char** argv) {
  GtkWidget *window;
  GtkWidget *table;
  GtkWidget *fixed;

  GtkWidget *entry1;
  GtkWidget *entry2;
  GtkWidget *entry3;
  int i;
  char addr_server_text[50];
  char port_server_text[50];
  char username_text[50];
  char com_connexion[200];

  if (argc!=6)
  {
  	printf("Usage : .joueur @ip_server numport_server @ip_joueur numport_joueur prenom\n");
  	exit(1);
  }

  printf("mainserver   addr: %s\n", argv[1]);
  printf("mainserver   port: %s\n", argv[2]);
  printf("local server addr: %s\n", argv[3]);
  printf("local server port: %s\n", argv[4]);
  printf("         username: %s\n", argv[5]);

  strcpy(mainServerAddr,argv[1]);
  strcpy(mainServerPort,argv[2]);
  strcpy(localServerAddr,argv[3]);
  strcpy(localServerPort,argv[4]);
  strcpy(username,argv[5]);

  localServerThreadPortno=atoi(argv[4]);

  server_thread_ret = pthread_create( &server_thread, NULL, server_func, NULL);
  if (server_thread_ret)
  {
    fprintf(stderr,"Error - pthread_create() return code: %d\n", server_thread_ret);
    exit(EXIT_FAILURE);
  }

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_title(GTK_WINDOW(window), "The Resistance");
  gtk_container_set_border_width(GTK_CONTAINER(window), 15);
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

  fixed = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(window), fixed);

  /* background image */
  GtkWidget* bgImage;
  bgImage = gtk_image_new_from_file("img.png");
  gtk_fixed_put(GTK_FIXED(fixed), bgImage, 600, 400);
  gtk_widget_set_size_request(bgImage, 200, 200);

  sprintf(addr_server_text,"Adresse serveur: %s",argv[1]);
  labelAddrServer = gtk_label_new(addr_server_text);
  gtk_fixed_put(GTK_FIXED(fixed), labelAddrServer, 0, 0);
  gtk_widget_set_size_request(labelAddrServer,200,20);

  sprintf(port_server_text,"Port serveur: %s",argv[2]);
  labelPortServer = gtk_label_new(port_server_text);
  gtk_fixed_put(GTK_FIXED(fixed), labelPortServer, 200, 0);
  gtk_widget_set_size_request(labelPortServer,200,20);

  sprintf(username_text,"Nom du joueur : %s",argv[5]);
  labelUsername = gtk_label_new(username_text);
  gtk_fixed_put(GTK_FIXED(fixed), labelUsername, 400, 0);
  gtk_widget_set_size_request(labelUsername,200,20);

  for (i = 0; i < 5; i++)
  {
  	labelPlayer[i] = gtk_label_new("Inconnu");
  	gtk_fixed_put(GTK_FIXED(fixed), labelPlayer[i], 0, 100+i*20);
  	gtk_widget_set_size_request(labelPlayer[i],100,20);

    checkboxPlayer[i] = gtk_check_button_new();
	GTK_WIDGET_UNSET_FLAGS(checkboxPlayer[i], GTK_CAN_FOCUS);
  	gtk_fixed_put(GTK_FIXED(fixed), checkboxPlayer[i], 100, 100+i*20);
  	gtk_widget_set_size_request(checkboxPlayer[i],30,20);

  	rolePlayer[i] = gtk_label_new("?");
  	gtk_fixed_put(GTK_FIXED(fixed), rolePlayer[i], 150, 100+i*20);
  	gtk_widget_set_size_request(rolePlayer[i],60,20);

    votePlayer[i] = gtk_label_new("--------");
  	gtk_fixed_put(GTK_FIXED(fixed), votePlayer[i], 210, 100+i*20);
  	gtk_widget_set_size_request(votePlayer[i],60,20);
  }

  boutonProposition = gtk_button_new_with_label("Proposition");
  gtk_fixed_put(GTK_FIXED(fixed), boutonProposition, 400, 100);
  gtk_widget_set_size_request(votePlayer[i],100,20);
  g_signal_connect(G_OBJECT(boutonProposition), "clicked", G_CALLBACK(click_boutonProposition), G_OBJECT(window));

  labelMissionNumber = gtk_label_new(mission_number_text);
  gtk_fixed_put(GTK_FIXED(fixed), labelMissionNumber, 0, 220);
  gtk_widget_set_size_request(labelMissionNumber,200,20);

  labelMissionMeneur = gtk_label_new(mission_meneur_text);
  gtk_fixed_put(GTK_FIXED(fixed), labelMissionMeneur, 0, 245);
  gtk_widget_set_size_request(labelMissionMeneur,200,20);

  labelMissionNbj = gtk_label_new(mission_nbj_text);
  gtk_fixed_put(GTK_FIXED(fixed), labelMissionNbj, 0, 270);
  gtk_widget_set_size_request(labelMissionNbj,200,20);

  GtkWidget* rebelImage;
  rebelImage = gtk_image_new_from_file("rebel.png");
  gtk_fixed_put(GTK_FIXED(fixed), rebelImage, 80, 320);
  gtk_widget_set_size_request(rebelImage, 80, 80);

  strcpy(mission_win_text, "0");
  labelMissionWin = gtk_label_new(mission_win_text);
  gtk_fixed_put(GTK_FIXED(fixed), labelMissionWin, 0, 400);
  gtk_widget_set_size_request(labelMissionWin,200,20);

  GtkWidget* spyImage;
  spyImage = gtk_image_new_from_file("spy.png");
  gtk_fixed_put(GTK_FIXED(fixed), spyImage, 280, 320);
  gtk_widget_set_size_request(spyImage, 80, 80);

  strcpy(mission_lose_text, "0");
  labelMissionLose = gtk_label_new(mission_lose_text);
  gtk_fixed_put(GTK_FIXED(fixed), labelMissionLose, 200, 400);
  gtk_widget_set_size_request(labelMissionLose,200,20);

  text_view = gtk_text_view_new();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD);
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), 
                                  GTK_POLICY_AUTOMATIC, 
                                  GTK_POLICY_AUTOMATIC); 
  gtk_container_add (GTK_CONTAINER (scrolled_window), text_view);
  gtk_fixed_put(GTK_FIXED(fixed), scrolled_window, 10, 480);
  gtk_widget_set_size_request(scrolled_window,400,100);

  radiovotePlayer[0] = gtk_button_new_with_label("Oui");
  gtk_fixed_put(GTK_FIXED(fixed), radiovotePlayer[0], 400, 200);
  gtk_widget_set_size_request(radiovotePlayer[0],60,30);
  g_signal_connect(radiovotePlayer[0], "clicked", G_CALLBACK(voteOui), (gpointer) window);
  gtk_widget_set_sensitive (radiovotePlayer[0], FALSE);

  radiovotePlayer[1] = gtk_button_new_with_label("Non");
  gtk_fixed_put(GTK_FIXED(fixed), radiovotePlayer[1], 500, 200);
  gtk_widget_set_size_request(radiovotePlayer[1],60,30);
  g_signal_connect(radiovotePlayer[1], "clicked", G_CALLBACK(voteNon), (gpointer) window);
  gtk_widget_set_sensitive (radiovotePlayer[1], FALSE);
  
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  gtk_widget_show_all(window);

  sprintf(com_connexion,"C %s %s %s",localServerAddr,localServerPort,username);
  sendMessageToMainServer(com_connexion);

  gtk_main();

  //g_free(buffer);

  return 0;
}
