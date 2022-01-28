/* Task: Sockets             
 * Course: DD1338           
 * Mutlithreaded client in C 
 *
 * Physical dyselxia written by Carl Chemnitz
 */

/* ########################
 * ## Included libraries ##
 * ######################## */

/* Standard C libraries*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
/* Threads libraries */
#include <pthread.h>
/* Network & Communication libraries */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* #########################
 * ## Preprocessor macros ##
 * ######################### */

/* Definitions */
#define USER_ID             20
#define MESSAGE             1003
#define BUFFER              1024
#define EMPTY               "\33[2K\r"

/* Functions */
#define clear()             printf("\033[H\033[J")
#define gotoxy()            printf("\033[%d;%dH", (y) (x))

/* ######################
 * ## Global variables ##
 * ###################### */

char                        userid[20];

/* ################
 * ## Structures ##
 * ################ */

/* Thread data */
typedef struct {
  char*                     data;
  int                       socket;
} thread_data;

/* ###############
 * ## Functions ##
 * ############### */

char* ltrim(const char* s) {
  size_t len;
  char* cur;
  if(s && *s) {
    len = strlen(s);
    cur = s;
    while(*cur && isspace(*cur))
      ++cur, --len;
    if(s != cur)
      memmove(s, cur, len + 1);
  } return s;
}


/* Delete word in string if founder */
int delete(char* str, char* word, int pos) {
  for(int i = 0; word[i] != '\0'; i++) 
    if(word[i + pos] != str[i]) return 0;
  int j = 0, shift = strlen(word);
  do {
    str[j] = str[j + shift];
  } while(str[j + shift] != '\0');
  return 1;
}

void chopN(char *str, size_t n)
{
    assert(n != 0 && str != 0);
    size_t len = strlen(str);
    if (n > len)
        return;  // Or: n = len;
    memmove(str, str+n, len - n + 1);
}

/* Receiver function for thread to run */
void* receiver(void* data) {
  int                       sockfd, recv;
  char                      recv_buff[BUFFER], * prompt;
  thread_data*              td_data;

  td_data = (thread_data*)data;
  sockfd = td_data->socket;
  prompt = td_data->data;

  while(1) {
    memset(recv_buff, 0, BUFFER);
    recv = recvfrom(sockfd, recv_buff, BUFFER, 0, NULL, NULL);
    if(recv == -1) {
      fprintf(stderr, "[-] Failed to read stream.\n"); break; }
    else if(recv == 0) {
      printf("[!] User got disconnected from server.\n "); break; }
    else {
      printf("%s\n >> ", recv_buff);
      fflush(stdout);
    }
  }
}

/* Sender function to send information to connected TCP socket */
void* sender(int sockfd, struct sockaddr_in* serv_addr) {
  char                      send_buff[BUFFER];
  char                      target[USER_ID];
  char                      msg[MESSAGE];
  char*                     tok;

  char t[20] =              "SERVER";
  while(fgets(msg, MESSAGE, stdin) != NULL) {
    if(msg[0] == '\0' || msg[0] == '\n') continue;
    if(strncmp(msg, "STOP", 4) == 0) {
      printf("Disconnected from server...\n"); exit(0); }
    strcpy(send_buff, t);
    strcpy(target, t);
    if(strstr(msg, " ")) {
      tok = strtok(msg, " ");
      if(tok != NULL && strcmp(tok, "/msg") == 0) {
        tok = strtok(msg, " ");
        if(tok != NULL) {
          strcpy(target, tok);
          tok = strtok(msg, " ");
          if(tok != NULL) {
          tok = strtok(msg, " ");
            strcpy(msg, tok);
            printf("[!] Message was sent.\n");
          } else {
            printf("[!] Failure to send mission.\n");
            continue; } 
        } else {
          printf("[!] No message was given");
          continue; }
      } else
        strcpy(target, t);
    }
    strcat(target, " ");
    strcpy(send_buff, target);
    strcat(send_buff, msg);
    send(sockfd, send_buff, strlen(send_buff) + 1, 0);
    printf(" >> ");
  }
}

/* #############
 * ## Program ##
 * ############# */

/* Main, if you know you know */
int main(int argc, char** argv) {
  int                       sockfd = 0;
  struct sockaddr_in        serv_addr;
  char                      recv_buff[1024];
  pthread_t                 thread;

  if(argc != 3) { 
    printf("[-] : Usage <Server-IP> <Port>");
    return 1;
  }

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("[-] : Unable to create socket...\n");
    return 1;
  } 

  memset(&serv_addr, '0', sizeof(serv_addr));
  memset(recv_buff, '0', sizeof(recv_buff));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8080);
  
  if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
    printf("[-] : inet-pton error occured...\n");
    return 1;
  }
  
  if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("[-] : Connection failed...\n");
    return 1;
  }

  while(1) {
    printf("[?] : Username (Limit to 20 letter) << ");
    fgets(userid, BUFFER, stdin);  
    clear();
    if(strlen(userid) > 20) continue;
    break;
  }
  send(sockfd, userid, strlen(userid) + 1, 0);
  printf(" >> ");

  thread_data data;
  data.data = userid;
  data.socket = sockfd;

  pthread_create(&thread, NULL, receiver, (void*)&data);
  sender(sockfd, &serv_addr);

  close(sockfd);
  pthread_exit(NULL);
  return 0;
}
