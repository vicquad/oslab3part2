// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/ipc.h>
// #include <sys/shm.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <unistd.h>
// #include <time.h>

// #define SHM_KEY 9876 // Shared memory key for the bank account simulation

// // Function prototypes
// void ParentProcess(int *BankAccount, int *Turn); // Parent: Dear Old Dad
// void ChildProcess(int *BankAccount, int *Turn);  // Child: Poor Student

// int main() {
//     int shm_id;
//     int *ShmPTR;
//     pid_t pid;
//     int status;

//     // Create a shared memory segment for two integers (BankAccount and Turn)
//     shm_id = shmget(SHM_KEY, 2 * sizeof(int), IPC_CREAT | 0666);
//     if (shm_id < 0) {
//         perror("shmget error");
//         exit(1);
//     }
//     printf("Shared memory created.\n");

//     // Attach shared memory
//     ShmPTR = (int *) shmat(shm_id, NULL, 0);
//     if (ShmPTR == (int *) -1) {
//         perror("shmat error");
//         exit(1);
//     }
//     printf("Shared memory attached.\n");

//     // Initialize shared variables
//     ShmPTR[0] = 0; // BankAccount starts at $0
//     ShmPTR[1] = 0; // Turn starts with Parent (0 = Dad, 1 = Student)

//     // Fork a child process
//     pid = fork();
//     if (pid < 0) {
//         perror("fork error");
//         exit(1);
//     }

//     if (pid == 0) {
//         // Child process executes student behavior
//         ChildProcess(&ShmPTR[0], &ShmPTR[1]);
//         exit(0);
//     } else {
//         // Parent process executes dad behavior
//         ParentProcess(&ShmPTR[0], &ShmPTR[1]);

//         // Wait for child to finish
//         wait(&status);

//         // Detach and remove shared memory
//         shmdt(ShmPTR);
//         shmctl(shm_id, IPC_RMID, NULL);
//         printf("Shared memory detached and removed.\n");
//     }

//     return 0;
// }

// /**
//  * Parent process simulates "Dear Old Dad" depositing money if account balance is low.
//  */
// void ParentProcess(int *BankAccount, int *Turn) {
//     int account, deposit;
//     srand(time(NULL)); // Seed random number generator for Parent

//     for (int i = 0; i < 25; i++) {
//         sleep(rand() % 6); // Random sleep between 0-5 seconds

//         account = *BankAccount;

//         // Busy wait until it's Parent's turn
//         while (*Turn != 0);

//         // Only deposit if account balance <= $100
//         if (account <= 100) {
//             deposit = rand() % 101; // Random deposit between $0-$100

//             if (deposit % 2 == 0) { // Only deposit if even
//                 account += deposit;
//                 printf("Dear Old Dad: Deposits $%d / Balance = $%d\n", deposit, account);
//             } else {
//                 printf("Dear Old Dad: Doesn't have any money to give\n");
//             }
//         } else {
//             printf("Dear Old Dad: Thinks Student has enough cash ($%d)\n", account);
//         }

//         // Update shared memory and pass turn to child
//         *BankAccount = account;
//         *Turn = 1;
//     }
// }

// /**
//  * Child process simulates "Poor Student" withdrawing money.
//  */
// void ChildProcess(int *BankAccount, int *Turn) {
//     int account, withdrawal;
//     srand(time(NULL) + 1); // Different seed for Child process

//     for (int i = 0; i < 25; i++) {
//         sleep(rand() % 6); // Random sleep between 0-5 seconds

//         account = *BankAccount;

//         // Busy wait until it's Child's turn
//         while (*Turn != 1);

//         // Generate random withdrawal between $0-$50
//         withdrawal = rand() % 51;
//         printf("Poor Student needs $%d\n", withdrawal);

//         // Withdraw if enough balance
//         if (withdrawal <= account) {
//             account -= withdrawal;
//             printf("Poor Student: Withdraws $%d / Balance = $%d\n", withdrawal, account);
//         } else {
//             printf("Poor Student: Not enough cash ($%d)\n", account);
//         }

//         // Update shared memory and pass turn to parent
//         *BankAccount = account;
//         *Turn = 0;
//     }
// }


#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define SHM_KEY 9876 // Shared memory key

// Function prototypes
void ParentProcess(int *BankAccount, int *Turn); // Parent: Dear Old Dad
void ChildProcess(int *BankAccount, int *Turn);  // Child: Poor Student

int main() {
    int shm_id;
    int *ShmPTR;
    pid_t pid;
    int status;

    // Create shared memory segment for two integers (BankAccount and Turn)
    shm_id = shmget(SHM_KEY, 2 * sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget error");
        exit(1);
    }
    printf("Shared memory created.\n");

    // Attach shared memory
    ShmPTR = (int *) shmat(shm_id, NULL, 0);
    if (ShmPTR == (int *) -1) {
        perror("shmat error");
        exit(1);
    }
    printf("Shared memory attached.\n");

    // Initialize shared variables
    ShmPTR[0] = 0; // BankAccount starts at $0
    ShmPTR[1] = 0; // Turn starts with Parent (0 = Dad, 1 = Student)

    // Fork a child process
    pid = fork();
    if (pid < 0) {
        perror("fork error");
        exit(1);
    }

    if (pid == 0) {
        // Child process executes student behavior
        ChildProcess(&ShmPTR[0], &ShmPTR[1]);
        exit(0);
    } else {
        // Parent process executes dad behavior
        ParentProcess(&ShmPTR[0], &ShmPTR[1]);

        // Wait for child to finish
        wait(&status);

        // Detach and remove shared memory
        shmdt(ShmPTR);
        shmctl(shm_id, IPC_RMID, NULL);
        printf("Shared memory detached and removed.\n");
    }

    return 0;
}

/**
 * Parent process simulates "Dear Old Dad" depositing money if account balance is low.
 */
void ParentProcess(int *BankAccount, int *Turn) {
    int account, deposit;
    int i; // Loop variable declared outside for-loop (C90 compatibility)
    srand(time(NULL)); // Seed random number generator for Parent

    for (i = 0; i < 25; i++) {
        sleep(rand() % 6); // Random sleep between 0-5 seconds

        account = *BankAccount;

        // Busy wait until it's Parent's turn
        while (*Turn != 0);

        // Only deposit if account balance <= $100
        if (account <= 100) {
            deposit = rand() % 101; // Random deposit between $0-$100

            if (deposit % 2 == 0) { // Only deposit if even
                account += deposit;
                printf("Dear Old Dad: Deposits $%d / Balance = $%d\n", deposit, account);
            } else {
                printf("Dear Old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear Old Dad: Thinks Student has enough cash ($%d)\n", account);
        }

        // Update shared memory and pass turn to child
        *BankAccount = account;
        *Turn = 1;
    }
}

/**
 * Child process simulates "Poor Student" withdrawing money.
 */
void ChildProcess(int *BankAccount, int *Turn) {
    int account, withdrawal;
    int i; // Loop variable declared outside for-loop (C90 compatibility)
    srand(time(NULL) + 1); // Different seed for Child process

    for (i = 0; i < 25; i++) {
        sleep(rand() % 6); // Random sleep between 0-5 seconds

        account = *BankAccount;

        // Busy wait until it's Child's turn
        while (*Turn != 1);

        // Generate random withdrawal between $0-$50
        withdrawal = rand() % 51;
        printf("Poor Student needs $%d\n", withdrawal);

        // Withdraw if enough balance
        if (withdrawal <= account) {
            account -= withdrawal;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", withdrawal, account);
        } else {
            printf("Poor Student: Not enough cash ($%d)\n", account);
        }

        // Update shared memory and pass turn to parent
        *BankAccount = account;
        *Turn = 0;
    }
}
