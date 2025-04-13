// Project by :
// Muhammad Ahmad Mustafa - i221591;
// Areej Zeb - i221561;
// CY - B

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <chrono>

using namespace std;

#define RESET "\u001B[0m"
#define RED "\u001B[0;101m"
#define GREEN "\u001B[0;102m"
#define YELLOW "\u001B[0;103m"
#define BLUE "\u001B[0;104m"
#define CYAN "\u001B[0;106m"
#define MAGENTA "\u001B[0;105m"
#define WHITE "\u001B[47m"

#define SIZE 11
#define CELL_NUMBER 40
#define MAX_PLAYERS 4
#define MAX_TOKENS 4
#define MAX_DICE_COUNT 6

int num_tokens; // Global variable to store the number of tokens per player

// Safe squares on the board where players cannot be hit
const int safe_squares[] = {1, 9, 14, 22, 27, 35};

class Cell
{
public:
    int x, y, value;
};

class Player
{
public:
    int x, y, team, index, id;
};

char board[SIZE][SIZE];
Cell cells[CELL_NUMBER];
Cell houses[MAX_PLAYERS][MAX_TOKENS];
Player players[MAX_PLAYERS][MAX_TOKENS];
sem_t dice_semaphore;
pthread_mutex_t board_mutex;
pthread_cond_t turn_cond;
bool game_over = false;
int current_turn = 0;
int num_players;
int finished_positions[MAX_PLAYERS] = {0};
int hit_rate[MAX_PLAYERS] = {0};
int consecutive_turns_no_six[MAX_PLAYERS] = {0};

void board_initialization(char b[SIZE][SIZE])
{
    char newBoard[SIZE][SIZE] = {
        {'r', 'r', ' ', 'O', 'O', 'O', 'O', 'g', ' ', 'g', 'g'},
        {'r', 'r', ' ', 'O', ' ', 'g', ' ', 'O', ' ', 'g', 'g'},
        {' ', ' ', ' ', 'O', ' ', 'g', ' ', 'O', ' ', ' ', ' '},
        {'r', 'O', 'O', 'O', ' ', 'g', ' ', 'O', 'O', 'O', 'O'},
        {'O', ' ', ' ', ' ', ' ', 'g', ' ', ' ', ' ', ' ', 'O'},
        {'O', 'r', 'r', 'r', 'r', 'H', 'b', 'b', 'b', 'b', 'O'},
        {'O', ' ', ' ', ' ', ' ', 'y', ' ', ' ', ' ', ' ', 'O'},
        {'O', 'O', 'O', 'O', ' ', 'y', ' ', 'O', 'O', 'O', 'b'},
        {' ', ' ', ' ', 'O', ' ', 'y', ' ', 'O', ' ', ' ', ' '},
        {'y', 'y', ' ', 'O', ' ', 'y', ' ', 'O', ' ', 'b', 'b'},
        {'y', 'y', ' ', 'y', 'O', 'O', 'O', 'O', ' ', 'b', 'b'}};

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            b[i][j] = newBoard[i][j];
        }
    }
}

void game_initialization()
{
    int newCells[CELL_NUMBER][2] = {
        {3, 0}, {3, 1}, {3, 2}, {3, 3}, {2, 3}, {1, 3}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {1, 7}, {2, 7}, {3, 7}, {3, 8}, {3, 9}, {3, 10}, {4, 10}, {5, 10}, {6, 10}, {7, 10}, {7, 9}, {7, 8}, {7, 7}, {8, 7}, {9, 7}, {10, 7}, {10, 6}, {10, 5}, {10, 4}, {10, 3}, {9, 3}, {8, 3}, {7, 3}, {7, 2}, {7, 1}, {7, 0}, {6, 0}, {5, 0}, {4, 0}};

    int newHouses[MAX_PLAYERS][4][2] = {
        {{0, 0}, {0, 1}, {1, 1}, {1, 0}},
        {{0, 9}, {0, 10}, {1, 10}, {1, 9}},
        {{9, 9}, {9, 10}, {10, 10}, {10, 9}},
        {{9, 0}, {9, 1}, {10, 1}, {10, 0}}};

    for (int i = 0; i < CELL_NUMBER; i++)
    {
        cells[i].y = newCells[i][0];
        cells[i].x = newCells[i][1];
        cells[i].value = 0;
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            houses[i][j].y = newHouses[i][j][0];
            houses[i][j].x = newHouses[i][j][1];
            houses[i][j].value = (j + 1) + (i * 10);
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            players[i][j].x = houses[i][j].x;
            players[i][j].y = houses[i][j].y;
            players[i][j].index = -1;
            players[i][j].team = i;
            players[i][j].id = j + 1;
        }
    }
}

void title_display()
{
    cout << " __________________________________________________________" << endl;
    cout << "|\e[1;31m    __            \e[1;32m   __        \e[1;36m______       \e[1;33m              " << RESET << "|" << endl;
    cout << "|\e[1;31m   / /   __  __ \e[1;32m____/ /___    \e[1;36m/ ____/___ _\e[1;33m____ ___  ___   " << RESET << "|" << endl;
    cout << "|\e[1;31m  / /   / / / / \e[1;32m __  / __ \\ \e[1;36m / / __/ __ `/\e[1;33m __ `__ \\/ _ \\  " << RESET << "|" << endl;
    cout << "|\e[1;31m / /___/ /_/ / \e[1;32m /_/ / /_/ / \e[1;36m/ /_/ / /_/ / \e[1;33m/ / / / /  __/  " << RESET << "|" << endl;
    cout << "|\e[1;31m/_____/\\__,_/ \e[1;32m\\__,_/\\____/\e[1;36m  \\____/\\__,_/\e[1;33m_/ /_/ /_/\\___/   " << RESET << "|" << endl;
    cout << "|__________________________________________________________|" << endl;
}

void display(char b[SIZE][SIZE])
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            string s = "";
            s += b[i][j];
            s += " ";
            switch (b[i][j])
            {
            case 'r':
                cout << RED << s << RESET;
                break;
            case 'g':
                cout << GREEN << s << RESET;
                break;
            case 'b':
                cout << CYAN << s << RESET;
                break;
            case 'y':
                cout << YELLOW << s << RESET;
                break;
            case 'H':
                cout << MAGENTA << s << RESET;
                break;
            case ' ':
                cout << WHITE << "  " << RESET;
                break;
            default:
                cout << s;
                break;
            }
        }
        cout << endl;
    }
    cout << endl;
}

void display_current(char b[SIZE][SIZE], Player players[MAX_PLAYERS][MAX_TOKENS])
{
    char current_board[SIZE][SIZE];

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            current_board[i][j] = b[i][j];
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (players[i][j].index != -1)
            {
                current_board[players[i][j].y][players[i][j].x] = '0' + players[i][j].id;
            }
        }
    }

    cout << endl;
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            string s = "";
            s += current_board[i][j];
            s += " ";
            switch (current_board[i][j])
            {
            case 'r':
                s = "O ";
                cout << RED << s << RESET;
                break;
            case 'g':
                s = "O ";
                cout << GREEN << s << RESET;
                break;
            case 'b':
                s = "O ";
                cout << CYAN << s << RESET;
                break;
            case 'y':
                s = "O ";
                cout << YELLOW << s << RESET;
                break;
            case '1':
            case '2':
            case '3':
            case '4':
                s = current_board[i][j];
                s += " ";
                for (int a = 0; a < MAX_PLAYERS; a++)
                {
                    for (int b = 0; b < 4; b++)
                    {
                        if (i == players[a][b].y && j == players[a][b].x)
                        {
                            switch (players[a][b].team)
                            {
                            case 0:
                                s = RED + s + RESET;
                                break;
                            case 1:
                                s = GREEN + s + RESET;
                                break;
                            case 2:
                                s = CYAN + s + RESET;
                                break;
                            case 3:
                                s = YELLOW + s + RESET;
                                break;
                            }
                        }
                    }
                }
                cout << s;
                break;
            case 'H':
                cout << MAGENTA << "H " << RESET;
                break;
            case ' ':
                cout << WHITE << "  " << RESET;
                break;
            default:
                cout << s;
                break;
            }
        }
        cout << endl;
    }
    cout << endl;
}

void verification()
{
    if (!cin)
    {
        cout << "ERROR - Enter a valid number";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string Token_Color(int id)
{
    switch (id)
    {
    case 0:
        return RED;
    case 1:
        return GREEN;
    case 2:
        return CYAN;
    case 3:
        return YELLOW;
    default:
        return "";
    }
}

int get_index(int turn)
{
    return turn * 10;
}

bool way_final(int turn, int index)
{
    switch (turn)
    {
    case 0:
        return index == 38;
    case 1:
        return index == 8;
    case 2:
        return index == 18;
    case 3:
        return index == 28;
    default:
        return false;
    }
}

bool is_safe_square(int index)
{
    for (int i = 0; i < sizeof(safe_squares) / sizeof(safe_squares[0]); i++)
    {
        if (safe_squares[i] == index)
            return true;
    }
    return false;
}

void move_on_final(Player players[MAX_PLAYERS][MAX_TOKENS], int turn, int choice)
{
    switch (turn)
    {
    case 0:
        players[turn][choice - 1].x += 1;
        break;
    case 2:
        players[turn][choice - 1].x -= 1;
        break;
    case 1:
        players[turn][choice - 1].y += 1;
        break;
    case 3:
        players[turn][choice - 1].y -= 1;
        break;
    default:
        break;
    }
}

struct thread_data
{
    int player_id;
    char (*board)[SIZE];
    Player (*players)[MAX_PLAYERS][MAX_TOKENS];
    Cell (*cells)[CELL_NUMBER];
};

void *player_thread(void *arg)
{
    thread_data *data = (thread_data *)arg;
    int player_id = data->player_id;
    char(*board)[SIZE] = data->board;
    Player(*players)[MAX_PLAYERS][MAX_TOKENS] = data->players;
    Cell(*cells)[CELL_NUMBER] = data->cells;
    int result;

    unsigned int seed = chrono::system_clock::now().time_since_epoch().count() + player_id;
    srand(seed);

    while (!game_over)
    {
        sem_wait(&dice_semaphore);
        pthread_mutex_lock(&board_mutex);

        if (game_over)
        {
            pthread_mutex_unlock(&board_mutex);
            sem_post(&dice_semaphore);
            break;
        }

        display_current(board, *players);
        cout << "It's time for player " << player_id + 1 << " to play." << endl;
        result = (rand() % MAX_DICE_COUNT) + 1;
        cout << "Dice roll result: " << result << endl;

        int choice;
        while (true)
        {
            cout << "Which piece would you like to move " << result << "? (1-" << num_tokens << "): ";
            cin >> choice;
            verification();
            if (choice >= 1 && choice <= num_tokens)
                break;
            cout << "Invalid choice. Please select a piece between 1 and " << num_tokens << "." << endl;
        }

        if ((*players)[player_id][choice - 1].index == -1 && result == 6)
        {
            (*players)[player_id][choice - 1].index = get_index(player_id);
            (*players)[player_id][choice - 1].x = (*cells)[get_index(player_id)].x;
            (*players)[player_id][choice - 1].y = (*cells)[get_index(player_id)].y;
        }
        else if ((*players)[player_id][choice - 1].index != -1)
        {
            int step = result;
            while (step > 0)
            {
                if (way_final(player_id, (*players)[player_id][choice - 1].index) && hit_rate[player_id] > 0)
                {
                    (*players)[player_id][choice - 1].index = 100;
                }
                if ((*players)[player_id][choice - 1].index < CELL_NUMBER)
                {
                    (*players)[player_id][choice - 1].index = ((*players)[player_id][choice - 1].index + 1) % CELL_NUMBER;
                    (*players)[player_id][choice - 1].x = (*cells)[(*players)[player_id][choice - 1].index].x;
                    (*players)[player_id][choice - 1].y = (*cells)[(*players)[player_id][choice - 1].index].y;
                }
                else
                {
                    (*players)[player_id][choice - 1].index = (*players)[player_id][choice - 1].index + 1;
                    move_on_final(*players, player_id, choice);
                    if ((*players)[player_id][choice - 1].x == 5 && (*players)[player_id][choice - 1].y == 5)
                    {
                        display_current(board, *players);
                        finished_positions[player_id] = player_id + 1;
                        game_over = true;
                        break;
                    }
                }
                step--;
            }

            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (i != player_id)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        if ((*players)[i][j].x == (*players)[player_id][choice - 1].x && (*players)[i][j].y == (*players)[player_id][choice - 1].y && !is_safe_square((*players)[i][j].index))
                        {
                            (*players)[i][j].index = -1;
                            (*players)[i][j].x = houses[i][j].x;
                            (*players)[i][j].y = houses[i][j].y;
                            hit_rate[player_id]++;
                        }
                    }
                }
            }
        }

        consecutive_turns_no_six[player_id] = (result == 6) ? 0 : consecutive_turns_no_six[player_id] + 1;

        pthread_mutex_unlock(&board_mutex);
        sem_post(&dice_semaphore);
        sleep(1);
    }
    return NULL;
}

void *master_thread_func(void *arg)
{
    while (!game_over)
    {
        pthread_mutex_lock(&board_mutex);

        for (int i = 0; i < num_players; i++)
        {
            if (consecutive_turns_no_six[i] >= 20)
            {
                finished_positions[i] = -1;
                pthread_cancel(pthread_self());
                cout << "Player " << i + 1 << " has been kicked out of the game due to 20 consecutive turns without a six." << endl;
            }

            bool all_tokens_home = true;
            for (int j = 0; j < num_tokens; j++)
            {
                if (players[i][j].index != 100)
                {
                    all_tokens_home = false;
                    break;
                }
            }

            if (all_tokens_home)
            {
                finished_positions[i] = i + 1;
                game_over = true;
                break;
            }
        }

        pthread_mutex_unlock(&board_mutex);
        sleep(1);
    }

    return NULL;
}

void play(char board[SIZE][SIZE], int num_players)
{
    pthread_t threads[MAX_PLAYERS];
    pthread_t master_thread;
    int player_ids[MAX_PLAYERS];
    thread_data thread_data[MAX_PLAYERS];

    sem_init(&dice_semaphore, 0, 1);
    pthread_mutex_init(&board_mutex, NULL);
    pthread_cond_init(&turn_cond, NULL);

    for (int i = 0; i < num_players; i++)
    {
        player_ids[i] = i;
        thread_data[i].player_id = player_ids[i];
        thread_data[i].board = board;
        thread_data[i].players = &players;
        thread_data[i].cells = &cells;
        pthread_create(&threads[i], NULL, player_thread, &thread_data[i]);
    }

    pthread_create(&master_thread, NULL, master_thread_func, NULL);

    while (!game_over)
    {
        sleep(1); // This ensures that the main thread can display updates periodically
    }

    for (int i = 0; i < num_players; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_join(master_thread, NULL);

    sem_destroy(&dice_semaphore);
    pthread_mutex_destroy(&board_mutex);
    pthread_cond_destroy(&turn_cond);

    // Display final results
    cout << "Game over! Final positions:" << endl;
    for (int i = 0; i < num_players; i++)
    {
        if (finished_positions[i] > 0)
        {
            cout << "Player " << i + 1 << " finished at position " << finished_positions[i] << "." << endl;
        }
        else
        {
            cout << "Player " << i + 1 << " did not finish." << endl;
        }
    }

    for (int i = 0; i < num_players; i++)
    {
        cout << "Player " << i + 1 << " hit rate: " << hit_rate[i] << endl;
    }
}

void menu(int &choice, int &players)
{
    title_display();

    while (choice > 2 || choice < 1)
    {
        cout << endl
             << "1 - To Play" << endl;
        cout << "2 - To Quit" << endl
             << endl
             << ">";
        cin >> choice;
        verification();
    }

    if (choice == 1)
    {
        while (players < 2 || players > 4)
        {
            cout << endl
                 << "How many players (between 2 and 4)? " << endl
                 << endl
                 << ">";
            cin >> players;
            verification();
        }

        while (num_tokens < 1 || num_tokens > 4)
        {
            cout << endl
                 << "How many tokens per player (between 1 and 4)? " << endl
                 << endl
                 << ">";
            cin >> num_tokens;
            verification();
        }
    }
}

int main()
{
    srand(static_cast<unsigned int>(std::time(nullptr)));

    board_initialization(board);
    game_initialization();

    while (true)
    {
        int choice = -1;
        int players = -1;
        menu(choice, players);

        if (choice == 2)
        {
            cout << endl
                 << "Stopping the game ..." << endl;
            return 0;
        }

        play(board, players);
    }

    return 0;
}
