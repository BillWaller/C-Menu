/* iloan.c
 * by Bill Waller
 * calculate installment loan values
 * Bill Waller
 */

#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FALSE 0
#define TRUE 1

char in_str[BUFSIZ + 1];

double calculate_i(double, double, double);
double calculate_n(double, double, double);
double calculate_pmt(double, double, double);
double calculate_pv(double, double, double);
int is_numeric(char *);
void accept_str(char *s);
char *format_currency(float);
char *format_interest(float);
double accept_pv();
double accept_n(double);
double accept_i(double, double);
double accept_pmt(double, double, double);
void error_press_any_key(char *);
void ABEND(int);

int main() {
    double pv, pmt, i, n;

    signal(SIGINT, ABEND);
    signal(SIGQUIT, ABEND);
    signal(SIGHUP, ABEND);
    printf("\nInstallment Loan Calculator\n\n");
    printf("You will be prompted to enter:\n\n");
    printf("    Present Value\n");
    printf("    Payment Amount\n");
    printf("    Number of Payments\n");
    printf("    Interest Rate\n\n");
    printf("Three of these values must be greater than 0.  The field\n");
    printf("with a value of 0 will be calculated.\n\n");

    while (1) {
        pmt = 0;
        if (!(pv = accept_pv()))
            break;
        if (!(n = accept_n(pv)))
            break;
        if (!(i = accept_i(pv, n)))
            break;
        if (pv == 0 || n == 0 || i == 0)
            if (!(pmt = accept_pmt(pv, n, i)))
                break;
        if (pv == 0) {
            pv = calculate_pv(n, i, pmt);
            break;
        }
        // if (pmt == 0) {
        //     pmt = calculate_pmt(pv, n, i);
        //     break;
        // }
        if (n == 0) {
            n = calculate_n(pv, i, pmt);
            break;
        }
        if (i == 0) {
            if ((pmt * n) >= pv) {
                i = calculate_i(pv, n, pmt);
                break;
            } else
                error_press_any_key("(Payment Amount * Number of Payments) is "
                                    "less than Present Value");
        }
    }
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
}

double accept_pv() {
    double pv;
    while (1) {
        accept_str("Present Value - - -> ");
        if (in_str[0] == '\0')
            pv = 0;
        else {
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

double accept_n(double pv) {
    double n;
    while (1) {
        accept_str("Number of Payments > ");
        if (is_numeric(in_str)) {
            n = atof(in_str);
            if (n < 0)
                error_press_any_key("Number of Payments can't be less than 0");
            else {
                if (pv == 0 && n == 0) {
                    error_press_any_key(
                        "Present Value and Number of Payments both 0");
                    return (FALSE);
                } else
                    break;
            }
        } else
            error_press_any_key("Number of Payments must be numeric");
    }
    return (n);
}

double accept_i(double pv, double n) {
    double i;
    while (1) {
        accept_str("Interest Rate - - -> ");
        if (in_str[0] == '\0')
            i = 0;
        else {
            if (is_numeric(in_str)) {
                i = atof(in_str);
                if (i < 0)
                    error_press_any_key("interest Rate can't be less than 0");
                else {
                    if (pv == 0 && i == 0) {
                        error_press_any_key(
                            "Present Value and interest Rate both 0");
                        return (FALSE);
                    }
                    if (n == 0 && i == 0) {
                        error_press_any_key(
                            "Number of Payments and Interest Rate both 0");
                        return (FALSE);
                    } else
                        break;
                }
            } else
                error_press_any_key("Interest Rate must be numeric");
        }
    }
    return (i);
}

double accept_pmt(double pv, double n, double i) {
    double pmt;
    while (1) {
        accept_str("Payment Amount  - -> ");
        if (in_str[0] == '\0')
            pmt = 0;
        else {
            if (is_numeric(in_str)) {
                pmt = atof(in_str);
                if (pv == 0 && pmt == 0) {
                    error_press_any_key(
                        "Present Value and Payment Amount both 0");
                    return (FALSE);
                }
                if (n == 0 && pmt == 0) {
                    error_press_any_key(
                        "Number of Payments and Payment Amount both 0");
                    return (FALSE);
                }
                if (i == 0 && pmt == 0) {
                    error_press_any_key(
                        "Interest Rate and Payment Amount both 0");
                    return (FALSE);
                }
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

    i1 = i / 1200;
    pv = pmt * (1 - pow(1 + i1, -n)) / i1;
    printf("Present Value - - - - - -> %s\n", format_currency(pv));
    return (pv);
}

double calculate_n(double pv, double i, double pmt) {
    double i1, n;

    i1 = i / 1200;
    n = -log(1 - pv * i1 / pmt) / log(1 + i1);
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
    printf("interest Rate - - - - - -> %s\n", format_interest(i));
    return (i);
}

double calculate_pmt(double pv, double n, double i) {
    double i1, pmt;

    i1 = i / 1200;
    pmt = pv * i1 / (1 - pow(1 + i1, -n));
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
