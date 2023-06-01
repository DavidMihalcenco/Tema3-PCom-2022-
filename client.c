#include "helpers.h"
#include "buffer.h"
#include "requests.h"

int main() {

    char *message;
    char *response;
    int sockfd;
    char *cookies = malloc (MAX_CHAR*sizeof(char));
    char *tokens = malloc (MAX_CHAR*sizeof(char));
    int logged = 0;
    int library = 0;

    while(1) {
        
        //Aloc memorie si extrag comanda.
        char *command;
        command = (char *)malloc(MAX_CHAR * sizeof(char));
        fgets(command,MAX_CHAR,stdin);

        char *logins = 0;
        char *toke = 0;

        if(strncmp(command,"register",8)==0){

             if(logged == 1){
                printf("You are logged-in,logout please!\n");
                continue;
            }

            size_t size = 256;
            char *username = (char *) malloc (MAX_CHAR * sizeof(char));
            char *password = (char *) malloc (MAX_CHAR * sizeof(char));
            char *item;
            cJSON *payload;

            //Extrag username
            printf("username=");
            getline(&username,&size,stdin);
            //Scap de newline inlocuiesc cu null-terminator
            int user_l = strlen(username);
            username[user_l-1]='\0';

            //Extrag parola
            printf("password=");
            getline(&password,&size,stdin);
            //Scap de newline inlocuiesc cu null-terminator
            int pass_l = strlen(password);
            password[pass_l-1]='\0';
            
            //Creez obiectul cu username si parola
            payload = cJSON_CreateObject();
            cJSON_AddItemToObject(payload,"username",cJSON_CreateString(username));
            cJSON_AddItemToObject(payload,"password",cJSON_CreateString(password));
            item = cJSON_Print(payload);

            //Creez mesajul
            message = compute_post_request(SV_HOST,"/api/v1/tema/auth/register",
                                        "application/json",&item,1,NULL,0,NULL,0);

            //Deschid conexiunea cu serverul
            sockfd = open_connection(SV_HOST,SV_PORT,PF_INET,SOCK_STREAM,0);

            //Trimit mesajul catre server
            send_to_server(sockfd,message);
            
            //Primesc raspunsul de la server
            response = receive_from_server(sockfd);
            
            //Verific daca a venit un mesaj cu erroare, daca da intorc un mesaj erroare
            char *err = strstr(response, "error");
            if (err != NULL) {
                printf("Error! Can't create account with name : %s\n",username);
            } else {
                printf("User added successfully!\n");
            }

            //Inchid conexiunea cu serverul
            close_connection(sockfd);

        }else if(strncmp(command,"exit",4)==0){
            break;
        }else if(strncmp(command,"login",5)==0){
            
            //Verific daca sunt logat, daca da atunci nu pot sa ma loghez iar
            if(logged == 1){
                printf("You are logged-in,logout please!\n");
                continue;
            }

            size_t size = 1000;
            char *username = (char *) malloc (MAX_CHAR * sizeof(char));
            char *password = (char *) malloc (MAX_CHAR * sizeof(char));
            char *item;
            cJSON *payload;


            //Primesc username
            printf("username=");
            getline(&username,&size,stdin);
            //Scap de newline inlocuiesc cu null-terminator
            int user_l = strlen(username);
            username[user_l-1]='\0';

            //Primesc parola
            printf("password=");
            getline(&password,&size,stdin);
            //Scap de newline inlocuiesc cu null-terminator
            int pass_l = strlen(password);
            password[pass_l-1]='\0';

            //Creez obiectul
            payload = cJSON_CreateObject();
            cJSON_AddItemToObject(payload,"username",cJSON_CreateString(username));
            cJSON_AddItemToObject(payload,"password",cJSON_CreateString(password));
            item = cJSON_Print(payload);

            //Creez mesajul
            message = compute_post_request(SV_HOST,"/api/v1/tema/auth/login",
                                                "application/json",&item,1,NULL,0,NULL,0);

            //Deschid conexiunea cu serverul
            sockfd = open_connection(SV_HOST,SV_PORT,PF_INET,SOCK_STREAM,0);
            
            //Trimit mesajul
            send_to_server(sockfd,message);
            
            //Primesc raspunsul
            response = receive_from_server(sockfd);

            //Verific daca am un mesaj cu erroare
            char *err = strstr(response, "error");
            if (err != NULL) {
                printf("Error! Inccorect username or password!\n");
            } else {
                //Extrag cookie-ul
                char *set_cookie = strstr(response,"Set-Cookie");
                logins = strtok(set_cookie,";");
                logins += 12;
                strcpy(cookies,logins);
                logged = 1;
                printf("Welcome back %s\n",username);
            }

            //Inchid conexiunea
            close_connection(sockfd);
        }else if(strncmp(command,"enter_library",13)==0){
            
            //Verific daca sunt deja in biblioteca
            if(library == 1){
                printf("You are in library aleardy!\n");
                continue;
            }

            message = compute_get_request(SV_HOST,"/api/v1/tema/library/access"
                                                        ,NULL,&cookies,1,NULL,0);
            
            sockfd = open_connection(SV_HOST,SV_PORT,PF_INET,SOCK_STREAM,0);

            send_to_server(sockfd,message);

            response = receive_from_server(sockfd);

            char *err = strstr(response, "error");
            if (err != NULL) {
                printf("Error! You are not logged in!\n");
            } else {
                //Extrag tokenul si il salvez in tokens,
                //setez library pe 1 pentru ca ma aflu in biblioteca
                char *token = strstr(response,"token");
                char *auth = malloc(1000*sizeof(char));
                strcat(auth,"Authorization: Bearer ");
                token += 8;
                toke = strtok(token,"\"");
                strcat(auth,toke);
                strcpy(tokens,auth);
                library = 1;
                printf("Library access permited!\n");
            }

            close_connection(sockfd);
            
        }else if(strncmp(command,"get_books",9)==0){
            //Verific daca am tokens,daca nu inseamna ca nu am acces la biblioteca
            if(strcmp(tokens,"")==0){
                printf("You dont have access to library!\n");
            }else{
                message = compute_get_request(SV_HOST,"/api/v1/tema/library/books"
                ,NULL,&cookies,1,&tokens,1);
            
                sockfd = open_connection(SV_HOST,SV_PORT,PF_INET,SOCK_STREAM,0);

                send_to_server(sockfd,message);

                response = receive_from_server(sockfd);

                char *err = strstr(response, "error");
                
                if (err != NULL) {
                    printf("Error! Access Denied\n");
                } else {
                    //Creez un obiect de tip carte
                    int i;
                    cJSON *carte;
                    cJSON *name;
                    cJSON *id;
                    char *carti = strstr(response,"[{");
                    cJSON *root = cJSON_Parse(carti);
                    int sizec = cJSON_GetArraySize(root);
                    //Verific daca sunt carti
                    if(sizec == 0){
                        printf("There are no books!\n");
                    }else {
                        for(i=0; i<sizec; i++){
                            carte = cJSON_GetArrayItem(root,i);
                            id = cJSON_GetObjectItem(carte, "id");
                            name = cJSON_GetObjectItem(carte,"title");
                            char *rs =  cJSON_Print(id);
                            char *rs1 =  cJSON_Print(name);
                            printf("Name of book with id %s is : %s\n",rs, rs1);
                        }
                    }
                }
                close_connection(sockfd);
            }
        }else if(strncmp(command,"get_book",8)==0){
            if(strcmp(tokens,"")==0){

                printf("You dont have access to library!\n");

            }else{
                
                //Extrag id
                int idnew;
                printf("id= ");
                scanf("%d",&idnew);

                //Creez adresa cu id la final
                char string_addr[40] = "/api/v1/tema/library/books/";
                char string_ip[32];

                sprintf(string_ip,"%d",idnew);
                strcat(string_addr,string_ip);

                message = compute_get_request(SV_HOST,string_addr,NULL,&cookies,1,&tokens,1);

                sockfd = open_connection(SV_HOST,SV_PORT,PF_INET,SOCK_STREAM,0);

                send_to_server(sockfd,message);

                response = receive_from_server(sockfd);

                char *err = strstr(response, "error");
                if (err != NULL) {
                    printf("Error! Book doesn't exist\n");
                } else {
                    //Extrag o carte si afisez tot continutul ei.
                    int i;
                    cJSON *carte;
                    cJSON *name;
                    cJSON *author;
                    cJSON *publisher;
                    cJSON *genre;
                    cJSON *page_count;
                    char *carti = strstr(response,"[{");
                    cJSON *root = cJSON_Parse(carti);
                    int sizec = cJSON_GetArraySize(root);

                    for(i=0; i<sizec; i++){
                        carte = cJSON_GetArrayItem(root,i);
                        name = cJSON_GetObjectItem(carte,"title");
                        author = cJSON_GetObjectItem(carte,"author");
                        publisher = cJSON_GetObjectItem(carte,"publisher");
                        genre = cJSON_GetObjectItem(carte,"genre");
                        page_count = cJSON_GetObjectItem(carte,"page_count");
                        char *rs =  cJSON_Print(name);
                        char *rs1 =  cJSON_Print(author);
                        char *rs2 =  cJSON_Print(publisher);
                        char *rs3 =  cJSON_Print(genre);
                        char *rs4 =  cJSON_Print(page_count);
                        printf("INFO ABOUT %s with id : %d\n Author : %s\n",rs,idnew,rs1);
                        printf(" Publisher : %s\n Genre : %s\n Page_Count : %s\n",rs2,rs3,rs4);
                        
                    }
                }
                close_connection(sockfd);
            }
        }else if(strncmp(command,"add_book",8)==0){
            if(strcmp(tokens,"")==0){
                printf("You dont have access to library!\n");
            }else{
                //Astept toate informatiile despre carte
                char *item;
                cJSON *payload;
                size_t size = 1000;
                int page_count = 0;
                
                printf("title= ");
                char *title = (char *) malloc (size * sizeof(char));
                getline(&title,&size,stdin);
                int title_l = strlen(title);
                title[title_l-1]='\0';

                printf("author= ");
                char *author = (char *) malloc (size * sizeof(char));
                getline(&author,&size,stdin);
                int author_l = strlen(author);
                author[author_l-1]='\0';

                printf("genre= ");
                char *genre = (char *) malloc (size * sizeof(char));
                getline(&genre,&size,stdin);
                int genre_l = strlen(genre);
                genre[genre_l-1]='\0';
                
                printf("publisher= ");
                char *publisher = (char *) malloc (size * sizeof(char));
                getline(&publisher,&size,stdin);
                int publisher_l = strlen(publisher);
                publisher[publisher_l-1]='\0';
                
                printf("page_count= ");
                scanf("%d",&page_count);

                //Verific daca pagina cartilor a fost setata ca numar
                if(page_count==0){
                    printf("Error! Inccorect data!\n");
                    continue;
                }

                //Creez o carte
                payload = cJSON_CreateObject();
                cJSON_AddItemToObject(payload,"title",cJSON_CreateString(title));
                cJSON_AddItemToObject(payload,"author",cJSON_CreateString(author));
                cJSON_AddItemToObject(payload,"genre",cJSON_CreateString(genre));
                cJSON_AddItemToObject(payload,"page_count",cJSON_CreateNumber(page_count));
                cJSON_AddItemToObject(payload,"publisher",cJSON_CreateString(publisher));
                item = cJSON_Print(payload);

                message = compute_post_request(SV_HOST,"/api/v1/tema/library/books","application/json",&item,1,&cookies,1,&tokens,1);

                sockfd = open_connection(SV_HOST,SV_PORT,PF_INET,SOCK_STREAM,0);

                send_to_server(sockfd,message);

                response = receive_from_server(sockfd);

                char *err = strstr(response, "error");
                if (err != NULL) {
                    printf("Error! Inccorect data!\n");
                } else {
                   printf("Succes! Book %s has been added!\n",title);
                }

                close_connection(sockfd);

            }
        }else if(strncmp(command,"delete_book",11)==0){
            if(strcmp(tokens,"")==0){
                printf("You dont have access to library!\n");
            }else{
                //Astept un id
                int idnew;
                printf("id= ");
                scanf("%d",&idnew);
                //Creez adresa corecta
                char string_addr[40] = "/api/v1/tema/library/books/";
                char string_ip[32];

                sprintf(string_ip,"%d",idnew);
                strcat(string_addr,string_ip);

                message = compute_delete_request(SV_HOST,string_addr,NULL,&cookies,1,&tokens,1);
                
                sockfd = open_connection(SV_HOST,SV_PORT,PF_INET,SOCK_STREAM,0);

                send_to_server(sockfd,message);

                response = receive_from_server(sockfd);

                char *err = strstr(response, "error");
                if (err != NULL) {
                    printf("Error! Inccorect id!\n");
                } else {
                   printf("Succes! Book with id %d has been deleted!\n",idnew);
                }
                close_connection(sockfd);
            }
        }else if(strncmp(command,"logout",6)==0){
            if(strcmp(cookies,"")==0){
                printf("You are not logged in!\n");
            }else{
                message = compute_get_request(SV_HOST,"/api/v1/tema/auth/logout",NULL,&cookies,1,NULL,0);

                sockfd = open_connection(SV_HOST,SV_PORT,PF_INET,SOCK_STREAM,0);

                send_to_server(sockfd,message);

                response = receive_from_server(sockfd);

                char *err = strstr(response, "error");
                if (err != NULL) {
                    printf("Error! Unsucessful logout!\n");
                } else {
                   printf("Logout successful\n");
                   logged = 0;
                   library = 0;
                }
                //setez tokens si cookies cu nimic pentru a putea face verificarile
                //daca sunt logat sau daca am acces la biblioteca.
                strcpy(tokens,"");
                strcpy(cookies,"");
                close_connection(sockfd);
            }
        }
    }
}