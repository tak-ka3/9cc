#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
// enumは複数の変数に一連の整数値をつける
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM, // 整数トークン
  TK_EOF, // 入力の終わりを表すトークン
} TokenKind;

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_SE, // ==
  ND_SNE, // !=
  ND_SL, // set less than <
  ND_SLE, // set less than equal <=
  ND_NUM, // 整数
} NodeKind;

// struct TokenをTokenという名前の型で定義している
// わざわざ型を定義するときに、struct Token *t; などと定義しなくていいようになる
/* typedef struct Token{
  ~~~
}Token;
という書き方も一般的
*/
// 入力をトークン列に変換する
typedef struct Token Token;

struct Token {
  TokenKind kind; // トークンの型
  Token *next; // 次の入力トークン
  int val; // kindがTK_NUMの場合、その数値
  char *str; // トークン文字列
  int len; // トークンの長さ
};

// トークン列をパースして抽象構文木に変換する
typedef struct Node Node;

struct Node {
  NodeKind kind; // ノードの型
  Node* lhs; // 左辺
  Node* rhs; // 右辺
  int val; // 整数だった場合の型
};

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startwith(char *p, char *q);
bool at_eof();
Token *tokenize(char *p);
Node *new_node(NodeKind kind, Node* lhs, Node* rhs);
Node *new_node_num(int val);
Node *primary();
Node *unary();
Node *mul();
Node *add();
Node *relational();
Node *equality();
Node *expr();


void gen(Node *node);

