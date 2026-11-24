#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#define ADMIN 1
#define CUSTOMER 2

#define STACK_CAPACITY 100
#define QUEUE_CAPACITY 50

#define STATUS_PENDING 0
#define STATUS_APPROVED 1
#define STATUS_REJECTED 2

#define COMPLAINT_PENDING 0
#define COMPLAINT_IN_PROGRESS 1
#define COMPLAINT_RESOLVED 2


// Data Structures
typedef struct Admin
{
    char username[31];
    char password[31];
    struct Admin *nxt;
} Admin;

typedef struct Account
{
    int accountNumber;
    char holderName[51];
    char accountType[10];
    double balance;
    char password[31];
    int isBlocked;
    int transactionCount;
    struct Account *nxt;
} Account;

typedef struct Registration
{
    int requestId;
    char name[51];
    char accountType[10];
    double initialBalance;
    char password[31];
    int status;
    struct Registration *nxt;
} Registration;

typedef struct Loan
{
    int loanId;
    int accountNumber;
    double amount;
    int status;
    struct Loan *nxt;
} Loan;

typedef struct Complaint
{
    int complaintId;
    int accountNumber;
    char text[101];
    int status;
    struct Complaint *nxt;
} Complaint;

typedef struct Activity
{
    char timestamp[26];
    char message[161];
    struct Activity *nxt;
} Activity;

typedef struct Transaction
{
    char type[20];
    int accountNumber;
    int senderAccount;
    int receiverAccount;
    double amount;
    char description[101];
} Transaction;

// Stack (array based) - newest transaction is on top
typedef struct TransactionStack
{
    Transaction items[STACK_CAPACITY];
    int top;
    int totalCount;
    int depositCount;
    int withdrawalCount;
    int transferCount;
    double totalDeposits;
    double totalWithdrawals;
    double totalTransfers;
} TransactionStack;

typedef struct Customer
{
    char name[51];
    int accountNumber;
    int queueNumber;
} Customer;

// Circular array queue - customer waiting line
typedef struct CustomerQueue
{
    Customer items[QUEUE_CAPACITY];
    int front;
    int rear;
    int size;
} CustomerQueue;


// Function Prototypes
void loadAdmins();
void loadAccounts();
void loadRegistrations();
void loadLoans();
void loadComplaints();
void loadTransactions();
void loadActivities();
void loadCounters();
void fixCounters();

void saveAdmins();
void saveAccounts();
void saveRegistrations();
void saveLoans();
void saveComplaints();
void saveTransactions();
void saveCounters();

void clearInput();
int readInt(const char *message);
double readDouble(const char *message);
double readPositiveAmount(const char *message);
void readText(const char *message, char *destination, int destinationSize);
int confirmAction(const char *message);
int containsIgnoreCase(const char *text, const char *pattern);

void initializeStack();
int pushTransaction(Transaction transaction);
void initializeQueue();
int isQueueFull();
int isQueueEmpty();
int enqueueCustomer(Customer customer);
int dequeueCustomer(Customer *removedCustomer);
void displayQueue();

void addActivity(const char *message);
void logAdminAction(const char *action);
void viewActivityLog();

void login(int role);
void submitRegistrationRequest();

void adminPanel();
void customerPanel(Account *account);

void viewAllAccounts();
void createAccount();
void searchAccountByNumber();
void searchAccountByName();
void updateAccount();
void deleteAccount();
void adminDeposit();
void adminWithdraw();
void adminTransfer();
void viewAllTransactions();
void manageRegistrations();
void blockUnblockMenu();
void resetCustomerPassword();
void loanManagement();
void handleComplaints();
void manageQueue();
void dashboardOverview();
void generateReports();
void deleteInactiveUsers();

int performDeposit(Account *account, double amount);
int performWithdraw(Account *account, double amount);
int performTransfer(Account *sender, Account *receiver, double amount);
void showTransactions(const char *filter, int accountNumber);

void applyForLoan(Account *account);
int displayLoans(int onlyPending, int accountNumber);
void processLoan(int approve);
void submitComplaint(int accountNumber);
void displayComplaints(int accountNumber);
void changeComplaintStatus(int newStatus);
int displayRegistrations(int onlyPending);
void processRegistration(int approve);
void changeBlockStatus(int mode);
void changePassword(Account *account);

Account *findAccount(int accountNumber);
Account *addAccountToList(const char *name, const char *type, double balance, const char *password);
void appendAccountNode(Account *node);
int removeAccount(int accountNumber);
int isInactive(const Account *account);
int displayInactiveAccounts();

const char *statusText(int status);
const char *complaintStatusText(int status);
const char *accountStatusText(const Account *account);
void displayAccountDetails(Account *account);

void freeAdmins();
void freeAccounts();
void freeRegistrations();
void freeLoans();
void freeComplaints();
void freeActivities();


Admin *adminHead = NULL;
Account *accountHead = NULL;
Registration *registrationHead = NULL;
Loan *loanHead = NULL;
Complaint *complaintHead = NULL;
Activity *activityHead = NULL;

TransactionStack txStack;
CustomerQueue waitingQueue;

int nextAccountNumber = 1001;
int nextRegistrationId = 1;
int nextLoanId = 1;
int nextComplaintId = 1;
int nextQueueNumber = 1;

char currentAdmin[31] = "";


int main()
{
    int choice;

    initializeStack();
    initializeQueue();

    loadAdmins();
    loadAccounts();
    loadRegistrations();
    loadLoans();
    loadComplaints();
    loadTransactions();
    loadActivities();
    loadCounters();
    fixCounters();

    while (1)
    {
        printf("\n================================\n");
        printf("    BANK MANAGEMENT SYSTEM\n");
        printf("================================\n\n");

        printf("1. Admin Login\n");
        printf("2. Customer Login\n");
        printf("3. Open New Account (Registration Request)\n");
        printf("0. Exit\n\n");

        choice = readInt("Enter your choice: ");

        switch (choice)
        {
        case 1:
            login(ADMIN);
            break;
        case 2:
            login(CUSTOMER);
            break;
        case 3:
            submitRegistrationRequest();
            break;
        case 0:
            freeAdmins();
            freeAccounts();
            freeRegistrations();
            freeLoans();
            freeComplaints();
            freeActivities();
            printf("\nThank you for using the Bank Management System!\n");
            return 0;

        default:
            printf("Invalid choice!\n");
        }
    }

    return 0;
}


// Load Persistent Records from Text Files into Singly Linked Lists
void loadAdmins()
{
    FILE *fp = fopen("admins.txt", "r");

    if (fp == NULL)
    {
        // First run: create the default admin
        Admin *newAdmin = malloc(sizeof(Admin));

        printf("\n[INFO] No existing admin records found. Creating default admin.\n");
        if (newAdmin == NULL)
        {
            printf("\n[ERROR] Memory allocation failed!\n");
            return;
        }
        strcpy(newAdmin->username, "admin");
        strcpy(newAdmin->password, "admin123");
        newAdmin->nxt = NULL;
        adminHead = newAdmin;
        saveAdmins();
        return;
    }

    while (1)
    {
        Admin *newAdmin = malloc(sizeof(Admin));

        if (newAdmin == NULL)
        {
            printf("\n[ERROR] Memory allocation failed!\n");
            fclose(fp);
            return;
        }

        int result = fscanf(fp, "%30[^|]|%30[^\n]\n",
                            newAdmin->username,
                            newAdmin->password);

        if (result != 2)
        {
            free(newAdmin);
            break;
        }

        newAdmin->nxt = NULL;

        if (adminHead == NULL)
        {
            adminHead = newAdmin;
        }
        else
        {
            Admin *temp = adminHead;

            while (temp->nxt != NULL)
            {
                temp = temp->nxt;
            }

            temp->nxt = newAdmin;
        }
    }

    fclose(fp);
    printf("\n[SUCCESS] Admin records loaded successfully!\n");
}

void loadAccounts()
{
    FILE *fp = fopen("accounts.txt", "r");

    if (fp == NULL)
    {
        printf("\n[INFO] No existing account records found.\n");
        return;
    }

    while (1)
    {
        Account *newAccount = malloc(sizeof(Account));

        if (newAccount == NULL)
        {
            printf("\n[ERROR] Memory allocation failed!\n");
            fclose(fp);
            return;
        }

        int result = fscanf(fp, "%d|%50[^|]|%9[^|]|%lf|%30[^|]|%d|%d\n",
                            &newAccount->accountNumber,
                            newAccount->holderName,
                            newAccount->accountType,
                            &newAccount->balance,
                            newAccount->password,
                            &newAccount->isBlocked,
                            &newAccount->transactionCount);

        if (result != 7)
        {
            free(newAccount);
            break;
        }

        appendAccountNode(newAccount);
    }

    fclose(fp);
    printf("\n[SUCCESS] Account records loaded successfully!\n");
}

void loadRegistrations()
{
    FILE *fp = fopen("registrations.txt", "r");

    if (fp == NULL)
    {
        printf("\n[INFO] No existing registration records found.\n");
        return;
    }

    while (1)
    {
        Registration *newReg = malloc(sizeof(Registration));

        if (newReg == NULL)
        {
            printf("\n[ERROR] Memory allocation failed!\n");
            fclose(fp);
            return;
        }

        int result = fscanf(fp, "%d|%50[^|]|%9[^|]|%lf|%30[^|]|%d\n",
                            &newReg->requestId,
                            newReg->name,
                            newReg->accountType,
                            &newReg->initialBalance,
                            newReg->password,
                            &newReg->status);

        if (result != 6)
        {
            free(newReg);
            break;
        }

        newReg->nxt = NULL;

        if (registrationHead == NULL)
        {
            registrationHead = newReg;
        }
        else
        {
            Registration *temp = registrationHead;

            while (temp->nxt != NULL)
            {
                temp = temp->nxt;
            }

            temp->nxt = newReg;
        }
    }

    fclose(fp);
    printf("\n[SUCCESS] Registration records loaded successfully!\n");
}

void loadLoans()
{
    FILE *fp = fopen("loans.txt", "r");

    if (fp == NULL)
    {
        printf("\n[INFO] No existing loan records found.\n");
        return;
    }

    while (1)
    {
        Loan *newLoan = malloc(sizeof(Loan));

        if (newLoan == NULL)
        {
            printf("\n[ERROR] Memory allocation failed!\n");
            fclose(fp);
            return;
        }

        int result = fscanf(fp, "%d|%d|%lf|%d\n",
                            &newLoan->loanId,
                            &newLoan->accountNumber,
                            &newLoan->amount,
                            &newLoan->status);

        if (result != 4)
        {
            free(newLoan);
            break;
        }

        newLoan->nxt = NULL;

        if (loanHead == NULL)
        {
            loanHead = newLoan;
        }
        else
        {
            Loan *temp = loanHead;

            while (temp->nxt != NULL)
            {
                temp = temp->nxt;
            }

            temp->nxt = newLoan;
        }
    }

    fclose(fp);
    printf("\n[SUCCESS] Loan records loaded successfully!\n");
}

void loadComplaints()
{
    FILE *fp = fopen("complaints.txt", "r");

    if (fp == NULL)
    {
        printf("\n[INFO] No existing complaint records found.\n");
        return;
    }

    while (1)
    {
        Complaint *newComplaint = malloc(sizeof(Complaint));

        if (newComplaint == NULL)
        {
            printf("\n[ERROR] Memory allocation failed!\n");
            fclose(fp);
            return;
        }

        int result = fscanf(fp, "%d|%d|%d|%100[^\n]\n",
                            &newComplaint->complaintId,
                            &newComplaint->accountNumber,
                            &newComplaint->status,
                            newComplaint->text);

        if (result != 4)
        {
            free(newComplaint);
            break;
        }

        newComplaint->nxt = NULL;

        if (complaintHead == NULL)
        {
            complaintHead = newComplaint;
        }
        else
        {
            Complaint *temp = complaintHead;

            while (temp->nxt != NULL)
            {
                temp = temp->nxt;
            }

            temp->nxt = newComplaint;
        }
    }

    fclose(fp);
    printf("\n[SUCCESS] Complaint records loaded successfully!\n");
}

void loadTransactions()
{
    FILE *fp = fopen("transactions.txt", "r");

    if (fp == NULL)
    {
        printf("\n[INFO] No existing transaction records found.\n");
        return;
    }

    // First line holds the running totals
    int result = fscanf(fp, "%d|%d|%d|%d|%lf|%lf|%lf\n",
                        &txStack.totalCount,
                        &txStack.depositCount,
                        &txStack.withdrawalCount,
                        &txStack.transferCount,
                        &txStack.totalDeposits,
                        &txStack.totalWithdrawals,
                        &txStack.totalTransfers);

    if (result != 7)
    {
        initializeStack();
        fclose(fp);
        return;
    }

    // Remaining lines: oldest first, so the newest ends up on top
    while (txStack.top < STACK_CAPACITY - 1)
    {
        Transaction t;

        result = fscanf(fp, "%19[^|]|%d|%d|%d|%lf|%100[^\n]\n",
                        t.type,
                        &t.accountNumber,
                        &t.senderAccount,
                        &t.receiverAccount,
                        &t.amount,
                        t.description);

        if (result != 6)
        {
            break;
        }

        txStack.top++;
        txStack.items[txStack.top] = t;
    }

    fclose(fp);
    printf("\n[SUCCESS] Transaction records loaded successfully!\n");
}

void loadActivities()
{
    FILE *fp = fopen("activity.txt", "r");

    if (fp == NULL)
    {
        printf("\n[INFO] No existing activity log found.\n");
        return;
    }

    while (1)
    {
        Activity *newActivity = malloc(sizeof(Activity));

        if (newActivity == NULL)
        {
            printf("\n[ERROR] Memory allocation failed!\n");
            fclose(fp);
            return;
        }

        int result = fscanf(fp, "%25[^|]|%160[^\n]\n",
                            newActivity->timestamp,
                            newActivity->message);

        if (result != 2)
        {
            free(newActivity);
            break;
        }

        // Insert at head so the newest record is always first
        newActivity->nxt = activityHead;
        activityHead = newActivity;
    }

    fclose(fp);
    printf("\n[SUCCESS] Activity log loaded successfully!\n");
}

void loadCounters()
{
    FILE *fp = fopen("counters.txt", "r");

    if (fp == NULL)
    {
        return;
    }

    if (fscanf(fp, "%d %d %d %d", &nextAccountNumber, &nextRegistrationId,
               &nextLoanId, &nextComplaintId) != 4)
    {
        nextAccountNumber = 1001;
        nextRegistrationId = 1;
        nextLoanId = 1;
        nextComplaintId = 1;
    }

    fclose(fp);
}

// Make sure the ID counters are always higher than every existing ID
void fixCounters()
{
    Account *a;
    Registration *r;
    Loan *l;
    Complaint *c;

    for (a = accountHead; a != NULL; a = a->nxt)
    {
        if (a->accountNumber >= nextAccountNumber)
            nextAccountNumber = a->accountNumber + 1;
    }
    for (r = registrationHead; r != NULL; r = r->nxt)
    {
        if (r->requestId >= nextRegistrationId)
            nextRegistrationId = r->requestId + 1;
    }
    for (l = loanHead; l != NULL; l = l->nxt)
    {
        if (l->loanId >= nextLoanId)
            nextLoanId = l->loanId + 1;
    }
    for (c = complaintHead; c != NULL; c = c->nxt)
    {
        if (c->complaintId >= nextComplaintId)
            nextComplaintId = c->complaintId + 1;
    }
}


// Save Records to Text Files
void saveAdmins()
{
    FILE *fp = fopen("admins.txt", "w");

    if (fp == NULL)
    {
        printf("\n[ERROR] Unable to open admin file!\n");
        return;
    }

    Admin *temp = adminHead;
    while (temp != NULL)
    {
        fprintf(fp, "%s|%s\n", temp->username, temp->password);
        temp = temp->nxt;
    }

    fclose(fp);
}

void saveAccounts()
{
    FILE *fp = fopen("accounts.txt", "w");

    if (fp == NULL)
    {
        printf("\n[ERROR] Unable to open account file!\n");
        return;
    }

    Account *temp = accountHead;
    while (temp != NULL)
    {
        fprintf(fp, "%d|%s|%s|%.2f|%s|%d|%d\n",
                temp->accountNumber,
                temp->holderName,
                temp->accountType,
                temp->balance,
                temp->password,
                temp->isBlocked,
                temp->transactionCount);

        temp = temp->nxt;
    }

    fclose(fp);
}

void saveRegistrations()
{
    FILE *fp = fopen("registrations.txt", "w");

    if (fp == NULL)
    {
        printf("\n[ERROR] Unable to open registration file!\n");
        return;
    }

    Registration *temp = registrationHead;
    while (temp != NULL)
    {
        fprintf(fp, "%d|%s|%s|%.2f|%s|%d\n",
                temp->requestId,
                temp->name,
                temp->accountType,
                temp->initialBalance,
                temp->password,
                temp->status);

        temp = temp->nxt;
    }

    fclose(fp);
}

void saveLoans()
{
    FILE *fp = fopen("loans.txt", "w");

    if (fp == NULL)
    {
        printf("\n[ERROR] Unable to open loan file!\n");
        return;
    }

    Loan *temp = loanHead;
    while (temp != NULL)
    {
        fprintf(fp, "%d|%d|%.2f|%d\n",
                temp->loanId,
                temp->accountNumber,
                temp->amount,
                temp->status);

        temp = temp->nxt;
    }

    fclose(fp);
}

void saveComplaints()
{
    FILE *fp = fopen("complaints.txt", "w");

    if (fp == NULL)
    {
        printf("\n[ERROR] Unable to open complaint file!\n");
        return;
    }

    Complaint *temp = complaintHead;
    while (temp != NULL)
    {
        fprintf(fp, "%d|%d|%d|%s\n",
                temp->complaintId,
                temp->accountNumber,
                temp->status,
                temp->text);

        temp = temp->nxt;
    }

    fclose(fp);
}

void saveTransactions()
{
    int i;
    FILE *fp = fopen("transactions.txt", "w");

    if (fp == NULL)
    {
        printf("\n[ERROR] Unable to open transaction file!\n");
        return;
    }

    fprintf(fp, "%d|%d|%d|%d|%.2f|%.2f|%.2f\n",
            txStack.totalCount,
            txStack.depositCount,
            txStack.withdrawalCount,
            txStack.transferCount,
            txStack.totalDeposits,
            txStack.totalWithdrawals,
            txStack.totalTransfers);

    for (i = 0; i <= txStack.top; i++)
    {
        fprintf(fp, "%s|%d|%d|%d|%.2f|%s\n",
                txStack.items[i].type,
                txStack.items[i].accountNumber,
                txStack.items[i].senderAccount,
                txStack.items[i].receiverAccount,
                txStack.items[i].amount,
                txStack.items[i].description);
    }

    fclose(fp);
}

void saveCounters()
{
    FILE *fp = fopen("counters.txt", "w");

    if (fp == NULL)
    {
        printf("\n[ERROR] Unable to open counter file!\n");
        return;
    }

    fprintf(fp, "%d %d %d %d\n", nextAccountNumber, nextRegistrationId,
            nextLoanId, nextComplaintId);
    fclose(fp);
}


// INPUT HELPERS
void clearInput()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

int readInt(const char *message)
{
    int value;

    printf("%s", message);
    while (scanf("%d", &value) != 1)
    {
        if (feof(stdin))
        {
            exit(0);
        }
        clearInput();
        printf("Invalid input! Please enter a whole number: ");
    }
    clearInput();
    return value;
}

double readDouble(const char *message)
{
    double value;

    printf("%s", message);
    while (scanf("%lf", &value) != 1)
    {
        if (feof(stdin))
        {
            exit(0);
        }
        clearInput();
        printf("Invalid input! Please enter a valid amount: ");
    }
    clearInput();
    return value;
}

double readPositiveAmount(const char *message)
{
    double amount = readDouble(message);

    while (amount <= 0 || amount > 1000000000.0)
    {
        amount = readDouble("Amount must be greater than 0 (and below 1,000,000,000): ");
    }
    return amount;
}

// Reads a full line. The '|' character is replaced because it is the file separator.
void readText(const char *message, char *destination, int destinationSize)
{
    size_t i;

    printf("%s", message);

    while (1)
    {
        if (fgets(destination, destinationSize, stdin) == NULL)
        {
            exit(0);
        }

        // If the line was longer than the buffer, throw away the rest
        if (strchr(destination, '\n') == NULL)
        {
            clearInput();
        }
        destination[strcspn(destination, "\n")] = '\0';

        if (strlen(destination) > 0)
        {
            break;
        }
        printf("This field cannot be empty. Please try again: ");
    }

    for (i = 0; destination[i] != '\0'; i++)
    {
        if (destination[i] == '|')
        {
            destination[i] = '/';
        }
    }
}

int confirmAction(const char *message)
{
    char line[100];

    printf("%s (y/n): ", message);
    if (fgets(line, sizeof(line), stdin) == NULL)
    {
        return 0;
    }
    return (line[0] == 'y' || line[0] == 'Y');
}

int containsIgnoreCase(const char *text, const char *pattern)
{
    size_t i, j;
    size_t textLen = strlen(text);
    size_t patLen = strlen(pattern);

    if (patLen == 0)
    {
        return 1;
    }
    for (i = 0; i + patLen <= textLen; i++)
    {
        for (j = 0; j < patLen; j++)
        {
            if (tolower((unsigned char)text[i + j]) != tolower((unsigned char)pattern[j]))
            {
                break;
            }
        }
        if (j == patLen)
        {
            return 1;
        }
    }
    return 0;
}


// STACK (Transaction History)
void initializeStack()
{
    txStack.top = -1;
    txStack.totalCount = 0;
    txStack.depositCount = 0;
    txStack.withdrawalCount = 0;
    txStack.transferCount = 0;
    txStack.totalDeposits = 0;
    txStack.totalWithdrawals = 0;
    txStack.totalTransfers = 0;
}

int pushTransaction(Transaction transaction)
{
    // Running totals are updated first so reports stay correct even if the stack is full
    txStack.totalCount++;
    if (strcmp(transaction.type, "DEPOSIT") == 0)
    {
        txStack.depositCount++;
        txStack.totalDeposits += transaction.amount;
    }
    else if (strcmp(transaction.type, "WITHDRAWAL") == 0)
    {
        txStack.withdrawalCount++;
        txStack.totalWithdrawals += transaction.amount;
    }
    else if (strcmp(transaction.type, "TRANSFER") == 0)
    {
        txStack.transferCount++;
        txStack.totalTransfers += transaction.amount;
    }

    if (txStack.top == STACK_CAPACITY - 1)
    {
        printf("\n[INFO] Transaction history is full. This transaction was not recorded in history.\n");
        return 0;
    }

    txStack.top++;
    txStack.items[txStack.top] = transaction;
    return 1;
}


// QUEUE (Customer Waiting Line)
void initializeQueue()
{
    waitingQueue.front = -1;
    waitingQueue.rear = -1;
    waitingQueue.size = 0;
}

int isQueueFull()
{
    return waitingQueue.size == QUEUE_CAPACITY;
}

int isQueueEmpty()
{
    return waitingQueue.size == 0;
}

int enqueueCustomer(Customer customer)
{
    if (isQueueFull())
    {
        printf("\n[ERROR] Waiting queue is full. The customer could not be added.\n");
        return 0;
    }

    if (isQueueEmpty())
    {
        waitingQueue.front = 0;
    }

    waitingQueue.rear = (waitingQueue.rear + 1) % QUEUE_CAPACITY;
    waitingQueue.items[waitingQueue.rear] = customer;
    waitingQueue.size++;
    return 1;
}

int dequeueCustomer(Customer *removedCustomer)
{
    if (isQueueEmpty())
    {
        printf("\n[INFO] Waiting queue is empty. No customer to serve.\n");
        return 0;
    }

    *removedCustomer = waitingQueue.items[waitingQueue.front];

    if (waitingQueue.size == 1)
    {
        waitingQueue.front = -1;
        waitingQueue.rear = -1;
    }
    else
    {
        waitingQueue.front = (waitingQueue.front + 1) % QUEUE_CAPACITY;
    }

    waitingQueue.size--;
    return 1;
}

void displayQueue()
{
    int i;
    int index;

    if (isQueueEmpty())
    {
        printf("\n[INFO] Waiting queue is empty.\n");
        return;
    }

    printf("\n--- Customers Waiting (next to be served first) ---\n");
    index = waitingQueue.front;

    for (i = 0; i < waitingQueue.size; i++)
    {
        printf("%d. Queue #%d | %s", i + 1, waitingQueue.items[index].queueNumber,
               waitingQueue.items[index].name);

        if (waitingQueue.items[index].accountNumber != 0)
        {
            printf(" | Account #%d", waitingQueue.items[index].accountNumber);
        }
        else
        {
            printf(" | No account");
        }
        printf("\n");

        index = (index + 1) % QUEUE_CAPACITY;
    }
}


// ACTIVITY LOG (Linked List, newest at head)
void addActivity(const char *message)
{
    FILE *fp;
    time_t now;
    struct tm *timeInfo;
    Activity *node = malloc(sizeof(Activity));

    if (node == NULL)
    {
        return; // logging must never crash the program
    }

    now = time(NULL);
    timeInfo = localtime(&now);
    if (timeInfo != NULL)
    {
        strftime(node->timestamp, sizeof(node->timestamp), "%Y-%m-%d %H:%M:%S", timeInfo);
    }
    else
    {
        strcpy(node->timestamp, "unknown time");
    }
    snprintf(node->message, sizeof(node->message), "%s", message);

    node->nxt = activityHead;
    activityHead = node;

    fp = fopen("activity.txt", "a");
    if (fp != NULL)
    {
        fprintf(fp, "%s|%s\n", node->timestamp, node->message);
        fclose(fp);
    }
}

void logAdminAction(const char *action)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "[%s] %s", currentAdmin, action);
    addActivity(buffer);
}

void viewActivityLog()
{
    Activity *temp = activityHead;
    int count = 0;

    printf("\n--- Activity Log (newest first) ---\n");

    if (temp == NULL)
    {
        printf("\n[INFO] The activity log is empty.\n");
        return;
    }

    while (temp != NULL)
    {
        printf("%s | %s\n", temp->timestamp, temp->message);
        temp = temp->nxt;
        count++;
    }
    printf("\nTotal records: %d\n", count);
}


// LOGIN
void login(int role)
{
    char buffer[100];

    if (role == ADMIN)
    {
        char username[31];
        char password[31];

        printf("\n--- Admin Login ---\n");
        readText("Enter Username: ", username, 31);
        readText("Enter Password: ", password, 31);

        Admin *temp = adminHead;
        int found = 0;

        while (temp != NULL)
        {
            if (!strcmp(username, temp->username) && !strcmp(password, temp->password))
            {
                found = 1;
                break;
            }
            temp = temp->nxt;
        }

        if (found)
        {
            printf("\n[SUCCESS] Welcome, %s! Login successful.\n", temp->username);
            strcpy(currentAdmin, temp->username);
            logAdminAction("Admin logged in");
            adminPanel();
            logAdminAction("Admin logged out");
            currentAdmin[0] = '\0';
        }
        else
        {
            printf("\n[ERROR] Invalid Username or Password for Admin login.\n");
            snprintf(buffer, sizeof(buffer), "Failed admin login attempt (username: %s)", username);
            addActivity(buffer);
        }
    }
    else if (role == CUSTOMER)
    {
        char password[31];
        int accountNumber;

        printf("\n--- Customer Login ---\n");
        accountNumber = readInt("Enter Account Number: ");
        readText("Enter Password: ", password, 31);

        Account *temp = findAccount(accountNumber);

        if (temp != NULL && !strcmp(password, temp->password))
        {
            if (temp->isBlocked)
            {
                printf("\n[ERROR] Your account is BLOCKED. Please contact the bank.\n");
                return;
            }
            printf("\n[SUCCESS] Welcome, %s! Login successful.\n", temp->holderName);
            snprintf(buffer, sizeof(buffer), "Customer %d logged in", accountNumber);
            addActivity(buffer);
            customerPanel(temp);
        }
        else
        {
            printf("\n[ERROR] Invalid Account Number or Password for Customer login.\n");
        }
    }
}


// REGISTRATION REQUEST (public)
void submitRegistrationRequest()
{
    int typeChoice;

    printf("\n--- Open New Account (Registration Request) ---\n\n");

    Registration *newReg = malloc(sizeof(Registration));

    if (newReg == NULL)
    {
        printf("\n[ERROR] Memory allocation failed!\n");
        return;
    }

    readText("Enter Customer Name: ", newReg->name, 51);

    printf("1. Savings\n2. Current\n");
    typeChoice = readInt("Select Account Type: ");
    if (typeChoice == 1)
    {
        strcpy(newReg->accountType, "Savings");
    }
    else if (typeChoice == 2)
    {
        strcpy(newReg->accountType, "Current");
    }
    else
    {
        printf("\n[ERROR] Invalid account type. Request not saved.\n");
        free(newReg);
        return;
    }

    newReg->initialBalance = readDouble("Enter Initial Deposit: ");
    if (newReg->initialBalance < 0)
    {
        printf("\n[ERROR] Initial deposit cannot be negative. Request not saved.\n");
        free(newReg);
        return;
    }

    readText("Choose a Password: ", newReg->password, 31);

    newReg->requestId = nextRegistrationId++;
    newReg->status = STATUS_PENDING;
    newReg->nxt = NULL;

    if (registrationHead == NULL)
    {
        registrationHead = newReg;
    }
    else
    {
        Registration *temp = registrationHead;
        while (temp->nxt != NULL)
        {
            temp = temp->nxt;
        }
        temp->nxt = newReg;
    }

    saveRegistrations();
    saveCounters();
    printf("\n[SUCCESS] Registration request submitted. Request ID: %d (Status: Pending)\n",
           newReg->requestId);
}


// ADMIN PANEL
void adminPanel()
{
    int choice;

    while (1)
    {
        printf("\n========== ADMIN PANEL ==========\n");
        printf("Logged in as: %s\n", currentAdmin);

        printf("\n---------- ACCOUNT MANAGEMENT ----------\n");
        printf("1. View All Accounts\n");
        printf("2. Create Account\n");
        printf("3. Search Account (by Account Number)\n");
        printf("4. Search Account (by Name)\n");
        printf("5. Update Account\n");
        printf("6. Delete Account\n");

        printf("\n---------- TRANSACTIONS ----------\n");
        printf("7. Deposit Money\n");
        printf("8. Withdraw Money\n");
        printf("9. Transfer Money\n");
        printf("10. View All Transactions\n");

        printf("\n---------- REGISTRATION & SECURITY ----------\n");
        printf("11. Approve or Reject Registration\n");
        printf("12. Block and Unblock Accounts\n");
        printf("13. Reset Customer Password\n");

        printf("\n---------- LOANS & COMPLAINTS ----------\n");
        printf("14. Loan Management\n");
        printf("15. Handle Complaints\n");

        printf("\n---------- CUSTOMER QUEUE ----------\n");
        printf("16. Manage Waiting Queue\n");

        printf("\n---------- REPORTS ----------\n");
        printf("17. Dashboard Overview\n");
        printf("18. Generate Reports\n");
        printf("19. Activity Log\n");
        printf("20. Delete Inactive Users\n");

        printf("\n0. Logout\n");

        choice = readInt("\nEnter your choice: ");

        switch (choice)
        {
        case 1:
            viewAllAccounts();
            break;
        case 2:
            createAccount();
            break;
        case 3:
            searchAccountByNumber();
            break;
        case 4:
            searchAccountByName();
            break;
        case 5:
            updateAccount();
            break;
        case 6:
            deleteAccount();
            break;
        case 7:
            adminDeposit();
            break;
        case 8:
            adminWithdraw();
            break;
        case 9:
            adminTransfer();
            break;
        case 10:
            viewAllTransactions();
            break;
        case 11:
            manageRegistrations();
            break;
        case 12:
            blockUnblockMenu();
            break;
        case 13:
            resetCustomerPassword();
            break;
        case 14:
            loanManagement();
            break;
        case 15:
            handleComplaints();
            break;
        case 16:
            manageQueue();
            break;
        case 17:
            dashboardOverview();
            break;
        case 18:
            generateReports();
            break;
        case 19:
            viewActivityLog();
            break;
        case 20:
            deleteInactiveUsers();
            break;
        case 0:
            printf("\nLogging out from Admin Panel...\n");
            return;

        default:
            printf("\nInvalid choice! Please try again.\n");
        }
    }
}


// ACCOUNT MANAGEMENT (Admin)
void viewAllAccounts()
{
    printf("\n--- View All Accounts ---\n");

    if (accountHead == NULL)
    {
        printf("\nNo accounts found in the system.\n");
        return;
    }

    int count = 0;
    Account *temp = accountHead;

    printf("\n%-12s %-22s %-10s %-14s %-9s\n",
           "Account No.", "Holder Name", "Type", "Balance", "Status");

    while (temp != NULL)
    {
        printf("%-12d %-22s %-10s %-14.2f %-9s\n", temp->accountNumber,
               temp->holderName, temp->accountType, temp->balance,
               accountStatusText(temp));
        temp = temp->nxt;
        count++;
    }
    printf("\nTotal accounts in the system: %d\n", count);
}

void createAccount()
{
    char name[51];
    char accountType[10];
    double initialBalance;
    int typeChoice;
    char buffer[100];

    printf("\n--- Create New Account ---\n\n");

    readText("Enter Account Holder Name: ", name, 51);

    printf("1. Savings\n2. Current\n");
    typeChoice = readInt("Select Account Type: ");

    if (typeChoice == 1)
    {
        strcpy(accountType, "Savings");
    }
    else if (typeChoice == 2)
    {
        strcpy(accountType, "Current");
    }
    else
    {
        printf("\n[ERROR] Invalid account type. Account was not created.\n");
        return;
    }

    initialBalance = readDouble("Enter Initial Balance: ");
    if (initialBalance < 0)
    {
        printf("\n[ERROR] Initial balance cannot be negative. Account was not created.\n");
        return;
    }

    Account *newAccount = addAccountToList(name, accountType, initialBalance, "changeme");

    if (newAccount == NULL)
    {
        printf("\n[ERROR] Memory allocation failed!\n");
        return;
    }

    saveAccounts();
    saveCounters();
    printf("\n[SUCCESS] Account created successfully! Account Number: %d\n",
           newAccount->accountNumber);
    printf("[INFO] Default password is 'changeme'.\n");

    snprintf(buffer, sizeof(buffer), "Created account %d", newAccount->accountNumber);
    logAdminAction(buffer);
}

void searchAccountByNumber()
{
    printf("\n--- Search Account ---\n");

    if (accountHead == NULL)
    {
        printf("\nNo accounts available to search!\n");
        return;
    }

    int accountNumber = readInt("Enter Account Number: ");
    Account *temp = findAccount(accountNumber);

    if (temp == NULL)
    {
        printf("\n[ERROR] No account found with Account Number: %d\n", accountNumber);
        return;
    }

    printf("\n[SUCCESS] Account found!\n");
    printf("\n========== Account Details ==========\n");
    displayAccountDetails(temp);
}

void searchAccountByName()
{
    char name[51];
    int found = 0;

    printf("\n--- Search Account by Name ---\n");

    if (accountHead == NULL)
    {
        printf("\nNo accounts available to search!\n");
        return;
    }

    readText("Enter Name (or part of a name): ", name, 51);

    Account *temp = accountHead;
    while (temp != NULL)
    {
        if (containsIgnoreCase(temp->holderName, name))
        {
            found++;
            printf("\n--- Account %d ---\n", found);
            displayAccountDetails(temp);
        }
        temp = temp->nxt;
    }

    if (found == 0)
    {
        printf("\n[ERROR] No customer found with that name.\n");
    }
    else
    {
        printf("\nTotal accounts found: %d\n", found);
    }
}

void updateAccount()
{
    printf("\n--- Update Account Information ---\n");

    if (accountHead == NULL)
    {
        printf("\nNo accounts available to update!\n");
        return;
    }

    int accountNumber = readInt("Enter Account Number: ");
    Account *temp = findAccount(accountNumber);
    int choice;
    char buffer[100];

    if (temp == NULL)
    {
        printf("\n[ERROR] No account found with Account Number: %d\n", accountNumber);
        return;
    }

    printf("\n[SUCCESS] Account found!\n");
    printf("\n--- Current Account Information ---\n");
    displayAccountDetails(temp);

    while (1)
    {
        printf("\n========== Update Menu ==========\n");
        printf("1. Update Holder Name\n");
        printf("2. Update Account Type\n");
        printf("0. Finish Updating\n");

        choice = readInt("\nEnter your choice: ");

        switch (choice)
        {
        case 1:
            readText("Enter New Holder Name: ", temp->holderName, 51);
            printf("\n[SUCCESS] Holder name updated successfully!\n");
            break;

        case 2:
        {
            int typeChoice;
            printf("1. Savings\n2. Current\n");
            typeChoice = readInt("Enter New Account Type: ");
            if (typeChoice == 1)
            {
                strcpy(temp->accountType, "Savings");
                printf("\n[SUCCESS] Account type updated successfully!\n");
            }
            else if (typeChoice == 2)
            {
                strcpy(temp->accountType, "Current");
                printf("\n[SUCCESS] Account type updated successfully!\n");
            }
            else
            {
                printf("\n[ERROR] Invalid choice! Nothing changed.\n");
            }
            break;
        }

        case 0:
            printf("\n[INFO] Updating process completed!\n");
            printf("\n--- Updated Account Information ---\n");
            displayAccountDetails(temp);
            saveAccounts();
            snprintf(buffer, sizeof(buffer), "Updated information of account %d", accountNumber);
            logAdminAction(buffer);
            return;

        default:
            printf("\n[ERROR] Invalid choice! Please try again.\n");
        }
    }
}

void deleteAccount()
{
    printf("\n--- Delete Account ---\n");

    if (accountHead == NULL)
    {
        printf("\nNo accounts available to delete!\n");
        return;
    }

    int accountNumber = readInt("Enter Account Number: ");
    Account *temp = findAccount(accountNumber);
    char buffer[100];

    if (temp == NULL)
    {
        printf("\n[ERROR] No account found with Account Number: %d\n", accountNumber);
        return;
    }

    printf("\n--- Account Information ---\n");
    displayAccountDetails(temp);

    if (confirmAction("\nAre you sure you want to delete this account?"))
    {
        removeAccount(accountNumber);
        saveAccounts();
        printf("\n[SUCCESS] Account deleted successfully!\n");
        snprintf(buffer, sizeof(buffer), "Deleted account %d", accountNumber);
        logAdminAction(buffer);
    }
    else
    {
        printf("\n[INFO] Account deletion cancelled.\n");
    }
}


// TRANSACTIONS
int performDeposit(Account *account, double amount)
{
    Transaction transaction;

    if (account->isBlocked)
    {
        printf("\n[ERROR] This account is BLOCKED. Deposit cancelled.\n");
        return 0;
    }

    account->balance += amount;
    account->transactionCount++;

    strcpy(transaction.type, "DEPOSIT");
    transaction.accountNumber = account->accountNumber;
    transaction.senderAccount = 0;
    transaction.receiverAccount = account->accountNumber;
    transaction.amount = amount;
    strcpy(transaction.description, "Money deposited into account");
    pushTransaction(transaction);

    saveAccounts();
    saveTransactions();
    printf("\n[SUCCESS] Deposit successful. New balance: %.2f\n", account->balance);
    return 1;
}

int performWithdraw(Account *account, double amount)
{
    Transaction transaction;

    if (account->isBlocked)
    {
        printf("\n[ERROR] This account is BLOCKED. Withdrawal cancelled.\n");
        return 0;
    }
    if (amount > account->balance)
    {
        printf("\n[ERROR] Insufficient balance. Withdrawal cancelled.\n");
        return 0;
    }

    account->balance -= amount;
    account->transactionCount++;

    strcpy(transaction.type, "WITHDRAWAL");
    transaction.accountNumber = account->accountNumber;
    transaction.senderAccount = account->accountNumber;
    transaction.receiverAccount = 0;
    transaction.amount = amount;
    strcpy(transaction.description, "Money withdrawn from account");
    pushTransaction(transaction);

    saveAccounts();
    saveTransactions();
    printf("\n[SUCCESS] Withdrawal successful. New balance: %.2f\n", account->balance);
    return 1;
}

int performTransfer(Account *sender, Account *receiver, double amount)
{
    Transaction transaction;

    if (sender->accountNumber == receiver->accountNumber)
    {
        printf("\n[ERROR] Sender and receiver cannot be the same account.\n");
        return 0;
    }
    if (sender->isBlocked)
    {
        printf("\n[ERROR] Sender account is BLOCKED. Transfer cancelled.\n");
        return 0;
    }
    if (receiver->isBlocked)
    {
        printf("\n[ERROR] Receiver account is BLOCKED. Transfer cancelled.\n");
        return 0;
    }
    if (amount > sender->balance)
    {
        printf("\n[ERROR] Insufficient balance. Transfer cancelled.\n");
        return 0;
    }

    sender->balance -= amount;
    receiver->balance += amount;
    sender->transactionCount++;
    receiver->transactionCount++;

    strcpy(transaction.type, "TRANSFER");
    transaction.accountNumber = sender->accountNumber;
    transaction.senderAccount = sender->accountNumber;
    transaction.receiverAccount = receiver->accountNumber;
    transaction.amount = amount;
    strcpy(transaction.description, "Money transferred between accounts");
    pushTransaction(transaction);

    saveAccounts();
    saveTransactions();
    printf("\n[SUCCESS] Transfer successful. Sender balance: %.2f | Receiver balance: %.2f\n",
           sender->balance, receiver->balance);
    return 1;
}

void adminDeposit()
{
    printf("\n--- Deposit Money ---\n");

    int accountNumber = readInt("Enter Account Number: ");
    Account *account = findAccount(accountNumber);

    if (account == NULL)
    {
        printf("\n[ERROR] Account not found. Deposit cancelled.\n");
        return;
    }

    double amount = readPositiveAmount("Enter Deposit Amount: ");
    performDeposit(account, amount);
}

void adminWithdraw()
{
    printf("\n--- Withdraw Money ---\n");

    int accountNumber = readInt("Enter Account Number: ");
    Account *account = findAccount(accountNumber);

    if (account == NULL)
    {
        printf("\n[ERROR] Account not found. Withdrawal cancelled.\n");
        return;
    }

    double amount = readPositiveAmount("Enter Withdrawal Amount: ");
    performWithdraw(account, amount);
}

void adminTransfer()
{
    printf("\n--- Transfer Money ---\n");

    int senderNumber = readInt("Enter Sender Account Number: ");
    int receiverNumber = readInt("Enter Receiver Account Number: ");

    if (senderNumber == receiverNumber)
    {
        printf("\n[ERROR] Sender and receiver cannot be the same account.\n");
        return;
    }

    Account *sender = findAccount(senderNumber);
    Account *receiver = findAccount(receiverNumber);

    if (sender == NULL)
    {
        printf("\n[ERROR] Sender account not found. Transfer cancelled.\n");
        return;
    }
    if (receiver == NULL)
    {
        printf("\n[ERROR] Receiver account not found. Transfer cancelled.\n");
        return;
    }

    double amount = readPositiveAmount("Enter Transfer Amount: ");
    performTransfer(sender, receiver, amount);
}

// filter = NULL shows every type. accountNumber = 0 shows every account.
void showTransactions(const char *filter, int accountNumber)
{
    int i;
    int count = 0;
    char sender[16];
    char receiver[16];

    for (i = txStack.top; i >= 0; i--) // top of stack = newest
    {
        Transaction *t = &txStack.items[i];

        if (filter != NULL && strcmp(t->type, filter) != 0)
        {
            continue;
        }
        if (accountNumber != 0 && t->senderAccount != accountNumber &&
            t->receiverAccount != accountNumber)
        {
            continue;
        }
        if (count == 0)
        {
            printf("\n%-12s %-10s %-10s %-14s %s\n", "Type", "Sender", "Receiver", "Amount", "Description");
        }
        if (t->senderAccount == 0)
            strcpy(sender, "-");
        else
            snprintf(sender, sizeof(sender), "%d", t->senderAccount);
        if (t->receiverAccount == 0)
            strcpy(receiver, "-");
        else
            snprintf(receiver, sizeof(receiver), "%d", t->receiverAccount);

        printf("%-12s %-10s %-10s %-14.2f %s\n", t->type, sender, receiver, t->amount, t->description);
        count++;
    }
    if (count == 0)
    {
        printf("\n[INFO] No matching transactions found.\n");
    }
}

void viewAllTransactions()
{
    int choice;

    while (1)
    {
        printf("\n--- View All Transactions (newest first) ---\n");
        printf("1. All Transactions\n");
        printf("2. Deposits Only\n");
        printf("3. Withdrawals Only\n");
        printf("4. Transfers Only\n");
        printf("5. Loan Credits Only\n");
        printf("0. Back\n");

        choice = readInt("\nEnter your choice: ");

        switch (choice)
        {
        case 1:
            showTransactions(NULL, 0);
            break;
        case 2:
            showTransactions("DEPOSIT", 0);
            break;
        case 3:
            showTransactions("WITHDRAWAL", 0);
            break;
        case 4:
            showTransactions("TRANSFER", 0);
            break;
        case 5:
            showTransactions("LOAN", 0);
            break;
        case 0:
            return;
        default:
            printf("\n[ERROR] Invalid choice! Please try again.\n");
        }
    }
}


// REGISTRATION MANAGEMENT (Admin)
int displayRegistrations(int onlyPending)
{
    Registration *temp = registrationHead;
    int count = 0;

    while (temp != NULL)
    {
        if (onlyPending && temp->status != STATUS_PENDING)
        {
            temp = temp->nxt;
            continue;
        }
        if (count == 0)
        {
            printf("\n%-8s %-22s %-10s %-14s %-9s\n",
                   "Req ID", "Name", "Type", "Initial Dep.", "Status");
        }
        printf("%-8d %-22s %-10s %-14.2f %-9s\n", temp->requestId,
               temp->name, temp->accountType, temp->initialBalance,
               statusText(temp->status));
        count++;
        temp = temp->nxt;
    }
    if (count == 0)
    {
        printf("\n[INFO] No registration requests to display.\n");
    }
    return count;
}

// approve = 1 creates the account, approve = 0 rejects the request
void processRegistration(int approve)
{
    char buffer[150];

    if (displayRegistrations(1) == 0)
    {
        return;
    }

    int requestId = readInt("\nEnter Request ID: ");
    Registration *reg = registrationHead;

    while (reg != NULL)
    {
        if (reg->requestId == requestId)
        {
            break;
        }
        reg = reg->nxt;
    }

    if (reg == NULL)
    {
        printf("\n[ERROR] Request not found.\n");
        return;
    }
    if (reg->status != STATUS_PENDING)
    {
        printf("\n[INFO] This request has already been %s.\n", statusText(reg->status));
        return;
    }

    if (approve)
    {
        Account *account = addAccountToList(reg->name, reg->accountType,
                                            reg->initialBalance, reg->password);
        if (account == NULL)
        {
            printf("\n[ERROR] Memory allocation failed! Request stays pending.\n");
            return;
        }
        reg->status = STATUS_APPROVED;
        saveAccounts();
        saveCounters();
        printf("\n[SUCCESS] Request approved. New Account Number: %d\n", account->accountNumber);
        snprintf(buffer, sizeof(buffer),
                 "Approved registration request %d (account %d created)",
                 requestId, account->accountNumber);
    }
    else
    {
        reg->status = STATUS_REJECTED;
        printf("\n[SUCCESS] Request %d rejected.\n", requestId);
        snprintf(buffer, sizeof(buffer), "Rejected registration request %d", requestId);
    }

    saveRegistrations();
    logAdminAction(buffer);
}

void manageRegistrations()
{
    int choice;

    while (1)
    {
        printf("\n--- Approve or Reject Registration ---\n");
        printf("1. Display Pending Requests\n");
        printf("2. Approve a Request\n");
        printf("3. Reject a Request\n");
        printf("4. View All Requests (with status)\n");
        printf("0. Back\n");

        choice = readInt("\nEnter your choice: ");

        switch (choice)
        {
        case 1:
            displayRegistrations(1);
            break;
        case 2:
            processRegistration(1);
            break;
        case 3:
            processRegistration(0);
            break;
        case 4:
            displayRegistrations(0);
            break;
        case 0:
            return;
        default:
            printf("\n[ERROR] Invalid choice! Please try again.\n");
        }
    }
}


// BLOCK / UNBLOCK / RESET PASSWORD
// mode: 1 = block, 0 = unblock
void changeBlockStatus(int mode)
{
    int accountNumber = readInt("Enter Account Number: ");
    Account *account = findAccount(accountNumber);
    char buffer[100];

    if (account == NULL)
    {
        printf("\n[ERROR] Account not found.\n");
        return;
    }
    if (mode == 1 && account->isBlocked)
    {
        printf("\n[INFO] Account %d is already blocked.\n", accountNumber);
        return;
    }
    if (mode == 0 && !account->isBlocked)
    {
        printf("\n[INFO] Account %d is not blocked.\n", accountNumber);
        return;
    }

    account->isBlocked = mode;
    saveAccounts();
    printf("\n[SUCCESS] Account %d is now %s.\n", accountNumber, accountStatusText(account));
    snprintf(buffer, sizeof(buffer), "%s account %d", mode ? "Blocked" : "Unblocked", accountNumber);
    logAdminAction(buffer);
}

void blockUnblockMenu()
{
    int choice;

    while (1)
    {
        printf("\n--- Block and Unblock Accounts ---\n");
        printf("1. Block an Account\n");
        printf("2. Unblock an Account\n");
        printf("3. Display Account Status\n");
        printf("0. Back\n");

        choice = readInt("\nEnter your choice: ");

        switch (choice)
        {
        case 1:
            changeBlockStatus(1);
            break;
        case 2:
            changeBlockStatus(0);
            break;
        case 3:
        {
            int accountNumber = readInt("Enter Account Number: ");
            Account *account = findAccount(accountNumber);
            if (account == NULL)
            {
                printf("\n[ERROR] Account not found.\n");
            }
            else
            {
                printf("\nAccount %d (%s) status: %s\n", account->accountNumber,
                       account->holderName, accountStatusText(account));
            }
            break;
        }
        case 0:
            return;
        default:
            printf("\n[ERROR] Invalid choice! Please try again.\n");
        }
    }
}

void resetCustomerPassword()
{
    printf("\n--- Reset Customer Password ---\n");

    int accountNumber = readInt("Enter Account Number: ");
    Account *account = findAccount(accountNumber);
    char newPassword[31];
    char again[31];
    char buffer[100];

    if (account == NULL)
    {
        printf("\n[ERROR] Account not found.\n");
        return;
    }

    printf("Customer: %s\n", account->holderName);
    readText("Enter New Password: ", newPassword, 31);
    readText("Re-enter New Password: ", again, 31);

    if (strcmp(newPassword, again) != 0)
    {
        printf("\n[ERROR] Passwords do not match. Reset cancelled.\n");
        return;
    }
    if (!confirmAction("Confirm password reset?"))
    {
        printf("\n[INFO] Password reset cancelled.\n");
        return;
    }

    strcpy(account->password, newPassword);
    saveAccounts();
    printf("\n[SUCCESS] Password reset successfully!\n");
    snprintf(buffer, sizeof(buffer), "Reset password of account %d", accountNumber);
    logAdminAction(buffer);
}


// LOAN MANAGEMENT
void applyForLoan(Account *account)
{
    Loan *newLoan;

    if (account->isBlocked)
    {
        printf("\n[ERROR] This account is BLOCKED. Loan application cancelled.\n");
        return;
    }

    newLoan = malloc(sizeof(Loan));
    if (newLoan == NULL)
    {
        printf("\n[ERROR] Memory allocation failed!\n");
        return;
    }

    newLoan->loanId = nextLoanId++;
    newLoan->accountNumber = account->accountNumber;
    newLoan->amount = readPositiveAmount("Enter Loan Amount: ");
    newLoan->status = STATUS_PENDING;
    newLoan->nxt = NULL;

    if (loanHead == NULL)
    {
        loanHead = newLoan;
    }
    else
    {
        Loan *temp = loanHead;
        while (temp->nxt != NULL)
        {
            temp = temp->nxt;
        }
        temp->nxt = newLoan;
    }

    saveLoans();
    saveCounters();
    printf("\n[SUCCESS] Loan application submitted. Loan ID: %d (Status: Pending)\n", newLoan->loanId);
}

// accountNumber = 0 shows every loan, otherwise only that account's loans
int displayLoans(int onlyPending, int accountNumber)
{
    Loan *temp = loanHead;
    int count = 0;

    while (temp != NULL)
    {
        if ((onlyPending && temp->status != STATUS_PENDING) ||
            (accountNumber != 0 && temp->accountNumber != accountNumber))
        {
            temp = temp->nxt;
            continue;
        }
        if (count == 0)
        {
            printf("\n%-8s %-14s %-14s %-9s\n", "Loan ID", "Account No.", "Amount", "Status");
        }
        printf("%-8d %-14d %-14.2f %-9s\n", temp->loanId, temp->accountNumber,
               temp->amount, statusText(temp->status));
        count++;
        temp = temp->nxt;
    }
    if (count == 0)
    {
        printf("\n[INFO] No loan applications to display.\n");
    }
    return count;
}

// approve = 1 credits the loan amount to the account, approve = 0 rejects it
void processLoan(int approve)
{
    char buffer[150];

    if (displayLoans(1, 0) == 0)
    {
        return;
    }

    int loanId = readInt("\nEnter Loan ID: ");
    Loan *loan = loanHead;

    while (loan != NULL)
    {
        if (loan->loanId == loanId)
        {
            break;
        }
        loan = loan->nxt;
    }

    if (loan == NULL)
    {
        printf("\n[ERROR] Loan not found.\n");
        return;
    }
    if (loan->status != STATUS_PENDING)
    {
        printf("\n[INFO] This loan has already been %s.\n", statusText(loan->status));
        return;
    }

    if (approve)
    {
        Transaction transaction;
        Account *account = findAccount(loan->accountNumber);

        if (account == NULL)
        {
            printf("\n[ERROR] The customer account no longer exists. Cannot approve.\n");
            return;
        }
        if (account->isBlocked)
        {
            printf("\n[ERROR] The customer account is BLOCKED. Cannot approve.\n");
            return;
        }

        account->balance += loan->amount;
        account->transactionCount++;
        loan->status = STATUS_APPROVED;

        strcpy(transaction.type, "LOAN");
        transaction.accountNumber = loan->accountNumber;
        transaction.senderAccount = 0;
        transaction.receiverAccount = loan->accountNumber;
        transaction.amount = loan->amount;
        strcpy(transaction.description, "Approved loan credited to account");
        pushTransaction(transaction);

        saveAccounts();
        saveTransactions();
        printf("\n[SUCCESS] Loan %d approved. %.2f credited to account %d.\n",
               loanId, loan->amount, loan->accountNumber);
        snprintf(buffer, sizeof(buffer), "Approved loan %d (%.2f) for account %d",
                 loanId, loan->amount, loan->accountNumber);
    }
    else
    {
        loan->status = STATUS_REJECTED;
        printf("\n[SUCCESS] Loan %d rejected.\n", loanId);
        snprintf(buffer, sizeof(buffer), "Rejected loan %d for account %d",
                 loanId, loan->accountNumber);
    }

    saveLoans();
    logAdminAction(buffer);
}

void loanManagement()
{
    int choice;

    while (1)
    {
        printf("\n--- Loan Management ---\n");
        printf("1. Apply for a Loan (for a customer)\n");
        printf("2. View All Loan Applications\n");
        printf("3. Approve a Loan\n");
        printf("4. Reject a Loan\n");
        printf("0. Back\n");

        choice = readInt("\nEnter your choice: ");

        switch (choice)
        {
        case 1:
        {
            int accountNumber = readInt("Enter Customer Account Number: ");
            Account *account = findAccount(accountNumber);
            if (account == NULL)
            {
                printf("\n[ERROR] Account not found. Loan application cancelled.\n");
            }
            else
            {
                applyForLoan(account);
            }
            break;
        }
        case 2:
            displayLoans(0, 0);
            break;
        case 3:
            processLoan(1);
            break;
        case 4:
            processLoan(0);
            break;
        case 0:
            return;
        default:
            printf("\n[ERROR] Invalid choice! Please try again.\n");
        }
    }
}


// COMPLAINTS
void submitComplaint(int accountNumber)
{
    Complaint *newComplaint = malloc(sizeof(Complaint));

    if (newComplaint == NULL)
    {
        printf("\n[ERROR] Memory allocation failed!\n");
        return;
    }

    newComplaint->complaintId = nextComplaintId++;
    newComplaint->accountNumber = accountNumber;
    readText("Describe the Complaint: ", newComplaint->text, 101);
    newComplaint->status = COMPLAINT_PENDING;
    newComplaint->nxt = NULL;

    if (complaintHead == NULL)
    {
        complaintHead = newComplaint;
    }
    else
    {
        Complaint *temp = complaintHead;
        while (temp->nxt != NULL)
        {
            temp = temp->nxt;
        }
        temp->nxt = newComplaint;
    }

    saveComplaints();
    saveCounters();
    printf("\n[SUCCESS] Complaint submitted. Complaint ID: %d\n", newComplaint->complaintId);
}

// accountNumber = 0 shows every complaint, otherwise only that account's
void displayComplaints(int accountNumber)
{
    Complaint *temp = complaintHead;
    int count = 0;

    while (temp != NULL)
    {
        if (accountNumber != 0 && temp->accountNumber != accountNumber)
        {
            temp = temp->nxt;
            continue;
        }
        if (count == 0)
        {
            printf("\n%-6s %-12s %-12s %s\n", "ID", "Account No.", "Status", "Complaint");
        }
        printf("%-6d %-12d %-12s %s\n", temp->complaintId, temp->accountNumber,
               complaintStatusText(temp->status), temp->text);
        count++;
        temp = temp->nxt;
    }
    if (count == 0)
    {
        printf("\n[INFO] No complaints found.\n");
    }
}

// newStatus: COMPLAINT_IN_PROGRESS or COMPLAINT_RESOLVED
void changeComplaintStatus(int newStatus)
{
    int complaintId = readInt("Enter Complaint ID: ");
    Complaint *temp = complaintHead;
    char buffer[100];

    while (temp != NULL)
    {
        if (temp->complaintId == complaintId)
        {
            break;
        }
        temp = temp->nxt;
    }

    if (temp == NULL)
    {
        printf("\n[ERROR] Complaint not found.\n");
        return;
    }
    if (temp->status == COMPLAINT_RESOLVED)
    {
        printf("\n[INFO] This complaint is already resolved.\n");
        return;
    }

    temp->status = newStatus;
    saveComplaints();
    printf("\n[SUCCESS] Complaint %d is now: %s\n", complaintId, complaintStatusText(newStatus));
    snprintf(buffer, sizeof(buffer), "Complaint %d marked as %s",
             complaintId, complaintStatusText(newStatus));
    logAdminAction(buffer);
}

void handleComplaints()
{
    int choice;

    while (1)
    {
        printf("\n--- Handle Complaints ---\n");
        printf("1. Submit a Complaint (for a customer)\n");
        printf("2. View All Complaints\n");
        printf("3. Search Complaints by Account Number\n");
        printf("4. Mark Complaint as In Progress\n");
        printf("5. Resolve a Complaint\n");
        printf("0. Back\n");

        choice = readInt("\nEnter your choice: ");

        switch (choice)
        {
        case 1:
        {
            int accountNumber = readInt("Enter Customer Account Number: ");
            if (findAccount(accountNumber) == NULL)
            {
                printf("\n[ERROR] Account not found. Complaint cancelled.\n");
            }
            else
            {
                submitComplaint(accountNumber);
            }
            break;
        }
        case 2:
            displayComplaints(0);
            break;
        case 3:
        {
            int accountNumber = readInt("Enter Customer Account Number: ");
            if (accountNumber == 0)
            {
                printf("\n[ERROR] Invalid account number.\n");
            }
            else
            {
                displayComplaints(accountNumber);
            }
            break;
        }
        case 4:
            changeComplaintStatus(COMPLAINT_IN_PROGRESS);
            break;
        case 5:
            changeComplaintStatus(COMPLAINT_RESOLVED);
            break;
        case 0:
            return;
        default:
            printf("\n[ERROR] Invalid choice! Please try again.\n");
        }
    }
}


// WAITING QUEUE
void manageQueue()
{
    int choice;

    while (1)
    {
        printf("\n--- Manage Waiting Queue ---\n");
        printf("1. Add a Customer to the Queue\n");
        printf("2. Serve the Next Customer\n");
        printf("3. Display the Waiting Queue\n");
        printf("0. Back\n");

        choice = readInt("\nEnter your choice: ");

        switch (choice)
        {
        case 1:
        {
            Customer customer;

            readText("Enter Customer Name: ", customer.name, 51);
            customer.accountNumber = readInt("Enter Account Number (0 if none): ");
            customer.queueNumber = nextQueueNumber;

            if (enqueueCustomer(customer))
            {
                nextQueueNumber++;
                printf("\n[SUCCESS] Customer added successfully! Queue Number: %d\n",
                       customer.queueNumber);
            }
            break;
        }
        case 2:
        {
            Customer customer;

            if (dequeueCustomer(&customer))
            {
                printf("\n[SUCCESS] Serving customer: %s | Queue #%d", customer.name, customer.queueNumber);
                if (customer.accountNumber != 0)
                {
                    printf(" | Account #%d", customer.accountNumber);
                }
                printf("\n");
            }
            break;
        }
        case 3:
            displayQueue();
            break;
        case 0:
            return;
        default:
            printf("\n[ERROR] Invalid choice! Please try again.\n");
        }
    }
}


// REPORTS
void dashboardOverview()
{
    int totalCustomers = 0, activeAccounts = 0, blockedAccounts = 0;
    int pendingRegistrations = 0, pendingLoans = 0, totalComplaints = 0;
    double totalBalance = 0;
    Account *account;
    Registration *reg;
    Loan *loan;
    Complaint *complaint;

    for (account = accountHead; account != NULL; account = account->nxt)
    {
        totalCustomers++;
        totalBalance += account->balance;
        if (account->isBlocked)
            blockedAccounts++;
        else
            activeAccounts++;
    }
    for (reg = registrationHead; reg != NULL; reg = reg->nxt)
    {
        if (reg->status == STATUS_PENDING)
            pendingRegistrations++;
    }
    for (loan = loanHead; loan != NULL; loan = loan->nxt)
    {
        if (loan->status == STATUS_PENDING)
            pendingLoans++;
    }
    for (complaint = complaintHead; complaint != NULL; complaint = complaint->nxt)
    {
        totalComplaints++;
    }

    printf("\n==================== DASHBOARD OVERVIEW ====================\n");
    printf("  Total registered customers    : %d\n", totalCustomers);
    printf("  Total active accounts         : %d\n", activeAccounts);
    printf("  Total blocked accounts        : %d\n", blockedAccounts);
    printf("  Pending registration requests : %d\n", pendingRegistrations);
    printf("  Total transactions            : %d\n", txStack.totalCount);
    printf("  Pending loan applications     : %d\n", pendingLoans);
    printf("  Total complaints              : %d\n", totalComplaints);
    printf("  Total bank balance            : %.2f\n", totalBalance);
    printf("============================================================\n");
}

void generateReports()
{
    int customers = 0, active = 0, blocked = 0;
    int loansApproved = 0, loansRejected = 0, loansPending = 0;
    int complaintsPending = 0, complaintsInProgress = 0, complaintsResolved = 0;
    double totalBalance = 0;
    Account *account;
    Loan *loan;
    Complaint *complaint;

    for (account = accountHead; account != NULL; account = account->nxt)
    {
        customers++;
        totalBalance += account->balance;
        if (account->isBlocked)
            blocked++;
        else
            active++;
    }
    for (loan = loanHead; loan != NULL; loan = loan->nxt)
    {
        if (loan->status == STATUS_APPROVED)
            loansApproved++;
        else if (loan->status == STATUS_REJECTED)
            loansRejected++;
        else
            loansPending++;
    }
    for (complaint = complaintHead; complaint != NULL; complaint = complaint->nxt)
    {
        if (complaint->status == COMPLAINT_PENDING)
            complaintsPending++;
        else if (complaint->status == COMPLAINT_IN_PROGRESS)
            complaintsInProgress++;
        else
            complaintsResolved++;
    }

    printf("\n==================================================\n");
    printf("                 BANK SUMMARY REPORT\n");
    printf("==================================================\n");
    printf(" CUSTOMERS\n");
    printf("   Total customers        : %d\n", customers);
    printf("   Active accounts        : %d\n", active);
    printf("   Blocked accounts       : %d\n", blocked);
    printf("--------------------------------------------------\n");
    printf(" TRANSACTIONS\n");
    printf("   Total deposits         : %.2f (%d transactions)\n",
           txStack.totalDeposits, txStack.depositCount);
    printf("   Total withdrawals      : %.2f (%d transactions)\n",
           txStack.totalWithdrawals, txStack.withdrawalCount);
    printf("   Total transfers        : %.2f (%d transactions)\n",
           txStack.totalTransfers, txStack.transferCount);
    printf("   Total bank balance     : %.2f\n", totalBalance);
    printf("--------------------------------------------------\n");
    printf(" LOANS\n");
    printf("   Approved loans         : %d\n", loansApproved);
    printf("   Rejected loans         : %d\n", loansRejected);
    printf("   Pending loans          : %d\n", loansPending);
    printf("--------------------------------------------------\n");
    printf(" COMPLAINTS\n");
    printf("   Pending complaints     : %d\n", complaintsPending);
    printf("   In progress            : %d\n", complaintsInProgress);
    printf("   Resolved               : %d\n", complaintsResolved);
    printf("==================================================\n");
}


// DELETE INACTIVE USERS
// An account is "inactive" when it has never had any transaction.
int isInactive(const Account *account)
{
    return account->transactionCount == 0;
}

int displayInactiveAccounts()
{
    Account *temp = accountHead;
    int count = 0;

    while (temp != NULL)
    {
        if (isInactive(temp))
        {
            if (count == 0)
            {
                printf("\nInactive accounts (no transactions so far):\n");
                printf("%-12s %-22s %-10s %-14s %-9s\n",
                       "Account No.", "Holder Name", "Type", "Balance", "Status");
            }
            printf("%-12d %-22s %-10s %-14.2f %-9s\n", temp->accountNumber,
                   temp->holderName, temp->accountType, temp->balance,
                   accountStatusText(temp));
            count++;
        }
        temp = temp->nxt;
    }
    if (count == 0)
    {
        printf("\n[INFO] There are no inactive accounts.\n");
    }
    return count;
}

void deleteInactiveUsers()
{
    char buffer[100];

    printf("\n--- Delete Inactive Users ---\n");

    if (displayInactiveAccounts() == 0)
    {
        return;
    }

    int accountNumber = readInt("\nEnter Account Number to delete (0 to cancel): ");
    if (accountNumber == 0)
    {
        printf("\n[INFO] Cancelled.\n");
        return;
    }

    Account *account = findAccount(accountNumber);
    if (account == NULL)
    {
        printf("\n[ERROR] Account not found.\n");
        return;
    }
    if (!isInactive(account)) // safety check: never delete active users here
    {
        printf("\n[ERROR] This account is ACTIVE and cannot be deleted here.\n");
        return;
    }
    if (account->balance > 0)
    {
        printf("\n[INFO] Warning: this account still holds a balance of %.2f.\n", account->balance);
    }
    if (!confirmAction("Delete this inactive account?"))
    {
        printf("\n[INFO] Deletion cancelled.\n");
        return;
    }

    removeAccount(accountNumber);
    saveAccounts();
    printf("\n[SUCCESS] Inactive account deleted.\n");
    snprintf(buffer, sizeof(buffer), "Deleted inactive account %d", accountNumber);
    logAdminAction(buffer);
}


// CUSTOMER PANEL
void changePassword(Account *account)
{
    char oldPassword[31];
    char newPassword[31];
    char again[31];

    readText("Enter Current Password: ", oldPassword, 31);
    if (strcmp(oldPassword, account->password) != 0)
    {
        printf("\n[ERROR] Current password is incorrect.\n");
        return;
    }

    readText("Enter New Password: ", newPassword, 31);
    readText("Re-enter New Password: ", again, 31);

    if (strcmp(newPassword, again) != 0)
    {
        printf("\n[ERROR] Passwords do not match. Password not changed.\n");
        return;
    }

    strcpy(account->password, newPassword);
    saveAccounts();
    printf("\n[SUCCESS] Password changed successfully!\n");
}

void customerPanel(Account *account)
{
    int choice;
    char buffer[100];

    while (1)
    {
        printf("\n========== CUSTOMER PANEL ==========\n");

        printf("\n1. View My Profile\n");

        printf("\n---------- BANKING ----------\n");
        printf("2. Deposit Money\n");
        printf("3. Withdraw Money\n");
        printf("4. Transfer Money\n");
        printf("5. View My Transaction History\n");

        printf("\n---------- LOANS & COMPLAINTS ----------\n");
        printf("6. Apply for a Loan\n");
        printf("7. View My Loans\n");
        printf("8. Submit a Complaint\n");
        printf("9. View My Complaints\n");

        printf("\n---------- SECURITY ----------\n");
        printf("10. Change Password\n");

        printf("\n0. Logout\n");

        choice = readInt("\nEnter your choice: ");

        switch (choice)
        {
        case 1:
            printf("\n--- My Profile ---\n");
            displayAccountDetails(account);
            break;

        case 2:
            performDeposit(account, readPositiveAmount("Enter Deposit Amount: "));
            break;

        case 3:
            performWithdraw(account, readPositiveAmount("Enter Withdrawal Amount: "));
            break;

        case 4:
        {
            int receiverNumber = readInt("Enter Receiver Account Number: ");
            Account *receiver = findAccount(receiverNumber);

            if (receiverNumber == account->accountNumber)
            {
                printf("\n[ERROR] You cannot transfer to your own account.\n");
            }
            else if (receiver == NULL)
            {
                printf("\n[ERROR] Receiver account not found. Transfer cancelled.\n");
            }
            else
            {
                performTransfer(account, receiver, readPositiveAmount("Enter Transfer Amount: "));
            }
            break;
        }

        case 5:
            printf("\n--- My Transaction History (newest first) ---\n");
            showTransactions(NULL, account->accountNumber);
            break;

        case 6:
            applyForLoan(account);
            snprintf(buffer, sizeof(buffer), "Customer %d applied for a loan", account->accountNumber);
            addActivity(buffer);
            break;

        case 7:
            printf("\n--- My Loans ---\n");
            displayLoans(0, account->accountNumber);
            break;

        case 8:
            submitComplaint(account->accountNumber);
            snprintf(buffer, sizeof(buffer), "Customer %d submitted a complaint", account->accountNumber);
            addActivity(buffer);
            break;

        case 9:
            printf("\n--- My Complaints ---\n");
            displayComplaints(account->accountNumber);
            break;

        case 10:
            changePassword(account);
            break;

        case 0:
            printf("\nLogging out from Customer Panel...\n");
            return;

        default:
            printf("\n[ERROR] Invalid choice! Please try again.\n");
        }
    }
}


// ACCOUNT LIST HELPERS
Account *findAccount(int accountNumber)
{
    Account *temp = accountHead;

    while (temp != NULL)
    {
        if (temp->accountNumber == accountNumber)
        {
            return temp;
        }
        temp = temp->nxt;
    }
    return NULL;
}

void appendAccountNode(Account *node)
{
    node->nxt = NULL;

    if (accountHead == NULL)
    {
        accountHead = node;
    }
    else
    {
        Account *temp = accountHead;
        while (temp->nxt != NULL)
        {
            temp = temp->nxt;
        }
        temp->nxt = node;
    }
}

// Creates a new account and adds it at the end of the list (numbers are never reused)
Account *addAccountToList(const char *name, const char *type, double balance, const char *password)
{
    Account *newAccount = malloc(sizeof(Account));

    if (newAccount == NULL)
    {
        return NULL;
    }

    newAccount->accountNumber = nextAccountNumber++;
    strcpy(newAccount->holderName, name);
    strcpy(newAccount->accountType, type);
    newAccount->balance = balance;
    strcpy(newAccount->password, password);
    newAccount->isBlocked = 0;
    newAccount->transactionCount = 0;

    appendAccountNode(newAccount);
    return newAccount;
}

// Removes one account from the list and frees its memory. Returns 1 on success.
int removeAccount(int accountNumber)
{
    Account *temp = accountHead;
    Account *prev = NULL;

    while (temp != NULL && temp->accountNumber != accountNumber)
    {
        prev = temp;
        temp = temp->nxt;
    }
    if (temp == NULL)
    {
        return 0;
    }
    if (prev == NULL)
    {
        accountHead = temp->nxt;
    }
    else
    {
        prev->nxt = temp->nxt;
    }
    free(temp);
    return 1;
}


// COMMON FUNCTIONS
const char *statusText(int status)
{
    if (status == STATUS_APPROVED)
        return "Approved";
    if (status == STATUS_REJECTED)
        return "Rejected";
    return "Pending";
}

const char *complaintStatusText(int status)
{
    if (status == COMPLAINT_RESOLVED)
        return "Resolved";
    if (status == COMPLAINT_IN_PROGRESS)
        return "In Progress";
    return "Pending";
}

const char *accountStatusText(const Account *account)
{
    return account->isBlocked ? "Blocked" : "Active";
}

// Password is never displayed
void displayAccountDetails(Account *account)
{
    printf("Account Number: %d\n", account->accountNumber);
    printf("Holder Name: %s\n", account->holderName);
    printf("Account Type: %s\n", account->accountType);
    printf("Balance: %.2f\n", account->balance);
    printf("Status: %s\n", accountStatusText(account));
    printf("Transactions: %d\n", account->transactionCount);

    printf("=====================================\n");
}


// Release Heap Memory of all (Singly) Linked Lists
void freeAdmins()
{
    Admin *temp;
    while (adminHead != NULL)
    {
        temp = adminHead;
        adminHead = adminHead->nxt;
        free(temp);
    }
    printf("\n[SUCCESS] Admin memory released successfully!\n");
}

void freeAccounts()
{
    Account *temp;
    while (accountHead != NULL)
    {
        temp = accountHead;
        accountHead = accountHead->nxt;
        free(temp);
    }
    printf("\n[SUCCESS] Account memory released successfully!\n");
}

void freeRegistrations()
{
    Registration *temp;
    while (registrationHead != NULL)
    {
        temp = registrationHead;
        registrationHead = registrationHead->nxt;
        free(temp);
    }
    printf("\n[SUCCESS] Registration memory released successfully!\n");
}

void freeLoans()
{
    Loan *temp;
    while (loanHead != NULL)
    {
        temp = loanHead;
        loanHead = loanHead->nxt;
        free(temp);
    }
    printf("\n[SUCCESS] Loan memory released successfully!\n");
}

void freeComplaints()
{
    Complaint *temp;
    while (complaintHead != NULL)
    {
        temp = complaintHead;
        complaintHead = complaintHead->nxt;
        free(temp);
    }
    printf("\n[SUCCESS] Complaint memory released successfully!\n");
}

void freeActivities()
{
    Activity *temp;
    while (activityHead != NULL)
    {
        temp = activityHead;
        activityHead = activityHead->nxt;
        free(temp);
    }
    printf("\n[SUCCESS] Activity log memory released successfully!\n");
}
