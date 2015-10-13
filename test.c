#include <stdio.h>
#include <string.h>

/*

typedef struct node {
      struct node *next;
      char topic[0];
      char value[0];
      char question[0];
      char answer[0];
} node;

node *start = NULL;
node *current;

void add(char *line) {

  node *temp = malloc(sizeof(node)+strlen(line)+1);
  strcpy(temp->data, line);
  temp->next = NULL;
  current = start;

  if(start == NULL) {
    start = temp;
  } else {
    while(current->next != NULL) {
      current = current->next;
    }
    current->next = temp;
  }
}

*/

int main(int argc, char ** argv)
{
    char topic[500];
    char value[500];
    char question[500];
    char answer[500];
    char buf[1024];
    int qSerial = 7;
    char message[512];
    char serial[] = "Hello";

    fgets(buf, sizeof(buf), stdin);
    snprintf(message, sizeof(message), "A\n%s\n%[^\n]\n\n", serial, buf);
    printf(message);

    while(!feof(stdin))
    {
        fscanf( stdin, "%[^$]$%[^\t]\t%[^\t]\t%[^\n]\n", topic, value, question, answer );
        //fscanf( stdin, "%[^$]*\t %s %[^\t]\n", topic, value, question); //%[^\t]", topic, value, question );
        printf("Topic: %s\n", topic);
        printf("Value: %s\n", value);
        printf("Question: %s\n", question);
        printf("Answer: %s\n", answer);

        topic[strlen(topic)-1] = 0;
        snprintf(buf, sizeof(buf), "Q\n%d\n%s\n%s\n%s\n\n", qSerial, topic, value, question);
        //snprintf(buf, sizeof(buf), "Q\\%s!\n", question );
        //        printf("Format: %s\n", buf);

        /*
        char * q = malloc(sizeof(char) * strlen(tempQ));
        strcpy(tempQ, q);
        // answer
        char * a = //;
        strcpy();
*/
    }
}

/*
typedef struct QuestionType {
    char * question;
    char * answer;
    int ptval;
}Question;

typedef struct ListItem {
    void * this;
    Item * next;
}Item;

Question * buildQuestion(char * question, char * answer, int ptval);
Item * buildItem(void * this);

Item * itemList;

int main(int argc, char ** argv)
{
    char tempQ[50];
    char tempA[50];
    int tempval;

    while(!feof(stdin))
    {
        // scanf
        char * q = malloc(sizeof(char) * strlen(tempQ));
        strcpy(tempQ, q);
        // answer
        char * a = //;
        strcpy();

        itemList.this = buildQuestion(q, a, tempval);
        itemList.next = buildItem()


    }
}

*/
