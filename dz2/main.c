#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define MAX_COLORS_IN_TUBE 4
#define MAX_NUMBER_OF_TUBES 20
#define MAX_NUMBER_OF_CHILDREN 5000
#define MAX_NUMBER_OF_CHILDREN_HELPER 500000
#define MAX_DEEP 20

enum { IMPOSSIBLE_MOVE = -1, GOOD_MOVE, EMPTY_TUBE, USELESS_MOVE };

int n, k, p, c;

int8_t check_color[MAX_NUMBER_OF_TUBES];
int8_t random_array[MAX_NUMBER_OF_TUBES][MAX_COLORS_IN_TUBE];

struct node* helper_array[MAX_DEEP][MAX_NUMBER_OF_CHILDREN_HELPER];

struct node* generate_tree();
struct node* generate_initial_state();
struct node* newNode();
struct node* addChildNode();
struct node* find_winner();
struct node* give_hint();
struct node* find_move();

int lcg_generator();
int pouring();
int isWinner();
int play_game();

void print_tree();
void print_node();
void addChildren();
void mark_path();


typedef struct node {
    int status;
    int number_of_children;
    int* children[MAX_NUMBER_OF_CHILDREN];
    int8_t tubes[MAX_NUMBER_OF_TUBES][MAX_COLORS_IN_TUBE];
    void* parent_ptr;
    int8_t current_level;
    int id;
    int* winner_child;
}NODE;

struct node* newNode(int number_of_tubes, int8_t tubes[][MAX_COLORS_IN_TUBE], int status)
{
    int i, j;
    static int id_static;
    NODE* node_ptr = NULL;

    node_ptr = (NODE*)malloc(sizeof(NODE));

    for (i = 0; i < number_of_tubes; i++)
    {
        for (j = 0; j < MAX_COLORS_IN_TUBE; j++)
        {
            node_ptr->tubes[i][j] = tubes[i][j];
        }
    }
    node_ptr->current_level = 0;

    node_ptr->status = status;

    node_ptr->number_of_children = 0;

    node_ptr->id = id_static++;
    node_ptr->parent_ptr = NULL;
    node_ptr->winner_child = NULL;
    return node_ptr;
}

struct node* addChildNode(NODE* existing_node_ptr, int number_of_tubes, int8_t* tubes, int status, int8_t level)
{
    NODE* child_node_ptr = NULL;
    int child_index = existing_node_ptr->number_of_children;
    ++existing_node_ptr->number_of_children;

    child_node_ptr = newNode(number_of_tubes, tubes, status);
    child_node_ptr->parent_ptr = existing_node_ptr;
    child_node_ptr->current_level = level;

    existing_node_ptr->children[child_index] = (int*)child_node_ptr;
    child_node_ptr->winner_child = NULL;
    return child_node_ptr;
}

struct node* generate_initial_state(int c, int k, int n)
{
    NODE* root;
    int ctr = 0, i = 0, j = 0, seed;
    int8_t number;
    int a = 1664525, b = 1013904223;
    long long int x;
    seed = time(NULL);
    while (ctr != c * 4)
    {
        x = abs(a * seed + b);
        number = x % c + 1;
        seed = x;
        if (check_color[number - 1] < 4)
        {
            if (ctr == (i + 1) * 4)
            {
                i++;
                j = 0;
                random_array[i][j] = number;
                j++;
            }
            else
            {
                random_array[i][j] = number;
                j++;
            }
            check_color[number - 1]++;
            ctr++;
        }
    }
    for (i = n - k; i < n; i++)
    {
        random_array[i][0] = 0;
        random_array[i][1] = 0;
        random_array[i][2] = 0;
        random_array[i][3] = 0;
    }
    root = newNode(n, random_array, 0);
    helper_array[0][0] = root;
    return root;
}

int pouring(int8_t* tube1, int8_t* tube2)
{
    int i1, j1, i2, j2, l, m, pouring_colors;
    for (i1 = MAX_COLORS_IN_TUBE - 1; i1 >= 0; i1--)
    {
        if (tube1[i1] != 0)
        {
            break;
        }
    }
    if (i1 == -1)
    {
        return EMPTY_TUBE;
    }
    for (j1 = i1 - 1; j1 >= 0; j1--)
    {
        if (tube1[j1] != tube1[i1])
        {
            break;
        }
    }
    if (j1 == -1 && tube2[0] == 0)
    {
        return USELESS_MOVE;
    }
    pouring_colors = i1 - j1;
    int ctr;
    for (i2 = 0; i2 < MAX_COLORS_IN_TUBE; i2++)
    {
        if (tube2[i2] == 0)
        {
            if (i2 > 0 && tube1[i1] != tube2[i2 - 1])
            {
                return IMPOSSIBLE_MOVE;
            }
            if (i2 == 0 || tube1[i1] == tube2[i2 - 1])
            {
                ctr = 0;
                for (m = i2; m < i2 + pouring_colors; m++)
                {
                    if (m == MAX_COLORS_IN_TUBE)
                    {
                        break;
                    }
                    tube2[m] = tube1[i1];
                    ctr++;
                }
                for (l = i1; l > i1 - ctr; l--)
                {
                    tube1[l] = 0;
                }
            }

            break;
        }
    }
    if (i2 == MAX_COLORS_IN_TUBE)
    {
        return IMPOSSIBLE_MOVE;
    }

    return GOOD_MOVE;
}

void addChildren(int n, NODE* parent_node, int level)
{
    int i, j, ctr = 0;
    NODE* current_node;
    int8_t current_matrix[MAX_NUMBER_OF_TUBES][MAX_COLORS_IN_TUBE];
    int result, flag;
    static int level_s;
    static int node_ctr = 0;
    int is_winner = -1;

    current_node = parent_node;
    NODE* temp_node;
    temp_node = newNode(n, current_matrix, 0);
    *temp_node = *current_node;


    if (level != level_s)
    {
        node_ctr = 0;
        level_s = level;
    }

    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            *temp_node = *current_node;
            if (i != j)
            {
                is_winner = -1;
                result = pouring(temp_node->tubes[i], temp_node->tubes[j]);
                if (result == GOOD_MOVE)
                {
                    is_winner = isWinner(temp_node);
                    if (is_winner == 0)
                    {
                        mark_path(temp_node);
                    }
                    helper_array[level + 1][node_ctr] = addChildNode(current_node, n, temp_node->tubes, 0, level + 1);
                    node_ctr++;
                }
            }
        }
    }
}

int isWinner(NODE* node)
{
    int i, j;
    int8_t current_matrix[MAX_NUMBER_OF_TUBES][MAX_COLORS_IN_TUBE];
    NODE* temp_node;
    temp_node = newNode(n, current_matrix, 0);

    for (i = 0; i < n; i++)
    {
        if (node->tubes[i][0] == 0)
        {
            continue;
        }
        for (j = 1; j < MAX_COLORS_IN_TUBE; j++)
        {
            if (node->tubes[i][0] != node->tubes[i][j])
            {
                return -1;
            }
        }
    }
    node->status = 10;
    temp_node = node->parent_ptr;
    temp_node->status = 1;
    return 0;
}

void mark_path(NODE* node)
{
    int i, ctr = 0;
    int8_t current_matrix[MAX_NUMBER_OF_TUBES][MAX_COLORS_IN_TUBE];
    NODE* current_node;
    NODE* temp_node;
    NODE* temp2_node;

    current_node = newNode(n, current_matrix, 0);
    temp_node = newNode(n, current_matrix, 0);
    temp2_node = newNode(n, current_matrix, 0);

    *current_node = *node;
    *temp_node = *node;
    *temp2_node = *node;
    temp2_node->status = 1;
    while (current_node->parent_ptr != NULL)
    {
        temp2_node = current_node->parent_ptr;
        temp2_node->status = 1;
        temp2_node->winner_child = temp_node;
        temp_node = temp2_node;
        current_node = temp2_node;
    }
}

struct node* find_winner(NODE* start_node)
{
    NODE* current_node;
    NODE* temp_node;
    int8_t current_matrix[MAX_NUMBER_OF_TUBES][MAX_COLORS_IN_TUBE];
    temp_node = newNode(n, current_matrix, 0);

    current_node = start_node;
    *temp_node = *current_node;
    while (current_node->winner_child != NULL)
    {
        temp_node = current_node->winner_child;
        current_node = temp_node;
    }
    return current_node;
}

struct node* generate_tree(int n, int p)
{
    int i, ctr;

    NODE* parent_node;

    for (i = 0; i < p; i++)
    {
        ctr = 0;
        while (helper_array[i][ctr] != 0)
        {
            parent_node = helper_array[i][ctr];
            addChildren(n, parent_node, i);
            ctr++;
        }
    }
}

void print_tree(int n, int k, int p)
{
    int i, j;
    int ctr = 0;


    printf("------------------\n");
    printf("ROOT:\n");
    printf("------------------\n");
    print_node(n, helper_array[0][0]);
    for (i = 1; i <= p; i++)
    {
        printf("\n");
        printf("------------------\n");
        printf("LEVEL: %d\n", i);
        printf("------------------\n");
        ctr = 0;
        while (helper_array[i][ctr] != 0)
        {
            printf("------------------\n");
            printf("LEVEL: %d|NODE: %d|\n", helper_array[i][ctr]->current_level, ctr + 1);
            printf("------------------\n");

            print_node(n, helper_array[i][ctr]);
            ctr++;
        }
    }
}


int play_game(NODE* root, int n, int p)
{
    NODE* current_node;
    NODE* parent_node;
    NODE* tracking_node;
    NODE* temp_node;
    NODE* temp1_node;
    NODE* temp2_node;
    NODE* temp3_node;
    static int play_ctr = 0;

    int result = 0;
    int8_t played_matrix[MAX_NUMBER_OF_TUBES][MAX_COLORS_IN_TUBE];
    int move1, move2;

    temp1_node = newNode(n, played_matrix, 0);
    temp2_node = newNode(n, played_matrix, 0);
    temp3_node = newNode(n, played_matrix, 0);

    int option;
    current_node = root;
    parent_node = root;
    tracking_node = root;
    while (1) {
        print_node(n, current_node);
        printf("------MENI-----\n"
            "Izaberite opciju:\n"
            "1. Odigraj potez\n"
            "2. Hint\n");
        scanf("%d", &option);
        if (option < 0 || option >2) {
            printf("Unesite ispravnu opciju\n");
            continue;
        }
        if (option == 1) {
            printf("Broj poteza preostalo:%d\n", p - play_ctr);
            printf("Izaberi prvu epruvetu: ");
            scanf("%d", &move1);
            printf("Izaberi drugu epruvetu: ");
            scanf("%d", &move2);

            if (move1 == move2)
            {
                printf("Uzmi razlicite epruvete\n");
                continue;
            }
            result = pouring(current_node->tubes[move1 - 1], current_node->tubes[move2 - 1]);
            if (result == IMPOSSIBLE_MOVE)
            {
                printf("Nemoguc potez\n");
                continue;
            }
            else if (result == EMPTY_TUBE)
            {
                printf("Zasto uzimas praznu epruvetu?\n");
                continue;
            }
            else if (result == USELESS_MOVE)
            {
                printf("Beskoristan potez\n");
                continue;
            }

            tracking_node = find_move(parent_node, current_node);
            if (tracking_node != NULL)
            {
                parent_node = tracking_node;
            }
            else
            {
                printf("NEMA TAJ POTEZ\n");
                continue;
            }

            if (isWinner(tracking_node) == 0)
            {
                print_node(n, tracking_node);
                printf("POBEDA!!!!!!\n");
                return 0;
            }
            play_ctr++;
            if (play_ctr == p)
            {
                print_node(n, tracking_node);
                printf("NEMA VISE POTEZA\n");
                return 0;
            }
        }
        else if (option == 2) {
            current_node = give_hint(parent_node);

            if (current_node != NULL)
            {
                if (current_node->status == 10)
                {
                    print_node(n, current_node);
                    printf("POBEDA!!!!!!\n");
                    return 0;
                }
                tracking_node = find_move(parent_node, current_node);

                if (tracking_node != NULL)
                {
                    parent_node = tracking_node;
                    play_ctr++;
                }
                else
                {
                    printf("NEMA TAJ POTEZ\n");
                    continue;
                }

            }
            else
            {
                printf("NEMA HINT\n");
                continue;
            }
        }
    }
}

struct node* find_move(NODE* parent, NODE* move)
{
    int i, flag;
    NODE* new_node;

    for (i = 0; i < parent->number_of_children; i++)
    {
        new_node = parent->children[i];
        if (memcmp(move->tubes, new_node->tubes, sizeof(move->tubes)) == 0)
        {
            return new_node;
        }
    }
    return NULL;
}

struct node* give_hint(NODE* parent)
{
    NODE* new_node;
    NODE* new2_node;
    int i;
    new_node = parent->winner_child;
    if (new_node != NULL)
    {
        return new_node;
    }

    return NULL;
}

void print_node(int n, NODE* node)
{
    int i, j;

    for (i = MAX_COLORS_IN_TUBE - 1; i >= 0; i--)
    {
        for (j = 0; j < n; j++)
        {
            if (node->tubes[j][i] == 0)
            {
                printf(" | |");
            }
            else
            {
                printf(" |%d|", node->tubes[j][i]);
            }
        }
        printf("\n");
    }
}

int main()
{
    int option, ctr;;
    NODE* root;
    int8_t played_matrix[MAX_NUMBER_OF_TUBES][MAX_COLORS_IN_TUBE];
    root = newNode(n, played_matrix, 0);

    printf("Unesite n k p: ");
    scanf("%d %d %d", &n, &k, &p);
    while (1) {
        printf("------MENI-----\n"
            "Izaberite opciju:\n"
            "1. Generisi stablo\n"
            "2. Ispis stabla\n"
            "3. Ispis jednog validnog resenja\n"
            "4. Igraj igru\n"
            "5. Zavrsi\n");
        scanf("%d", &option);
        if (option < 0 || option >5) {
            printf("Unesite ispravnu opciju\n");
            continue;
        }
        if (option == 1) {
            root = generate_initial_state(n - k, k, n);
            generate_tree(n, p);
        }
        else if (option == 2) {
            print_tree(n, k, p);
        }
        else if (option == 3) {
            if (root->winner_child != NULL)
            {
                print_node(n, find_winner(root));
            }
            else
            {
                printf("NEMA POBEDNIKA\n");
            }
        }
        else if (option == 4) {
            play_game(root, n, p);
            for (int i = 0; i <= p; i++)
            {

                ctr = 0;
                while (helper_array[i][ctr] != 0)
                {
                    free(helper_array[i][ctr]);
                    ctr++;
                }
            }
            return 0;
        }
        else if (option == 5) {
            for (int i = 0; i <= p; i++)
            {

                ctr = 0;
                while (helper_array[i][ctr] != 0)
                {
                    free(helper_array[i][ctr]);
                    ctr++;
                }
            }
            break;
        }
    }
}
