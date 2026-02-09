/// iloan.c
/// Calculate installment loan values
//  Bill Waller Copyright (c) 2025
//  MIT License
//  billxwaller@gmail.com
///
/// iloan is a trivial application to demonstrate how a command-line
/// program can be integrated into C-Menu Form with simple file,
/// argument, or pipe i-o.
///
/// The program can be used in a non-interactive way by passing the four
/// values as arguments, with the value to be calculated set to 0.
/// For example, to calculate the payment amount for a $10,000 loan with
/// a 5% annual interest rate and 60 monthly payments, you could run:
///
/// iloan 10000 60 5 0
///
/// This is proof-of-concept code, and is not intended for production use.
/// It is not designed to be robust, and does not handle all edge cases or
/// input errors. It is intended solely to demonstrate how a simple
/// command-line program can be integrated into C-Menu Form.
///
/// In the future, more sophisticated abstractions, such as async event
/// handlers, serialization, rpc, database, and a standardized plugin
/// interface will be integrated into C-Menu.
///
/// Feel free to modify and enhance this code as needed, but please do not use
/// it in production without proper testing and validation. It is provided as-is
/// without any warranty or support. Use at your own risk.

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

void error_press_any_key(char *s) {
    printf("%s", s);
    getc(stdin);
    printf("\n\n");
}

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

int is_numeric(char *s) {
    char c;

    while ((c = *s++) != '\0' && c != '\n')
        if ((c < '0' || c > '9') && c != '.' && c != ',')
            return (FALSE);
    return (TRUE);
}

void accept_str(char *s) {
    fprintf(stderr, "%s", s);
    read(0, in_str, BUFSIZ);
}

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

char *format_interest(float a) {
    static char sstr[80];
    char *s;

    sprintf(sstr, "%-3.5f", a);
    s = sstr;
    return (s);
}

void ABEND(int e) {
    printf("ABEND: Error %d:\n", e);
    exit(EXIT_FAILURE);
}

void numbers(char *d, char *s) {
    while (*s != '\0') {
        if (*s == '-' || *s == '.' || (*s >= '0' && *s <= '9'))
            *d++ = *s++;
        else
            s++;
    }
    *d = '\0';
}
