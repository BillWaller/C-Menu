/** @file iloan.c
    @brief Installment Loan Calculator
    @author Bill Waller
    Copyright (c) 2025
    MIT License
    billxwaller@gmail.com
    @date 2026-02-09
 */

#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FALSE 0
#define TRUE 1
char in_str[BUFSIZ + 1];

void numbers(char *d, char *s);
double calculate_i(double, double, double);
double calculate_n(double, double, double);
double calculate_pmt(double, double, double);
double calculate_pv(double, double, double);
int is_numeric(char *);
void accept_str(char *s);
char *format_currency(float);
char *format_interest(float);
double accept_pv();
double accept_n();
double accept_i();
double accept_pmt();
void error_press_any_key(char *);
void ABEND(int);
int f_pv = 0;
int f_n = 0;
int f_i = 0;
int f_pmt = 0;
bool f_quiet = false;

/** @brief iloan is a trivial application to demonstrate how a command-line
  program can be integrated into C-Menu Form with simple file, argument, or pipe
  i-o.
  @details Positional arguments:
        present_value The present value of the loan (the amount borrowed).
        number_of_payments The total number of payments to be made.
        interest_rate The annual interest rate (as a percentage).
        payment_amount The amount of each payment.
  The program calculates the missing value based on the three provided
  values. For example, if the present value, number of payments, and interest
  rate are provided, it will calculate the payment amount. If the present value,
  interest rate, and payment amount are provided, it will calculate the number
  of payments, and so on.
  The program can be used in a non-interactive way by passing the four
  values as arguments, with the value to be calculated set to 0. For example, to
  calculate the payment amount for a $10,000 loan with a 5% annual interest rate
  and 60 monthly payments, you could run:
  @code
    iloan 10000 60 5 0
  @endcode
  This is proof-of-concept code, and is not intended for production use.
  It is not designed to be robust, and does not handle all edge cases or input
  errors. It is intended solely to demonstrate how a simple command-line program
  can be integrated into C-Menu Form.
  In the future, more sophisticated abstractions, such as async event
  handlers, serialization, rpc, database, and a standardized plugin interface
  will be integrated into C-Menu.
  Feel free to modify and enhance this code as needed, but please do not
  use it in production without proper testing and validation. It is provided
  as-is without any warranty or support. Use at your own risk.
*/
int main(int argc, char **argv) {
    double pv = 0, pmt = 0, i = 0, n = 0;
    char tmp_str[BUFSIZ];

    signal(SIGINT, ABEND);
    signal(SIGQUIT, ABEND);
    signal(SIGHUP, ABEND);

    if (argc > 1 &&
        ((strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) ||
         argc < 4)) {
        printf("Usage: iloan [present_value number_of_payments "
               "interest_rate payment_amount]\n\n");
        exit(EXIT_SUCCESS);
    }
    if (argc > 4) {
        numbers(tmp_str, argv[1]);
        sscanf(tmp_str, "%lf", &pv);
        sscanf(argv[2], "%lf", &n);
        sscanf(argv[3], "%lf", &i);
        numbers(tmp_str, argv[4]);
        sscanf(tmp_str, "%lf", &pmt);
        if (pv != 0)
            f_pv = 1;
        if (n != 0)
            f_n = 1;
        if (i != 0)
            f_i = 1;
        if (pmt != 0)
            f_pmt = 1;
        if (f_pv + f_n + f_i + f_pmt < 3) {
            error_press_any_key(
                "Error: At least three values must be greater than zero.");
            exit(EXIT_FAILURE);
        }
        f_quiet = true;
    } else {
        if (argc != 1) {
            printf("Usage: iloan [present_value number_of_payments "
                   "interest_rate payment_amount]\n\n");
            exit(EXIT_FAILURE);
        } else {
            if (argc == 1) {
                printf("\nInstallment Loan Calculator\n\n");
                printf("Three of these values must be greater than 0.  The "
                       "field\n");
                printf("with a value of 0 will be calculated.\n\n");
                while (f_pv + f_n + f_i + f_pmt < 3) {
                    if (pv == 0) {
                        pv = accept_pv();
                        if (pv != 0)
                            f_pv = 1;
                    }
                    if (n == 0) {
                        n = accept_n();
                        if (n != 0)
                            f_n = 1;
                    }
                    if (i == 0) {
                        i = accept_i();
                        if (i != 0)
                            f_i = 1;
                    }
                    if (pmt == 0) {
                        pmt = accept_pmt();
                        if (pmt != 0)
                            f_pmt = 1;
                    }
                    if (f_pv + f_n + f_i + f_pmt < 3) {
                        error_press_any_key("Error: At least three values must "
                                            "be greater than zero.");
                    }
                }
                printf("\nYou entered:\n\n");
                if (pv != 0)
                    printf("Present Value - - - - - -> %s\n",
                           format_currency(pv));
                if (n != 0)
                    printf("Number of Payments  - - -> %s\n",
                           format_currency(n));
                if (i != 0)
                    printf("Interest Rate - - - - - -> %s\n",
                           format_interest(i));
                if (pmt != 0)
                    printf("Payment Amount  - - - - -> %s\n",
                           format_currency(pmt));
                printf("\n\nCalculation result:\n\n");
            }
        }
    }
    if (pv == 0)
        pv = calculate_pv(n, i, pmt);
    else if (n == 0)
        n = calculate_n(pv, i, pmt);
    else if (i == 0)
        i = calculate_i(pv, n, pmt);
    else if (pmt == 0)
        pmt = calculate_pmt(pv, n, i);

    if (f_quiet) {
        printf("%s\n", format_currency(pv));
        printf("%s\n", format_currency(n));
        printf("%s\n", format_interest(i));
        printf("%s\n", format_currency(pmt));
    }
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
}
/** @brief The following functions are used to accept user input for the present
  value, number of payments, interest rate, and payment amount. They validate
  the input to ensure it is numeric and meets the required conditions (e.g., non-negative).
  If the input is invalid, an error message is displayed, and the user is prompted to enter the value again.
*/
double accept_pv() {
    double pv;
    while (1) {
        accept_str("Present Value - - -> ");
        if (in_str[0] == '\0') {
            pv = 0;
            break;
        } else {
            if (is_numeric(in_str)) {
                pv = atof(in_str);
                if (pv < 0)
                    error_press_any_key("Present Value can't be less than 0");
                else
                    break;
            } else
                error_press_any_key("Present Value must be numeric");
        }
    }
    return (pv);
}
/** @brief The accept_n function prompts the user to enter the number of payments for the loan. It validates the input to ensure it is numeric and non-negative. If the input is invalid, an error message is displayed, and the user is prompted to enter the value again until a valid input is provided.
 */
double accept_n() {
    double n;
    while (1) {
        accept_str("Number of Payments > ");
        if (is_numeric(in_str)) {
            n = atof(in_str);
            if (n < 0)
                error_press_any_key("Number of Payments can't be less than 0");
            else
                break;
        } else
            error_press_any_key("Number of Payments must be numeric");
    }
    return (n);
}
/** @brief The accept_i function prompts the user to enter the annual interest rate for the loan. It validates the input to ensure it is numeric and non-negative. If the input is invalid, an error message is displayed, and the user is prompted to enter the value again until a valid input is provided.
 */
double accept_i() {
    double i;
    while (1) {
        accept_str("Rate (annual) - - -> ");
        if (in_str[0] == '\0') {
            i = 0;
            break;
        } else {
            if (is_numeric(in_str)) {
                i = atof(in_str);
                if (i < 0)
                    error_press_any_key("interest Rate can't be less than 0");
                else
                    break;
            } else
                error_press_any_key("Interest Rate must be numeric");
        }
    }
    return (i);
}
/** brief The accept_pmt function prompts the user to enter the payment amount for the loan. It validates the input to ensure it is numeric and non-negative. If the input is invalid, an error message is displayed, and the user is prompted to enter the value again until a valid input is provided.
 */
double accept_pmt() {
    double pmt;
    while (1) {
        accept_str("Payment Amount  - -> ");
        if (in_str[0] == '\0') {
            pmt = 0;
            break;
        } else {
            if (is_numeric(in_str)) {
                pmt = atof(in_str);
                break;
            } else
                error_press_any_key("Payment Amount must be numeric");
        }
    }
    return (pmt);
}
/** @brief The error_press_any_key function is a utility function that displays an error message to the user and waits for them to press any key before continuing. It takes a string argument that contains the error message to be displayed. After the user presses a key, it prints two newlines for formatting.
 */
void error_press_any_key(char *s) {
    printf("%s", s);
    getc(stdin);
    printf("\n\n");
}
/** @brief The calculate_pv function calculates the present value of a loan based on the number of payments, interest rate, and payment amount. It uses the formula for the present value of an annuity to compute the result. If any of the input values are zero, it displays an error message and prompts the user to provide valid inputs.
 */
double calculate_pv(double n, double i, double pmt) {
    double i1, pv;

    if (n == 0 || i == 0 || pmt == 0)
        error_press_any_key(
            "3 non-zero values required to calculate Present Value");
    i1 = i / 1200;
    pv = pmt * (1 - pow(1 + i1, -n)) / i1;
    if (!f_quiet)
        printf("Present Value - - - - - -> %s\n", format_currency(pv));
    return (pv);
}
/** @brief The calculate_n function calculates the number of payments required to pay off a loan based on the present value, interest rate, and payment amount. It uses logarithmic functions to compute the result. If any of the input values are zero, it displays an error message and prompts the user to provide valid inputs.
 */
double calculate_n(double pv, double i, double pmt) {
    double i1, n;
    if (pv == 0 || i == 0 || pmt == 0)
        error_press_any_key("3 non-zero values required to calculate "
                            "Number of Payments");
    i1 = i / 1200;
    n = -log(1 - pv * i1 / pmt) / log(1 + i1);
    if (!f_quiet)
        printf("Number of Payments  - - -> %s\n", format_currency(n));
    return (n);
}
/** @brief The calculate_i function calculates the annual interest rate for a loan based on the present value, number of payments, and payment amount. It uses an iterative method to find the interest rate that satisfies the loan equation. If any of the input values are zero, it displays an error message and prompts the user to provide valid inputs.
 */
double calculate_i(double pv, double n, double pmt) {
    double i1 = 0, i;
    double delta = 0.0000001;
    double xdelta;
    double fdelta;
    double fprimi;
    double ffact;
    double fi;
    double fman;
    double fmann;
    if (pv == 0 || n == 0 || pmt == 0)
        error_press_any_key(
            "3 non-zero values required to calculate Interest Rate");
    ffact = pv / pmt;
    xdelta = 0;
    if (ffact < n) {
        i1 = 0.0125;
        xdelta = 0.9;
    }
    while (xdelta > delta) {
        fman = 1 / (1 + i1);
        fmann = pow(fman, n);
        fi = ffact * i1 + fmann - 1;
        fprimi = ffact - n * fmann * fman;
        fdelta = fi / fprimi;
        i1 = i1 - fdelta;
        if (fdelta < 0)
            xdelta = -fdelta;
        else
            xdelta = fdelta;
    }
    i = i1 * 1200;
    if (!f_quiet)
        printf("interest Rate - - - - - -> %s\n", format_interest(i));
    return (i);
}
/** @brief The calculate_pmt function calculates the payment amount for a loan based on the present value, number of payments, and interest rate. It uses the formula for the payment amount of an annuity to compute the result. If any of the input values are zero, it displays an error message and prompts the user to provide valid inputs.
 */
double calculate_pmt(double pv, double n, double i) {
    double i1, pmt;
    if (pv == 0 || n == 0 || i == 0)
        error_press_any_key(
            "3 non-zero values required to calculate Payment Amount");
    i1 = i / 1200;
    pmt = pv * i1 / (1 - pow(1 + i1, -n));
    if (!f_quiet)
        printf("Payment Amount  - - - - -> %s\n", format_currency(pmt));
    return (pmt);
}
/** @brief The is_numeric function checks if a given string consists of numeric characters, including digits, decimal points, and commas. It iterates through each character in the string and returns FALSE if it encounters any character that is not a digit, a decimal point, or a comma. If all characters are valid, it returns TRUE.
 */
int is_numeric(char *s) {
    char c;

    while ((c = *s++) != '\0' && c != '\n')
        if ((c < '0' || c > '9') && c != '.' && c != ',')
            return (FALSE);
    return (TRUE);
}
/** @brief The accept_str function prompts the user with a given string and reads input from the standard input into a buffer. It uses the fprintf function to display the prompt and the read function to capture the user's input. The input is stored in the global buffer in_str, which can be used by other functions for further processing.
 */
void accept_str(char *s) {
    fprintf(stderr, "%s", s);
    read(0, in_str, BUFSIZ);
}
/** @brief The format_currency function takes a floating-point number as input and formats it as a currency string. It adds commas as thousand separators and ensures that the number is displayed with two decimal places. The function uses a static buffer to store the formatted string and returns a pointer to that buffer. The formatted string is right-aligned and padded with spaces if necessary.
 */
char *format_currency(float a) {
    int digit_count, left_of_point;
    static char sstr[80];
    static char fstr[80];
    char *src_ptr, *dst_ptr, *sptr, *FPtr;

    sprintf(sstr, "%-18.2f", a);
    sptr = sstr;
    src_ptr = sstr;
    FPtr = fstr;
    dst_ptr = fstr;
    while (*src_ptr++ != '\0')
        ;
    src_ptr -= 2;
    left_of_point = FALSE;
    digit_count = 0;
    while (src_ptr >= sptr) {
        if (left_of_point) {
            if (*src_ptr >= '0' && *src_ptr <= '9') {
                if (digit_count == 3) {
                    *dst_ptr++ = ',';
                    digit_count = 1;
                } else
                    digit_count++;
            }
        } else if (*src_ptr == '.')
            left_of_point = TRUE;
        *dst_ptr++ = *src_ptr--;
    }
    src_ptr = dst_ptr - 1;
    dst_ptr = sptr;
    while (src_ptr >= FPtr)
        *dst_ptr++ = *src_ptr--;
    while (*--dst_ptr == ' ')
        ;
    *++dst_ptr = '\0';
    return (sptr);
}
/** @brief The format_interest function takes a floating-point number representing an interest rate and formats it as a string with five decimal places. It uses a static buffer to store the formatted string and returns a pointer to that buffer. The formatted string is left-aligned and padded with spaces if necessary.
 */
char *format_interest(float a) {
    static char sstr[80];
    char *s;

    sprintf(sstr, "%-3.5f", a);
    s = sstr;
    return (s);
}
/** @brief The ABEND function is a signal handler that is called when the program receives certain signals (e.g., SIGINT, SIGQUIT, SIGHUP). It takes an integer argument representing the signal number and prints an error message indicating that an abnormal end (ABEND) has occurred, along with the signal number. After displaying the message, it exits the program with a failure status.
 */
void ABEND(int e) {
    printf("ABEND: Error %d:\n", e);
    exit(EXIT_FAILURE);
}
/** @brief The numbers function takes two character pointers as arguments: a destination pointer (d) and a source pointer (s). It iterates through the characters in the source string (s) and copies only the numeric characters (digits, decimal points, and minus signs) to the destination string (d). The function effectively filters out any non-numeric characters from the source string and constructs a new string containing only the valid numeric characters. Finally, it null-terminates the destination string.
 */
void numbers(char *d, char *s) {
    while (*s != '\0') {
        if (*s == '-' || *s == '.' || (*s >= '0' && *s <= '9'))
            *d++ = *s++;
        else
            s++;
    }
    *d = '\0';
}
