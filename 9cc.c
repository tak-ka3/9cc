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

// プロトタイプ宣言
Node *expr();
Node *mul();
Node *primary();

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;

// error箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s\n", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap; // 可変長個の引数のリスト
  va_start(ap, fmt); // apが指す先を最初の引数のfmtに向けてあげる
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// トークンが期待しているものであるときはトークンを一つ読み進めて
// 真を返す。それ以外の場合は偽を返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED || 
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next;
  return true;
}

// 次のトークンが期待している記号の時は、トークンを一つ読み進める
// それ以外の時はエラーを報告する
void expect(char *op) {
  if (token->kind != TK_RESERVED || 
      strlen(op) != token->len || 
      memcmp(token->str, op, token->len)) {
    error("This is not '%c'", op);
  }
  token = token->next;
}

// 次のトークンが数値の場合は、トークンを一つ読み進めてその数値を返す
// それ以外はエラーを報告する
int expect_number() {
  if (token->kind != TK_NUM) {
    error("not a number");
  }
  int val = token->val;
  token = token->next;
  return val;
}

// 新しいトークンを作成してcurに繋げる
// curという現在のtokenから新しいtokというトークンを作る
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;
  
  // 空白で抜ける
  while (*p)
  {
    // 空文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }
    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10); // ここで、pのアドレスを更新している
      continue;
    }

    error("cannot tokenize");
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}

Node *new_node(NodeKind kind, Node* lhs, Node* rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  return new_node_num(expect_number()); // 数値はここでのみ返される
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for(;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

Node *relational() {
  Node *node = add();

  for(;;) {
    if (consume("<=")) {
      node = new_node(ND_SLE, node, add());
    } else if (consume(">=")) {
      node = new_node(ND_SLE, add(), node);
    } else if (consume("<")) {
      node = new_node(ND_SL, node, add());
    } else if (consume(">")) {
      node = new_node(ND_SL, add(), node);
    } else {
      return node;
    }
  }
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("==")) {
      node = new_node(ND_SE, node, relational());
    } else if (consume("!=")) {
      node = new_node(ND_SNE, node, relational());
    } else {
      return node;
    }
  }
}

Node *expr() {
  Node *node = equality();

  for (;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, equality());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, equality());
    } else {
      return node;
    }
  }
}

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }
  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
  
  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_SE:
      printf("  cmp rax, rdi\n");
      printf("  sete al");
      printf("  movzb rax, al");
    case ND_SNE:
      printf("  cmp rax, rdi\n");
      printf("  setne al");
      printf("  movzb rax, al");
      break;
    case ND_SLE:
      printf("  cmp rax, rdi\n");
      printf("  setle al");
      printf("  movzb rax, al");
      break;
    case ND_SL:
      printf("  cmp rax, rdi\n");
      printf("  setl al");
      printf("  movzb rax, al");
      break;
    default:
      error("error node kind\n");
      break;
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error_at(token->str, "引数の個数が正しくありません");
    return 1;
  }

  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 抽象構文木を降りながらコード生成
  gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
