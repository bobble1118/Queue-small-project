//Project version 6
//1. complete destroy funtion
//2. correct some error
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define type_num 3
#define idmax 1000

typedef struct item
{
    int item_id;
    int type; // simulate different type of items works in different places, works at same time
    int process_time; // needed time for processing
    int arrtime;      // arrive time
    int completed_time;
} dType;

typedef struct storage
{
    dType *data;          // use to handle struct item
    struct storage *next; // link to next node
} stType;

typedef struct Q
{
    stType *front;
    stType *rear;
    int Count;        // number of items in Q
    int ttlProcessed; // total items processed
    float ttlWaitTime;  // total waiting time
    int maxCount;     // largest number of items waiting
    int Q_ID;
    dType *occupy; // weather corresponding processor is occupy by item
    struct Q *next;
} QType;

//used to handle queues for each type
typedef struct QueueList
{
    QType *Q;
    struct QueueList *next;
} QLType;

void data_generator(char*); //generate new data if needed
void ReadFile(stType *[], char *);
FILE * Find_output_name(char*); //create an output file and return the file pointer
//void Type_process_time(int[]); //ask user to enter process time of each type
void countTypes(stType *[]); //tell user how many items in each type
void get_Q_num_eachtype(int[], stType *[]); //ask user to enter how many queues and processers for each type

void process(int[], stType *[], QLType *[],FILE*); //simulate items proccesing
int max_time(stType *[]); //return max arrival time of items
int all_empty(QLType *[]); //whether each processers and queues are empty
QType *find_least_item_Q(QLType *); //return the pointer point to the least item Q
void prResult(QLType *[],FILE*);//at last printf the information we needed
void prTypeList(stType*[]); //print the check list for first 3 types to let we check whether processing normally

//handling Queue
void CreateQ(QLType *[], int[]);
void enQ(QLType *, stType **, int);
dType *deQ(QLType *, int);
int emptyQ(QType *);
void setData(stType *, dType *);
dType *getData(stType **);

//handling file
void checkFileExistence(char *);
void handleExistingFile(char *);
void handleNewFile(char *);

//free everything
void Destroy_Q(QLType *[],int *); //destroy queues
void Destroy_typelist(stType *[]); //destroy typelists



int main(void)
{
    char filename[30] = "";
    stType *List_pointer[type_num] = {NULL}; //  type_num pointers point to each type's head
    int Qnum_each_type[type_num];            //  record every type's number
    FILE *output;

    printf("please enter file name:");
    scanf("%s", filename);

    checkFileExistence(filename);
    ReadFile(List_pointer, filename); // input data in one list
    output=Find_output_name(filename);// create a file pointer and prepare to build output file

    countTypes(List_pointer);                         // get 3 types ordered data
    get_Q_num_eachtype(Qnum_each_type, List_pointer); // ask user how many queue each type

    QLType *Qlist[type_num] = {NULL};
    process(Qnum_each_type, List_pointer, Qlist,output);

    prResult(Qlist,output);
    fclose(output);
    Destroy_Q(Qlist, Qnum_each_type);
    Destroy_typelist(List_pointer);
    printf("-----Program END-----\n");
}

void process(int Q_num[], stType *Type_List[], QLType *Qlist[],FILE *oup)
{
    CreateQ(Qlist, Q_num);         // create Qlists contain queues
    int now = 0;                   // current time
    int max = max_time(Type_List); // largest arrive time

    while (now <= max || all_empty(Qlist) != 1) //if something still working or waiting and current time< max arrival time, continue
    {
        printf("\n-------------------time %d---------------------\n", now);
        fprintf(oup, "\n-------------------time %d---------------------\n", now);
        for (int i = 0; i < type_num; i++) // for different type
        {
            QLType *currentQ = Qlist[i]; // point to current Q nodes of Qlist
            printf("    type %d:\n", i + 1);
            fprintf(oup,"    type %d:\n", i + 1);
            enQ(currentQ, &Type_List[i], now); // enQ each item to least item Queue
            // after enQ
            // check each queue whether corresponding processor is completed work or ready to work
            for (int j = 0; j < Q_num[i]; j++)
            {
                // deal with processing or completed condition first
                if (currentQ->Q->occupy != NULL)
                {
                    if (now < currentQ->Q->occupy->completed_time)
                    {
                        printf("item %d still processing in processor %d\n", currentQ->Q->occupy->item_id, currentQ->Q->Q_ID);
                        fprintf(oup,"item %d still processing in processor %d\n", currentQ->Q->occupy->item_id, currentQ->Q->Q_ID);
                    }
                    else
                    {
                        printf("item %d processing completed from processor %d\n", currentQ->Q->occupy->item_id, currentQ->Q->Q_ID);
                        fprintf(oup,"item %d processing completed from processor %d\n", currentQ->Q->occupy->item_id, currentQ->Q->Q_ID);
                        currentQ->Q->occupy = NULL; //nothing is using processor now
                    }
                }
                // later deQ item into idle processers
                if (emptyQ(currentQ->Q) != 1 && currentQ->Q->occupy == NULL) //items waiting in queue and nothing using processer
                {
                    currentQ->Q->occupy = deQ(currentQ, now); //processer now occupy by item which is deQ
                    printf("item %d go into  processor %d\n", currentQ->Q->occupy->item_id, currentQ->Q->Q_ID);
                    fprintf(oup,"item %d go into  processor %d\n", currentQ->Q->occupy->item_id, currentQ->Q->Q_ID);
                }
                currentQ = currentQ->next;
            }
        }
        now++;
    }
}

int max_time(stType *typelist[])
{
    int max_time = 0;
    int type_max_time = 0;
    stType *item = NULL;

    //go through every lists to find largest arrival time
    for (int i = 0; i < type_num; i++)
    {
        item = typelist[i];
        // find last node which arr time largest
        while (item->next != NULL)
            item = item->next;

        type_max_time = item->data->arrtime;
        if (type_max_time > max_time)
            max_time = type_max_time;
    }
    return max_time;
}

int all_empty(QLType *Qlist[])
{
    QLType *currentQ = NULL;
    //go through every queue lists
    for (int i = 0; i < type_num; i++)
    {
        currentQ = Qlist[i];
        while (currentQ != NULL)
        {
            if (currentQ->Q->Count > 0) // queue not empty
                return 0;
            if (currentQ->Q->occupy != NULL) // processor not empty
                return 0;
            currentQ = currentQ->next;
        }
    }
    return 1; // all empty
}

void data_generator(char*name)
{
    FILE*fp;
    fp=fopen(name,"w");
    srand((unsigned)time(NULL));
    int id[idmax];
    for(int i=0; i<idmax; i++) //at first create array with the id we want
    {
        id[i]=i+1;
    }
    for(int i=0; i<idmax; i++) //make the id array random
    {
        int temp,index;
        index=rand()%idmax;
        temp=id[i];
        id[i]=id[index];
        id[index]=temp;
    }
    for(int i=0; i<idmax; i++)
    {
        fprintf(fp,"%4d %4d %4d %4d\n",id[i],rand()%type_num+1,rand()%idmax,rand()%5+1);
    }
    fclose(fp);
}

void ReadFile(stType *L[], char *name)
{
    printf("///ReadFile///\n");
    int id = 0, type = 0, arrtime = 0, processtime=0;//type_time[type_num];
    FILE *fp = fopen(name, "r");
    stType *current = NULL, *previous = NULL;
    //Type_process_time(type_time);
    while (fscanf(fp, "%d", &id) != EOF)
    {
        fscanf(fp, "%d", &type);
        fscanf(fp, "%d", &arrtime);
        fscanf(fp, "%d",&processtime);

        stType *N = malloc(sizeof(stType));
        N->data = malloc(sizeof(dType)); // data also need allocate memory
        N->next = NULL;
        N->data->arrtime = arrtime;
        N->data->item_id = id;
        N->data->type = type;
        N->data->completed_time = 0;
        N->data->process_time = processtime;//type_time[type - 1];
        // ordered insert list
        // read one node insert one node
        if (L[type - 1] == NULL)
            L[type - 1] = N;
        else
        {
            current = L[type - 1]; //find type list's head
            while (current != NULL && current->data->arrtime <= arrtime)
            {
                current = current->next;
            }
            if (current == L[type - 1])
            { // new is smallest
                N->next = L[type - 1];
                L[type - 1] = N;
            }
            else if (current == NULL)
            { // largest
                if (L[type - 1]->next == NULL)
                { // only one node in list
                    L[type - 1]->next = N;
                }
                else
                { // two or above
                    previous = L[type - 1];
                    while (previous->next != current)
                        previous = previous->next;
                    previous->next = N;
                }
            }
            else
            { // middle
                previous = L[type - 1];
                while (previous->next != current)
                    previous = previous->next;
                previous->next = N;
                N->next = current;
            }
        }
    }
    prTypeList(L);
    fclose(fp);
}

FILE * Find_output_name(char str[])
{
    FILE * oup;
    int size=(int)strlen(str);
    char Filename[50]={'\0'};

    for (int i=0; i<size; i++)
    {
        if (str[i]=='.')
        {
            str[i]='\0';
        }
    }
    // if input file name input.txt
    //output file -> input_output.txt
    strcat(Filename, str);
    strcat(Filename, "_output.txt");

    oup=fopen(Filename, "w");
    return oup;
}

/*void Type_process_time(int type_time[])
{
    printf("please enter process time of each type: \n");
    for (int i = 0; i < type_num; i++)
    {
        printf("type %d : ", i + 1);
        scanf("%d", &type_time[i]);
    }
    printf("--------------------------------------------\n");
}*/

void countTypes(stType *type_list[])
{
    int typeCount[type_num] = {0}; // number of each type
    for (int i = 0; i < type_num; i++)
    {
        stType *temp = type_list[i];
        while (temp != NULL)
        {
            temp = temp->next;
            typeCount[i]++;
        }
        printf("Type %d count: %d\n", i + 1, typeCount[i]);
    }
}

void get_Q_num_eachtype(int Q_num[], stType *lists[])
{
    printf("choose the number of queue for each type(1<=x<=3):\n");
    for (int i = 1; i <= type_num; i++)
    {
        do
        {
            printf("type %d :", i);
            scanf("%d", &Q_num[i - 1]);
            if (Q_num[i - 1] > 3 || Q_num[i - 1] <= 0)
                printf("please enter 1-3\n");
        } while (Q_num[i - 1] > 3 || Q_num[i - 1] <= 0);
    }
}

void CreateQ(QLType *QList[], int Q_num[])
{
    QLType *current = NULL;
    for (int i = 0; i < type_num; i++) //create different number of queues of each type
    {
        QList[i] = malloc(sizeof(QLType));
        QList[i]->next = NULL;
        QList[i]->Q = NULL;
        for (int j = 0; j < Q_num[i]; j++)
        {
            QType *new_Q = malloc(sizeof(QType));
            // initialize
            new_Q->Count = 0;
            new_Q->front = NULL;
            new_Q->rear = NULL;
            new_Q->maxCount = 0;
            new_Q->Q_ID = j + 1;
            new_Q->ttlProcessed = 0;
            new_Q->ttlWaitTime = 0;
            new_Q->occupy = NULL;
            if (QList[i]->Q == NULL)
                QList[i]->Q = new_Q;
            else
            {
                current = QList[i];
                while (current->next != NULL)
                    current = current->next;
                current->next = malloc(sizeof(QLType));
                current->next->next = NULL;
                current->next->Q = new_Q;
            }
        }
    }
}

void enQ(QLType *Qlist, stType **current_item, int now)
{
    QType *least_item_Q = NULL;
    stType *temp = NULL;// node whose process time larger than new node
    stType *temp1 = NULL;// node before temp;

    while ((*current_item) != NULL && (*current_item)->data->arrtime <= now) // enQ every item whose arrtime = now
    {
        least_item_Q = find_least_item_Q(Qlist); //enQ to the least item queue
        temp = least_item_Q->front;
        stType *N = malloc(sizeof(stType));
        setData(N, (*current_item)->data);

        if (emptyQ(least_item_Q))
        {
            least_item_Q->front = least_item_Q->rear = N;
            printf("enQ item %d into queue %d\n", N->data->item_id, least_item_Q->Q_ID);
        }
        else
        {
            // Priority Queue
            while(temp != NULL && temp->data->process_time < N->data->process_time) temp = temp->next;
            if(temp == NULL) // N process longest
            {
                least_item_Q->rear->next = N;
                least_item_Q->rear = least_item_Q->rear->next;
            }
            else if(temp == least_item_Q->front) // N process shortest
            {
                N->next = least_item_Q->front;   // let N process first
                least_item_Q->front = N;
            }
            else //middle
            {
                temp1 = least_item_Q->front;
                while(temp1->next!=temp) temp1 = temp1->next;
                N->next = temp;
                temp1->next = N;
            }
            printf("enQ item %d into queue %d\n", N->data->item_id, least_item_Q->Q_ID);
        }

        // renew the count of Q
        least_item_Q->Count++;
        least_item_Q->ttlProcessed++;
        if (least_item_Q->Count > least_item_Q->maxCount)
            least_item_Q->maxCount = least_item_Q->Count;
        (*current_item) = (*current_item)->next;
    }
}

dType *deQ(QLType *Qlist, int now)
{
    // return item pointer to occupy
    dType *item = NULL;
    
    if (emptyQ(Qlist->Q))
        return NULL;
    else if (Qlist->Q->Count == 1) //only one item in queue
    {
        item = getData(&(Qlist->Q->front));
        Qlist->Q->rear = NULL;
    }
    else
        item = getData(&(Qlist->Q->front));

    Qlist->Q->Count--;

    int waiting_time = now - item->arrtime;
    Qlist->Q->ttlWaitTime += waiting_time;

    item->completed_time = now + item->process_time;
    return item;
}

void setData(stType *s, dType *data)
{
    s->data = data; //point to the item
    s->next = NULL;
}

int emptyQ(QType *Q)
{
    if (Q->Count == 0)
        return 1;
    else
        return 0;
}

QType *find_least_item_Q(QLType *Qlist)
{
    QType *least_item_Q = NULL;
    //go thorugh each queues in the Qlist
    while (Qlist != NULL)
    {
        if (least_item_Q == NULL)
            //make Qlist's head the least_item_Q
            least_item_Q = Qlist->Q;
        else
        {
            if (least_item_Q->Count > Qlist->Q->Count)
                least_item_Q = Qlist->Q;
        }
        Qlist = Qlist->next;
    }
    return least_item_Q;
}

void prTypeList(stType* L[])
{
    FILE * op=NULL;
    op=fopen("check_list.txt", "w");
    stType * current = NULL;
    // first three type is enough for checking the work of processor
    for(int i=0;i<3;i++)
    {
        current = L[i];
        switch (i)
        {
            case 0:
                fprintf(op, "----Type 1-----\n");
                break;
            case 1:
                fprintf(op, "----Type 2----:\n");
                break;
            case 2:
                fprintf(op, "----Type 3:----\n");
                break;
            default:
                break;
        }
        while (current!=NULL)
        {
            fprintf(op," id : %d , arrvial time : %d, process time: %d\n",current->data->item_id,current->data->arrtime,current->data->process_time);
            current = current->next;
        }
        fprintf(op, "\n");
    }
    fclose(op);
}

dType *getData(stType **s)
{
    dType *data;
    stType *p = *s;

    data = p->data;
    *s = (*s)->next;
    free(p);
    return data;
}

void prResult(QLType *Qlist[],FILE *oup)
{
    QLType *currentQ = NULL;
    printf("\n----------------------Result----------------------\n");
    fprintf(oup, "\n----------------------Result----------------------\n");
    for (int i = 0; i < type_num; i++)
    {
        printf("Type %d:\n",i+1);
        fprintf(oup,"Type %d:\n",i+1);
        currentQ = Qlist[i];
        while(currentQ != NULL)
        {
            printf("    Q%d\n",currentQ->Q->Q_ID);
            fprintf(oup,"    Q%d\n",currentQ->Q->Q_ID);
            printf("number of processed: %d\n",currentQ->Q->ttlProcessed);
            fprintf(oup,"number of processed: %d\n",currentQ->Q->ttlProcessed);
            printf("maxmum number waiting at one time: %d\n",currentQ->Q->maxCount);
            fprintf(oup,"maxmum number waiting at one time: %d\n",currentQ->Q->maxCount);
            printf("average waiting time: %f\n",currentQ->Q->ttlWaitTime/currentQ->Q->ttlProcessed);
            fprintf(oup,"average waiting time: %f\n",currentQ->Q->ttlWaitTime/currentQ->Q->ttlProcessed);
            currentQ = currentQ->next;
        }
        printf("----------------------------\n");
        fprintf(oup,"----------------------------\n");
    }
}

void Destroy_Q(QLType *Qlist[],int Q_each_num[])
{
    QLType * current = NULL;
    QType * free_Q   = NULL;
    
    for (int i = 0; i < type_num; i++)
    {
        current = Qlist[i];
        for (int j = 0; j < Q_each_num[i]; j++)
        {
            free_Q = current->Q;
            if (current != NULL)current = current->next;
            free(free_Q);
        }
    }
    printf("Completely destroy every queue...\n");
}

void Destroy_typelist(stType *typelist[])
{
    stType * current_node = NULL;
    stType * free_list = NULL;
    dType  * free_data = NULL;
    
    for (int i = 0; i < type_num; i++)
    {
        current_node = typelist[i];
        while (current_node != NULL)
        {
            free_list = current_node;
            free_data = free_list->data;
            if(current_node != NULL)current_node = current_node->next;
            free(free_data);
            free(free_list);
        }
    }
    printf("Completely destroy every typelists...\n");
}

void checkFileExistence(char *filename)
{
    FILE *file = fopen(filename, "r");

    if (file == NULL)
    {
        printf("The file \"%s\" does not exist.\n", filename);
        printf("Do you want to create a new data set? (Yes: 1, No: 0): ");
        int generateNewData;
        scanf("%d", &generateNewData);

        if (generateNewData == 1)
        {
            handleNewFile(filename);
        }
        else
        {
            exit(0);
        }
    }
    else
    {
        fclose(file);
        printf("The file \"%s\" exists.\n", filename);
        printf("Do you want to overwrite the existing file or read from it?\n");
        printf("Enter 0 to overwrite the file (and generate new data), or enter 1 to read from the existing file: ");
        int overwriteOption;
        scanf("%d", &overwriteOption);

        if (overwriteOption == 0)
        {
            handleNewFile(filename);
        }
        else if (overwriteOption == 1)
        {
            handleExistingFile(filename);
        }
        else
        {
            printf("Invalid option. Exiting.\n");
            exit(0);
        }
    }
}

void handleExistingFile(char *filename)
{
    printf("Reading data from the existing file \"%s\".\n", filename);
}

void handleNewFile(char *filename)
{
    printf("Generating new data set and overwriting the existing file \"%s\".\n", filename);
    data_generator(filename);
}
