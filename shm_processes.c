#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

/* Constants for banking simulation */
#define NUM_ITERATIONS 25
#define LOW_BALANCE_THRESHOLD 100
#define MAX_DEPOSIT 100
#define MAX_WITHDRAWAL 50
#define MAX_SLEEP_TIME 5

/* Shared memory indices */
#define BANK_ACCOUNT 0
#define TURN 1

/* Turn values for strict alternation */
#define PARENT_TURN 0
#define CHILD_TURN 1

void ChildProcess(int *BankAccount, int *Turn);

int main(int argc, char *argv[])
{
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;
    int account, deposit;
    int i;

    /* Create shared memory for 2 integers: BankAccount and Turn */
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        perror("shmget error");
        exit(1);
    }
    printf("Process has received a shared memory of two integers...\n");

    /* Attach shared memory */
    ShmPTR = (int *) shmat(ShmID, NULL, 0);
    if (ShmPTR == (int *) -1) {
        perror("shmat error");
        exit(1);
    }
    printf("Process has attached the shared memory...\n");

    /* Initialize shared variables */
    ShmPTR[BANK_ACCOUNT] = 0;      /* BankAccount starts at $0 */
    ShmPTR[TURN] = PARENT_TURN;    /* Parent goes first */
    printf("Orig Bank Account = %d\n", ShmPTR[BANK_ACCOUNT]);

    /* Fork child process */
    pid = fork();
    if (pid < 0) {
        perror("fork error");
        exit(1);
    }
    else if (pid == 0) {
        /* Child Process - Poor Student */
        ChildProcess(&ShmPTR[BANK_ACCOUNT], &ShmPTR[TURN]);
        exit(0);
    }

    /* Parent Process - Dear Old Dad */
    srand(time(NULL));

    for (i = 0; i < NUM_ITERATIONS; i++) {
        sleep(rand() % (MAX_SLEEP_TIME + 1));  /* Random sleep 0-5 seconds */

        /* Strict alternation: busy-wait for parent's turn
         * This is Tanenbaum's strict alternation solution.
         * Busy-waiting is acceptable here because:
         * 1) It's a simple two-process scenario
         * 2) Processes alternate quickly with sleep delays
         * 3) Assignment specifically requires this approach */
        while (ShmPTR[TURN] != PARENT_TURN)
            ;

        account = ShmPTR[BANK_ACCOUNT];

        /* Deposit logic: only if balance is low */
        if (account <= LOW_BALANCE_THRESHOLD) {
            deposit = rand() % (MAX_DEPOSIT + 1);  /* Random $0-$100 */

            if (deposit % 2 == 0) {  /* Only deposit even amounts */
                account += deposit;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", deposit, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        /* Update shared memory and pass turn to child */
        ShmPTR[BANK_ACCOUNT] = account;
        ShmPTR[TURN] = CHILD_TURN;
    }

    /* Wait for child to complete */
    if (wait(&status) == -1) {
        perror("wait error");
    }

    /* Report final balance */
    printf("Final Bank Account Balance: $%d\n", ShmPTR[BANK_ACCOUNT]);

    /* Clean up shared memory */
    if (shmdt((void *) ShmPTR) == -1) {
        perror("shmdt error");
    }
    if (shmctl(ShmID, IPC_RMID, NULL) == -1) {
        perror("shmctl error");
    }

    exit(0);
}

void ChildProcess(int *BankAccount, int *Turn)
{
    int account, withdrawal;
    int i;

    /* Different seed to ensure independent random numbers */
    srand(time(NULL) + getpid());

    for (i = 0; i < NUM_ITERATIONS; i++) {
        sleep(rand() % (MAX_SLEEP_TIME + 1));  /* Random sleep 0-5 seconds */

        /* Strict alternation: busy-wait for child's turn
         * Safe here due to guaranteed alternation between processes */
        while (*Turn != CHILD_TURN)
            ;

        account = *BankAccount;

        /* Generate withdrawal need */
        withdrawal = rand() % (MAX_WITHDRAWAL + 1);  /* Random $0-$50 */
        printf("Poor Student needs $%d\n", withdrawal);

        /* Attempt withdrawal if sufficient funds */
        if (withdrawal <= account) {
            account -= withdrawal;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", withdrawal, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        /* Update shared memory and pass turn back to parent */
        *BankAccount = account;
        *Turn = PARENT_TURN;
    }
}