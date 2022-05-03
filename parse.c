#include "9cc.h"

char *user_input;
Token *token;

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
    error_at(token->str, "expected \"%s\"", op);
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
	tok->len = len;
  cur->next = tok;
  return tok;
}

bool startwith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
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
		// Multi-letter punctuator
		if (startwith(p, "==") || startwith(p, "!=") ||
				startwith(p, "<=") || startwith(p, ">=")) {
					cur = new_token(TK_RESERVED, cur, p, 2);
					p += 2;
					continue;
		}
		// Single-letter punctuator
    if (strchr("+-*/()<>", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
			char *q = p;
      cur->val = strtol(p, &p, 10); // ここで、pのアドレスを更新している
      cur->len = p - q;
			continue;
    }

    error("cannot tokenize");
  }
  new_token(TK_EOF, cur, p, 0);
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
