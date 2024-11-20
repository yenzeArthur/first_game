#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

typedef enum {
    INIT,
    GAME_ON,
    GAME_OVER,
    END
} GAME_STATES;

GAME_STATES game_state = INIT;
int number, guess, attempts, score;
char entry;
FILE *fptr;

time_t start_time, current_time;
int time_limit = 30;
CRITICAL_SECTION game_state_lock;

void game_play();
void difficulty();
void show_high_score();
void play_correct_sound();
void play_incorrect_sound();
void append_score_to_file();
void display_highest_score();
DWORD WINAPI timer_thread(LPVOID lpParam);

int main() {
    srand(time(0)); // Seed random number generator
    score = 0;
    InitializeCriticalSection(&game_state_lock);

    HANDLE hTimerThread = CreateThread(NULL, 0, timer_thread, NULL, 0, NULL);
    printf("Welcome to Guess the Number!\n");

     if (hTimerThread == NULL) {
        printf("Failed to create timer thread. Exiting...\n");
        return 1;
    }

    do {
        game_play();
    } while (1);

    DeleteCriticalSection(&game_state_lock);
    return 0;
}

void game_play() {
    static int max_attempts = 20;

    switch (game_state) {
        case INIT:
            difficulty();
            attempts = 0;
            time(&start_time); // Record the start time
            printf("You have %d seconds and %d attempts. Good luck!\n", time_limit, max_attempts);
            game_state = GAME_ON;
            break;

        case GAME_ON:
            time(&current_time); // Get the current time
            if (difftime(current_time, start_time) > time_limit) {
                printf("Time's up! You couldn't guess the number in %d seconds.\n", time_limit);
                game_state = GAME_OVER;
            break;
            }
            
            if (attempts >= max_attempts) {
                printf("You've run out of attempts!\n");
                game_state = GAME_OVER;
                break;
            }

            printf("Enter your guess: ");
            if (scanf("%d", &guess) != 1) {
                printf("Invalid input! Please enter a number.\n");
                while (getchar() != '\n'); // Clear input buffer
                break;
            }

            attempts++;
            if (guess > number) {
                play_incorrect_sound();
                printf("Too high!\n");
            }

            else if (guess < number) {
                play_incorrect_sound();
                printf("Too low!\n");
            } 
            
            else {
                printf("Congratulations! You guessed it in %d attempts.\n", attempts);
                score += 3; // Increment score
                play_correct_sound();
                game_state = END;
            }
            break;

        case GAME_OVER:
            play_incorrect_sound();
            printf("Game over! The number was %d.\n", number);
            append_score_to_file();
            show_high_score();
            game_state = END;
            break;

        case END:
            printf("Your score this session is: %d\n", score);
            printf("Press 'Y' to play again or 'Q' to quit: ");
            entry = getch();

            if (entry == 'Q' || entry == 'q') {
                printf("\nThanks for playing! Exiting...\n");
                exit(0);
            } 
            
            else if (entry == 'Y' || entry == 'y') {
                max_attempts = 20 - (score / 10); // Adjust difficulty
                game_state = INIT;
            } 
            
            else {
                printf("\nInvalid choice! Exiting...\n");
                exit(0);
            }
            break;

        default:
            printf("Unknown state! Exiting...\n");
            exit(1);
    }
}

void difficulty() {
    if (score <= 30) {
        number = rand() % 100 + 1;
        printf("Welcome to Beginner Mode! Guess a number between 1 and 100.\n");
    } 
    
    else if (score <= 60) {
        number = rand() % 200 + 1;
        printf("Welcome to Amateur Mode! Guess a number between 1 and 200.\n");
    } 
    
    else if (score <= 90) {
        number = rand() % 300 + 1;
        printf("Welcome to Professional Mode! Guess a number between 1 and 300.\n");
    } 
    
    else {
        number = rand() % 500 + 1;
        printf("Welcome to World-Class Mode! Guess a number between 1 and 500.\n");
    }
}

void append_score_to_file() {
    fptr = fopen("score.txt", "a");
    if (fptr == NULL) {
        printf("Error saving your score. Please try again.\n");
        return;
    }
    fprintf(fptr, "%d\n", score);
    fclose(fptr);
}

void display_highest_score() {
    fptr = fopen("score.txt", "r");
    if (fptr == NULL) {
        printf("No high scores yet! Be the first to set one.\n");
        return;
    }

    int scores, max_score = 0;
    while (fscanf(fptr, "%d", &scores) == 1) {
        if (scores > max_score) {
            max_score = scores;
        }
    }

    printf("Highest Score is: %d\n", max_score);
    fclose(fptr);
}

void show_high_score() {
    printf("\n--- High Scores ---\n");
    display_highest_score();
    printf("-------------------\n");
}

void play_correct_sound() {
    Beep(1000, 300); // Frequency = 1000 Hz, Duration = 300 ms
}

void play_incorrect_sound() {
    Beep(500, 300); // Frequency = 500 Hz, Duration = 300 ms
}

DWORD WINAPI timer_thread(LPVOID lpParam) {
    time_t start_time, current_time;

    time(&start_time);
    while (1) {
        time(&current_time);

        EnterCriticalSection(&game_state_lock);
        if (game_state == GAME_OVER || game_state == END) {
            LeaveCriticalSection(&game_state_lock);
            break;
        }

        if (difftime(current_time, start_time) > time_limit) {
            printf("\nTime's up!\n");
            game_state = GAME_OVER;
        }
        LeaveCriticalSection(&game_state_lock);

        Sleep(100); // Check every 100ms
    }
    return 0;
}
