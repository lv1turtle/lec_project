/*
 * Copyright 2021. Heekuck Oh, all rights reserved
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

int sudoku[9][9] = {{6,3,9,8,4,1,2,7,5},{7,2,4,9,5,3,1,6,8},{1,8,5,7,2,6,3,9,4},{2,5,6,1,3,7,4,8,9},{4,9,1,5,8,2,6,3,7},{8,7,3,4,6,9,5,2,1},{5,4,2,3,9,8,7,1,6},{3,1,8,6,7,5,9,4,2},{9,6,7,2,1,4,8,5,3}};

/*
 * valid[0][0], valid[0][1], ..., valid[0][8]: 각 행이 올바르면 1, 아니면 0
 * valid[1][0], valid[1][1], ..., valid[1][8]: 각 열이 올바르면 1, 아니면 0
 * valid[2][0], valid[2][1], ..., valid[2][8]: 각 3x3 그리드가 올바르면 1, 아니면 0
 */
int valid[3][9];

/*
 * 스도쿠 퍼즐의 각 행이 올바른지 검사한다.
 * 행 번호는 0부터 시작하며, i번 행이 올바르면 valid[0][i]에 1을 기록한다.
 */
void *check_rows(void *arg)
{
    for (int i = 0; i < 9; i++) {
        int bitmask = 0; // Bitmask 사용 (= 000000000)
        for (int j = 0; j < 9; j++) {
            //bitmask를 사용하기 위해 sudoku의 수를 다루기 쉬운 bit로 만든다. 
            int sudoku_bit = pow(2,sudoku[i][j] - 1); // (sudoku의 수 - 1)을 2의 지수로 만들어 bit를 만든다. 1 -> 0001, 2 -> 0010, 3 -> 0100, ... , 9 -> 100000000
            bitmask = sudoku_bit ^ bitmask; // xor 연산을 이용, 1~9가 전부 들어왔다면 bitmask는 111111111, 즉 2^9-1 = 511이여야 한다.
        }
        if (bitmask == 511)
            valid[0][i] = 1;
        else
            valid[0][i] = 0;
    }
    pthread_exit(NULL);
}

/*
 * 스도쿠 퍼즐의 각 열이 올바른지 검사한다.
 * 열 번호는 0부터 시작하며, j번 열이 올바르면 valid[1][j]에 1을 기록한다.
 */
void *check_columns(void* arg)
{
    for (int j = 0; j < 9; j++) {
        int bitmask = 0; // Bitmask 사용 (= 000000000)
        for (int i = 0; i < 9; i++) {
            //bitmask를 사용하기 위해 sudoku의 수를 다루기 쉬운 bit로 만든다. 
            int sudoku_bit = pow(2, sudoku[i][j] - 1); // (sudoku의 수 - 1)을 2의 지수로 만들어 bit를 만든다. 1 -> 0001, 2 -> 0010, 3 -> 0100, ... , 9 -> 100000000
            bitmask = sudoku_bit ^ bitmask; // xor 연산을 이용, 1~9가 전부 들어왔다면 bitmask는 111111111, 즉 2^9-1 = 511이여야 한다.
        }
        if (bitmask == 511)
            valid[1][j] = 1;
        else
            valid[1][j] = 0;
    }
    pthread_exit(NULL);
}

/*
 * 스도쿠 퍼즐의 각 3x3 서브그리드가 올바른지 검사한다.
 * 3x3 서브그리드 번호는 0부터 시작하며, 왼쪽에서 오른쪽으로, 위에서 아래로 증가한다.
 * k번 서브그리드가 올바르면 valid[2][k]에 1을 기록한다.
 */
void *check_subgrid(void *arg)
{
    int k = *(int*)arg; // arg를 통해 k 값을 전달 받는다.
    /* k번째 subgrid의 시작점은 ( k / 3 * 3 , k % 3 * 3 )와 같다. */
    int y = (k / 3) * 3; // k번째 subgrid의 시작점의 column 값
    int x = (k % 3) * 3; // k번째 subgrid의 시작점의 row 값
    int bitmask = 0; // Bitmask 사용 (= 000000000)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int sudoku_bit = pow(2, sudoku[y+i][x+j] - 1); // (sudoku의 수 - 1)을 2의 지수로 만들어 bit를 만든다. 1 -> 0001, 2 -> 0010, 3 -> 0100, ... , 9 -> 100000000
            bitmask = sudoku_bit ^ bitmask; // xor 연산을 이용, 1~9가 전부 들어왔다면 bitmask는 111111111, 즉 2^9-1 = 511이여야 한다.
        }
    }
    if (bitmask == 511)
        valid[2][k] = 1;
    else
        valid[2][k] = 0;
    pthread_exit(NULL);
}

/*
 * 스도쿠 퍼즐이 올바르게 구성되어 있는지 11개의 스레드를 생성하여 검증한다.
 * 한 스레드는 각 행이 올바른지 검사하고, 다른 한 스레드는 각 열이 올바른지 검사한다.
 * 9개의 3x3 서브그리드에 대한 검증은 9개의 스레드를 생성하여 동시에 검사한다.
 */
void check_sudoku(void)
{
    int i, j;
    /*
     * 검증하기 전에 먼저 스도쿠 퍼즐의 값을 출력한다.
     */
    for (i = 0; i < 9; ++i) {
        for (j = 0; j < 9; ++j)
            printf("%2d", sudoku[i][j]);
        printf("\n");
    }
    printf("---\n");
    /*
     * 스레드를 생성하여 각 행을 검사하는 check_rows() 함수를 실행한다.
     */
    pthread_t row;
    if (pthread_create(&row, NULL, check_rows, NULL) != 0) {
        fprintf(stderr, "pthread_create error: check_rows\n");
        exit(-1);
    }
    /*
     * 스레드를 생성하여 각 열을 검사하는 check_columns() 함수를 실행한다.
     */
    pthread_t col;
    if (pthread_create(&col, NULL, check_columns, NULL) != 0) {
        fprintf(stderr, "pthread_create error: check_columns\n");
        exit(-1);
    }
    /*
     * 9개의 스레드를 생성하여 각 3x3 서브그리드를 검사하는 check_subgrid() 함수를 실행한다.
     * 3x3 서브그리드의 위치를 식별할 수 있는 값을 함수의 인자로 넘긴다.
     */
    pthread_t subgrid[9];
    int n[9];
    for (int k = 0; k < 9; k++) {
        n[k] = k;
        int *p = &n[k]; // 포인터 p를 통해 k값을 쓰레드에 전달한다.
        if (pthread_create(&subgrid[k], NULL, check_subgrid, p) != 0) {
            fprintf(stderr, "pthread_create error: check_subgrid\n");
            exit(-1);
        }
    }
    /*
     * 11개의 스레드가 종료할 때까지 기다린다.
     */
    pthread_join(row, NULL);
    pthread_join(col, NULL);
    for (int k = 0; k < 9; k++) {
        pthread_join(subgrid[k], NULL);
    }
    /*
     * 각 행에 대한 검증 결과를 출력한다.
     */
    printf("ROWS: ");
    for (i = 0; i < 9; ++i)
        printf(valid[0][i] == 1 ? "(%d,YES)" : "(%d,NO)", i);
    printf("\n");
    /*
     * 각 열에 대한 검증 결과를 출력한다.
     */
    printf("COLS: ");
    for (i = 0; i < 9; ++i)
        printf(valid[1][i] == 1 ? "(%d,YES)" : "(%d,NO)", i);
    printf("\n");
    /*
     * 각 3x3 서브그리드에 대한 검증 결과를 출력한다.
     */
    printf("GRID: ");
    for (i = 0; i < 9; ++i)
        printf(valid[2][i] == 1 ? "(%d,YES)" : "(%d,NO)", i);
    printf("\n---\n");
}

/*
 * 스도쿠 퍼즐의 값을 3x3 서브그리드 내에서 무작위로 섞는 함수이다.
 */
void *shuffle_sudoku(void *arg)
{
    int i, tmp;
    int grid;
    int row1, row2;
    int col1, col2;
    
    srand(time(NULL));
    for (i = 0; i < 100; ++i) {
        /*
         * 0부터 8번 사이의 서브그리드 하나를 무작위로 선택한다.
         */
        grid = rand() % 9;
        /*
         * 해당 서브그리드의 좌측 상단 행열 좌표를 계산한다.
         */
        row1 = row2 = (grid/3)*3;
        col1 = col2 = (grid%3)*3;
        /*
         * 해당 서브그리드 내에 있는 임의의 두 위치를 무작위로 선택한다.
         */
        row1 += rand() % 3; col1 += rand() % 3;
        row2 += rand() % 3; col2 += rand() % 3;
        /*
         * 홀수 서브그리드이면 두 위치에 무작위 수로 채우고,
         */
        if (grid & 1) {
            sudoku[row1][col1] = rand() % 8 + 1;
            sudoku[row2][col2] = rand() % 8 + 1;
        }
        /*
         * 짝수 서브그리드이면 두 위치에 있는 값을 맞바꾼다.
         */
        else {
            tmp = sudoku[row1][col1];
            sudoku[row1][col1] = sudoku[row2][col2];
            sudoku[row2][col2] = tmp;
        }
    }
    pthread_exit(NULL);
}

/*
 * 메인 함수는 위에서 작성한 함수가 올바르게 동작하는지 검사하기 위한 것으로 수정하면 안 된다.
 */
int main(void)
{
    int tmp;
    pthread_t tid;
    
    /*
     * 기본 스도쿠 퍼즐을 출력하고 검증한다.
     */
    check_sudoku();
    /*
     * 기본 퍼즐에서 값 두개를 맞바꾸고 검증해본다.
     */
    tmp = sudoku[5][3]; sudoku[5][3] = sudoku[6][2]; sudoku[6][2] = tmp;
    check_sudoku();
    /*
     * 기본 스도쿠 퍼즐로 다시 바꾼 다음, shuffle_sudoku 스레드를 생성하여 퍼즐을 섞는다.
     */
    tmp = sudoku[5][3]; sudoku[5][3] = sudoku[6][2]; sudoku[6][2] = tmp;
    if (pthread_create(&tid, NULL, shuffle_sudoku, NULL) != 0) {
        fprintf(stderr, "pthread_create error: shuffle_sudoku\n");
        exit(-1);
    }
    /*
     * 무작위로 섞는 중인 스도쿠 퍼즐을 검증해본다.
     */
    check_sudoku();
    /*
     * shuffle_sudoku 스레드가 종료될 때까지 기다란다.
     */
    pthread_join(tid, NULL);
    /*
     * shuffle_sudoku 스레드 종료 후 다시 한 번 스도쿠 퍼즐을 검증해본다.
     */
    check_sudoku();
    exit(0);
}
