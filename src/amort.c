/** @file amort.c
    @brief Installment Loan Amortization
    @author Bill Waller
    Copyright (c) 2026
    MIT License
    billxwaller@gmail.com
    @date 2026-05-05
 */

#define _XOPEN_SOURCE
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

char month[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

typedef struct {
    double pv;
    double pmt;
    double i;
    double n;
    double interest;
    double principal;
    double year_total_interest;
} Amort;

void print_totals(Amort *);

int main(int argc, char **argv) {
    setlocale(LC_NUMERIC, "");
    if (argc > 1 &&
        ((strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) ||
         argc < 5)) {
        printf("Usage: amort [present_value][number_of_payments]"
               "[interest_rate][payment_amount][yyyy-mm-dd]\n\n");
        exit(EXIT_SUCCESS);
    }
    struct tm tm;
    Amort *amort = malloc(sizeof(Amort));
    sscanf(argv[1], "%lf", &amort->pv);
    sscanf(argv[2], "%lf", &amort->n);
    sscanf(argv[3], "%lf", &amort->i);
    sscanf(argv[4], "%lf", &amort->pmt);
    if (amort->pv == 0 || amort->n == 0 || amort->i == 0 || amort->pmt == 0 ||
        argv[5][0] == '\0') {
        fprintf(stderr, "Error: All arguments must be non-zero.\n");
        exit(EXIT_FAILURE);
    }
    amort->interest = amort->pv * amort->i / 1200;
    if (amort->interest > amort->pmt) {
        fprintf(stderr, "Error: Payment amount less than interest\n");
        exit(EXIT_FAILURE);
    }
    memset(&tm, 0, sizeof(tm));
    if (strptime(argv[5], "%Y-%m-%d", &tm) == NULL)
        if (strptime(argv[5], "%Y%m%d", &tm) == NULL)
            printf("Error: Invalid date format. Use yyyy-mm-dd or yyyymmdd.\n"),
                exit(EXIT_FAILURE);
    amort->year_total_interest = 0;

    printf("First Payment: %04d-%02d-%02d\n\n", tm.tm_year + 1900,
           tm.tm_mon + 1, tm.tm_mday);
    printf(" Per  Mth Year    Balance     Payment   Principal  Interest\n");
    printf("----  --- ---- ------------ ---------- ---------- ----------\n");
    int m = 0;
    for (int x = 0; x < amort->n; x++) {
        if (amort->pv <= 0)
            break;
        m = (x + tm.tm_mon) % 12;
        int y = ((x + tm.tm_mon) / 12) + tm.tm_year + 1900;
        amort->interest = amort->pv * amort->i / 1200;
        amort->year_total_interest += amort->interest;
        if (amort->pv + amort->interest < amort->pmt) {
            amort->pmt = amort->pv + amort->interest;
            amort->principal = amort->pv;
        } else
            amort->principal = amort->pmt - amort->interest;
        amort->pv -= amort->principal;
        printf("%4d %4s %4d %'12.2f %'10.2f %'10.2f %'10.2f\n", x + 1, month[m],
               y, amort->pv, amort->pmt, amort->principal, amort->interest);
        if (m == 11)
            print_totals(amort);
    }
    if (m != 11)
        print_totals(amort);
    free(amort);
    return EXIT_SUCCESS;
}
void print_totals(Amort *amort) {
    printf("                                                  "
           "----------\n");
    printf("                                                  %'10.2f\n",
           amort->year_total_interest);
    printf("                                                  "
           "==========\n\n");
    amort->year_total_interest = 0;
}
